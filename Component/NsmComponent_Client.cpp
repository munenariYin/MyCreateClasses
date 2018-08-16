#include"../Network/NsmCommunicationProtocol.h"
#include"../MemberClasses/NsmCommunicationAsync.h"

#include "NsmComponent_Client.h"



RTTI_IMPL(CNsmComponent_Client, IComponent_Base)

CNsmComponent_Client::CNsmComponent_Client()
{
	NsmCommunicationProtocol::Init();
}

CNsmComponent_Client::~CNsmComponent_Client()
{
	// 終了フラグを立て、並列処理の終了を待つ
	m_isTerminateApp = true;
	if (m_commAsync != nullptr)
	{
		m_commAsync->SetEnableAsync(false);
		m_commAsync->WaitEndAsync();
	}

}

void CNsmComponent_Client::InitFromJson(const json11::Json& _rJsonObj)
{
	IComponent_Base::InitFromJson(_rJsonObj);

	auto jsonPortNo = _rJsonObj["PortNo"];
	if (jsonPortNo.is_number())
	{
		m_portNo = jsonPortNo.int_value();
	}
}

void CNsmComponent_Client::Start()
{
	// WinsockAPI使用準備
	unsigned short version = MAKEWORD(1, 1);
	if (WSAStartup(version, &m_wsaData) != 0)
	{
		// WinSockAPI初期化に失敗したら、コンポーネント自体を削除する。
		SetIsDeleteComponent(true);
		Dw_Scroll(DW_ScrollType::kUser, "WinSockAPIの初期化に失敗しました。");
		WSACleanup();
		return;
	}

}

void CNsmComponent_Client::Update(void)
{
	// 接続の試行
	if (m_connectionState == ConnectionState::NotConnection)
	{
		if (INPUT.KeyCheck(VK_RETURN))
		{
			if(BeginConnect());
		}
		return;
	}
	// 接続待機
	if (m_connectionState == ConnectionState::WaitConnection) 
	{
		WaitConnect();
		return;
	}



	// 接続を終了する
	if (!m_commAsync->IsEnableAsync()) m_commAsync->WaitEndAsync();
	
}

void CNsmComponent_Client::LateUpdate(void)
{
}

void CNsmComponent_Client::Draw(void)
{
}

void CNsmComponent_Client::Release(void)
{
}

bool CNsmComponent_Client::BeginConnect()
{
	if(m_commAsync == nullptr) m_commAsync = std::make_shared<CommunicationAsync>();

	if (!m_commAsync->PrepareConnectSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, AF_INET, SocketUtil::TransHostAddr("127.0.0.1"), m_portNo))
	{
		Dw_Scroll(DW_ScrollType::kUser, "ソケットの準備に失敗しました。");
		return false;
	}
	Dw_Scroll(DW_ScrollType::kUser, "接続開始");
	m_connectionState = ConnectionState::WaitConnection;
	return true;
}

void CNsmComponent_Client::AddInputDataBuffer(ControllerData& _inputData)
{
	if (m_commAsync == nullptr) return;
	m_commAsync->AddSendBuffer(_inputData);
}

void CNsmComponent_Client::WaitConnect()
{
	if (Connect()) return;
	// 未接続のスルーなら無視
	if (m_connectionState == ConnectionState::WaitConnection) return;
	// 失敗した場合...
	m_commAsync->Clear();
	m_connectionState = ConnectionState::NotConnection;
	
}

bool CNsmComponent_Client::Connect()
{
	SPtr<Socket> commSocket = m_commAsync->GetSocket();
	// 接続を試みる
	if (!commSocket->Connect())
	{
		int errorCode = m_commAsync->UpdateErrorCode();
		// エラーがノンブロッキングによるスルーであれば再試行
		if (errorCode == WSAEWOULDBLOCK) return false;
		// connect試行の後に接続された場合、成功とする。
		if (errorCode != WSAEISCONN)
		{
			std::string errorString = "接続失敗。エラーコード : " + std::to_string(errorCode);
			Dw_Scroll(DW_ScrollType::kUser, errorString.c_str());
			m_connectionState = ConnectionState::FailedConnection;
			return false;
		}
	}
	// 接続に成功すれば非同期処理を開始する。
	CreateCommunicateAsync();
	Dw_Scroll(DW_ScrollType::kUser, "接続成功");
	m_connectionState = ConnectionState::Connection;
	return true;
}

void CNsmComponent_Client::CreateCommunicateAsync()
{
	bool& refIsTerminateApp = m_isTerminateApp;
	WPtr<CommunicationAsync> commAsync = m_commAsync;
	// 会話スレッド //--------------------------------------------------
	std::function<bool()> communicationAsync = [this, commAsync, &refIsTerminateApp]()
	{
		while (true)
		{
			// サーバの終了、もしくは非同期処理の終了フラグが立てば終える
			if (refIsTerminateApp || !commAsync.lock()->IsEnableAsync()) return true;

			CommDataState dataState = commAsync.lock()->Receive();
			// ⑦メッセージ受信
			if (dataState == CommDataState::Success)
			{
				std::string recvData = "受信[server] port : " + std::to_string(commAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, recvData.c_str());
			}
			else if(dataState == CommDataState::Failed)
			{
				int errorCode = commAsync.lock()->UpdateErrorCode();
				if (errorCode != WSAEWOULDBLOCK)
				{
					std::string errorMessage = "recv()時、回線が切断されていました。エラーコード : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorMessage.c_str());
					// 終了フラグ
					commAsync.lock()->SetEnableAsync(false);
					return false;
				}
			}

			dataState = commAsync.lock()->Send();
			if (dataState == CommDataState::Success)
			{
				std::string recvData = "送信[server] port : " + std::to_string(commAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, recvData.c_str());
			}
			else if (dataState == CommDataState::Failed)
			{
				int errorCode = commAsync.lock()->UpdateErrorCode();
				if (errorCode != WSAEWOULDBLOCK)
				{
					std::string errorMessage = "send()時、回線が切断されていました。エラーコード : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorMessage.c_str());
					// 終了フラグ
					commAsync.lock()->SetEnableAsync(false);
					return false;
				}
			}

		}
		return true;
	};
	// 会話スレッド終了 //----------------------------------------------
	m_commAsync->SetAsync(communicationAsync);

}

ConnectionState CNsmComponent_Client::GetConnectionState()
{
	return m_connectionState;
}

