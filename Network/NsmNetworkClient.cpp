#include"../MemberClasses/NsmCommunicationAsync.h"
#include "../Nisiguti.h"

CNsmNetworkClient::CNsmNetworkClient()
{
	m_connState = ConnectionState::NotConnection;
	m_enableClient = false;
	m_commAsync = nullptr;
}

CNsmNetworkClient::~CNsmNetworkClient()
{
}

bool CNsmNetworkClient::StartUp()
{
	if (m_enableClient)
	{
		Dw_Scroll(DW_ScrollType::kUser, "StartUpは既に成功しています");
		return true;
	}
	// WinsockAPI使用準備
	unsigned short version = MAKEWORD(1, 1);
	if (WSAStartup(version, &m_wsaData) != 0)
	{
		Dw_Scroll(DW_ScrollType::kUser, "WinSockAPIの初期化に失敗しました。");
		WSACleanup();
		return false;
	}
	m_enableClient = true;
	return true;
}

void CNsmNetworkClient::EndClient()
{
	m_sendBuffer.clear();
	m_recvBuffer.clear();

	DisConnect();

	if (!m_enableClient) return;
	m_enableClient = false;
	WSACleanup();
}

bool CNsmNetworkClient::BeginConnect(const std::string & _serverlp, unsigned short _portNo)
{
	// 未接続である場合のみ接続を開始できる
	if (m_connState != ConnectionState::NotConnection)
	{
		Dw_Scroll(DW_ScrollType::kUser, "正常に接続ができない可能性があります。");
		return false;
	}
	if (m_commAsync == nullptr) m_commAsync = std::make_shared<CommunicationAsync>();
	if (m_commAsync->IsEnableAsync())
	{
		Dw_Scroll(DW_ScrollType::kUser, "既に別の非同期処理が行われています。");
		return false;
	}
	if (!m_commAsync->PrepareConnectSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, AF_INET, SocketUtil::TransHostAddr("127.0.0.1"), _portNo))
	{
		Dw_Scroll(DW_ScrollType::kUser, "ソケットの生成に失敗しました。");
		return false;
	}

	ConnectionState& refConnState = m_connState;
	bool& refEnableClient = m_enableClient;
	WPtr<CommunicationAsync> connectionAsync = m_commAsync;

	std::function<bool()> connectionProc = [&refConnState, &refEnableClient, connectionAsync]()
	{
		while (true)
		{
			if (connectionAsync.expired() || !connectionAsync.lock()->IsEnableAsync() || !refEnableClient)
			{
				refConnState = ConnectionState::FailedConnection;
				Dw_Scroll(DW_ScrollType::kUser, "スレッドが終了されました。");
				return false;
			}

			// 接続を試みる
			if (!connectionAsync.lock()->GetSocket()->Connect())
			{
				int errorCode = connectionAsync.lock()->UpdateErrorCode();
				// エラーの原因ががノンブロッキングによるスルーであれば再試行
				if (errorCode == WSAEWOULDBLOCK) continue;
				// connect試行の後に接続された場合、成功とする。
				if (errorCode != WSAEISCONN)
				{
					std::string errorString = "接続失敗。エラーコード : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorString.c_str());
					refConnState = ConnectionState::FailedConnection;
					return false;
				}
			}
			// 接続状態にして、スレッドを終了する。
			refConnState = ConnectionState::Connection;
			break;
		}
		return true;
	};

	// 接続待ち状態にする
	m_connState = ConnectionState::WaitConnection;
	m_commAsync->SetAsync(connectionProc);
	return true;
}

bool CNsmNetworkClient::CompleteConnecting()
{
	if (m_connState != ConnectionState::Connection)
	{
		Dw_Scroll(DW_ScrollType::kUser, "既に接続を完了しているか、接続を確立できません。");
		return false;
	}
	// 非同期の接続処理を終了する。
	if (m_commAsync == nullptr) return false;
	m_commAsync->WaitEndAsync();

	return BeginCommunication();
}

bool CNsmNetworkClient::BeginCommunication()
{
	// 他の非同期処理がまだ行われていれば、まだ通信を開始できない。
	if (m_connState != ConnectionState::Connection || m_commAsync == nullptr)
	{
		Dw_Scroll(DW_ScrollType::kUser, "接続が確立されていません");
		return false;
	}
	if (m_commAsync->IsValidAsync())
	{
		Dw_Scroll(DW_ScrollType::kUser, "他のスレッドが稼働しています。");
		return false;
	}
	
	bool& refEnableClient = m_enableClient;
	ConnectionState& refConnectState = m_connState;
	WPtr<CommunicationAsync> communicationAsync = m_commAsync;
	std::function<bool()> communicationProc = [&refEnableClient, &refConnectState, communicationAsync]()
	{
		while (true)
		{
			// クライアントが終了しているか、非同期処理の終了を感知したら終了
			if (!refEnableClient || communicationAsync.expired() || communicationAsync.lock()->IsEnableAsync())
			{
				refConnectState = ConnectionState::FailedConnection;
				break;
			}
			CommDataState dataState = communicationAsync.lock()->Receive();
			// ⑦メッセージ受信
			if (dataState == CommDataState::Success)
			{
				std::string recvData = "受信[server] port : " + std::to_string(communicationAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, recvData.c_str());
			}
			else if (dataState == CommDataState::Failed)
			{
				int errorCode = communicationAsync.lock()->UpdateErrorCode();
				if (errorCode != WSAEWOULDBLOCK)
				{
					std::string errorMessage = "recv()時、回線が切断されていました。エラーコード : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorMessage.c_str());
					// 終了フラグ
					refConnectState = ConnectionState::FailedConnection;
					return false;
				}
			}

			dataState = communicationAsync.lock()->Send();
			if (dataState == CommDataState::Success)
			{
				std::string recvData = "送信[server] port : " + std::to_string(communicationAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, recvData.c_str());
			}
			else if (dataState == CommDataState::Failed)
			{
				int errorCode = communicationAsync.lock()->UpdateErrorCode();
				if (errorCode != WSAEWOULDBLOCK)
				{
					std::string errorMessage = "send()時、回線が切断されていました。エラーコード : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorMessage.c_str());
					// 終了フラグ
					refConnectState = ConnectionState::FailedConnection;
					return false;
				}
			}
		}
		return true;
	};

	m_connState = ConnectionState::Communication;
	m_commAsync->SetAsync(communicationProc);

	return true;
}

void CNsmNetworkClient::DisConnect()
{
	m_connState = ConnectionState::NotConnection;
	if (m_commAsync == nullptr) return;
	m_commAsync->WaitEndAsync();
	m_commAsync->Clear();
	m_commAsync->ClearRecvBuffer();
	m_commAsync->ClearSendBuffer();
	m_commAsync = nullptr;
}

ConnectionState CNsmNetworkClient::GetConnectionState()
{
	return m_connState;
}
