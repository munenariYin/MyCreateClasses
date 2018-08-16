#include"../Network/NsmCommunicationProtocol.h"
#include"../MemberClasses/NsmCommunicationAsync.h"

#include "NsmComponent_Server.h"

// RTTI
RTTI_IMPL(CNsmComponent_Server, IComponent_Base)

CNsmComponent_Server::CNsmComponent_Server()
{
	m_isAccepting = false;
	m_isEndServer = false;
	NsmCommunicationProtocol::Init();
}

CNsmComponent_Server::~CNsmComponent_Server()
{
	m_isEndServer = true;
	m_isAccepting = false;

	if (m_acceptAsync != nullptr)
	{
		m_acceptAsync->WaitEndAsync();
	}
	m_acceptAsync->Clear();
	m_acceptAsync = nullptr;

	for (auto client : m_clientVector)
	{
		client.first->SetEnableAsync(false);
	}
	for (auto client : m_clientVector)
	{
		client.first->WaitEndAsync();
		client.first = nullptr;
	}
	m_clientVector.clear();
	WSACleanup();
}

void CNsmComponent_Server::InitFromJson(const json11::Json& _rJsonObj)
{
	IComponent_Base::InitFromJson(_rJsonObj);

	auto jsonPortNo= _rJsonObj["PortNo"];
	if (jsonPortNo.is_number())
	{
		m_portNo = static_cast<int>(jsonPortNo.number_value());
	}

	auto jsonClientMax = _rJsonObj["ClientMax"];
	if (jsonClientMax.is_number())
	{
		m_clientCntMax = static_cast<unsigned int>(jsonClientMax.number_value());
	}
}

void CNsmComponent_Server::Start(void)
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

	BeginAccept();
}

void CNsmComponent_Server::Update(void)
{
	for (auto client : m_clientVector)
	{
		// まずクライアントから受け取ったデータを集める
		client.first->MoveRecvData(m_recvBuffer);

		// 受信バッファからコントロールデータを取り出す
		size_t controllerSize = sizeof(ControllerData);
		if (m_recvBuffer.size() < controllerSize) continue;

		// ヘッダ情報を消す
		size_t headerSize = sizeof(HeaderData);
		m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + headerSize);

		// 入力情報を得た後、そのデータ部を消す
		ControllerData controllData;
		::memcpy(reinterpret_cast<char*>(&controllData), &m_recvBuffer[0], controllerSize);
		m_playersControllerData.push_back(std::pair<unsigned short, ControllerData>(client.first->GetSocket()->GetAddr().sin_port, controllData));
		m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + controllerSize);
		
		//for (auto client : m_clientVector)
		//{
		//	for(auto object :  client.second.lock()->GetSubLevel()->spObjectRoot->ChildList)

		//}
		
		for (auto subLevel : GW.GetNowLevel()->GetSubLevelList())
		{
			for (auto child : subLevel.second.spObjectRoot->GetChildList())
			{
				child->GetChildList();
			}
		}
	}
}

void CNsmComponent_Server::LateUpdate(void)
{
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		m_isEndServer = true;
	}

	// クライアントの終了の検知をしたら削除する。
	for (int i = m_clientVector.size() - 1; i >= 0; i--)
	{
		// クライアントの非同期処理の終了を感知したら終了させて配列から外す
		if (m_clientVector[i].first->IsEnableAsync())continue;
		// スレッドの終了を待った後にリストから外す。
		m_clientVector[i].first->WaitEndAsync();
		m_clientVector[i].first = nullptr;
		m_clientVector.erase(m_clientVector.begin() + i);
	}
	if (m_isEndServer) return;
}

void CNsmComponent_Server::Draw(void)
{
}

void CNsmComponent_Server::Release(void)
{

}

bool CNsmComponent_Server::BeginAccept()
{
	m_acceptAsync = std::make_shared<CommunicationAsync>();
	// accept用ソケットの作成
	if (!m_acceptAsync->PrepareAcceptSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, AF_INET, htonl(INADDR_ANY), m_portNo))
	{
		Dw_Scroll(DW_ScrollType::kUser, "Acceptの開始に失敗しました。");
		return false;
	}
	// 受付開始
	m_isAccepting = true;
	// ---------------------------------------------------------------------
	// 受付用のスレッドを作る
	// キャプチャに用いる変数を参照体に入れる
	bool& refIsEnd = m_isEndServer;	// サーバ自体の終了感知
	bool& refIsAccepting = m_isAccepting;	// 受付の中止感知
	WPtr<CommunicationAsync> acceptAsync = m_acceptAsync;

	std::function<bool()> acceptProc = [this, &refIsEnd, &refIsAccepting, acceptAsync]()
	{
		while (true)
		{
			// 受付終了が伝えられたら非同期処理を終了する
			if (!refIsAccepting) return true;
			SPtr<Socket> newClient = std::make_shared<Socket>();
			// 接続要求があれば受け付ける
			if (!acceptAsync.lock()->GetSocket()->Accept(*newClient))
			{
				int errorCode = acceptAsync.lock()->UpdateErrorCode();
				// エラーの確認をして、ノンブロッキングによるスルーが原因なら続行
				if (errorCode == WSAEWOULDBLOCK) continue;
				std::string errorString = "エラーコード : " + std::to_string(errorCode);
				Dw_Scroll(DW_ScrollType::kUser, errorString.c_str());
				refIsAccepting = false;
				return false;
			}
			// 新たなクライアントとして登録
			this->CreateClientAsync(newClient);
		}
		return true;
	};
	// 受付関数ここまで
	// -----------------------------------------------------------------------

	// 受付スレッドの開始
	m_acceptAsync->SetAsync(acceptProc);
	Dw_Scroll(DW_ScrollType::kUser, "Accept開始");
	return true;
}

void CNsmComponent_Server::InputControllerData(ControllerData& _outControllData, short _portNo)
{
	size_t inputCnt = m_playersControllerData.size();
	for (size_t i = 0; i < inputCnt; i++)
	{
		if (m_playersControllerData[i].first != _portNo) continue;
		_outControllData = m_playersControllerData[i].second;
		m_playersControllerData.erase(m_playersControllerData.begin() + i);
		return;
	}
}

void CNsmComponent_Server::CreateClientAsync(SPtr<Socket>& _socket)
{
	// キャプチャに用いる変数の用意
	// 新しいクライアント用のソケットを確保
	SPtr<CommunicationAsync> newClientAsync = std::make_shared<CommunicationAsync>();
	newClientAsync->SetSocket(_socket);
	WPtr<CommunicationAsync> clientAsync = newClientAsync;

	// サーバ自体が終了したかを確認するためのもの
	bool& refIsEndServer = m_isEndServer;
	// ----------------------------------------------------------------------------------------
	// スレッドで動かす関数
	std::function<bool()> communicateProc = [&refIsEndServer, clientAsync]()
	{
		while (true)
		{
			// サーバ自体が終了しようとしていたら、クライアントを正常に終了させるようにする。
			if (refIsEndServer || !clientAsync.lock()->IsEnableAsync()) return false;

			// 受信
			CommDataState dataState = clientAsync.lock()->Receive();
			if (dataState == CommDataState::Failed)
			{
				int errorNo = clientAsync.lock()->UpdateErrorCode();
				// ノンブロッキングが原因のエラーならスルー
				if (errorNo != WSAEWOULDBLOCK)
				{
					Dw_Scroll(DW_ScrollType::kUser, "回線が切断されていました。: recv()");
					clientAsync.lock()->SetEnableAsync(false);
					return false;
				}
			}
			
			// 送信
			dataState = clientAsync.lock()->Send();
			if (dataState == CommDataState::Failed)
			{
				if (clientAsync.lock()->UpdateErrorCode() != WSAEWOULDBLOCK)
				{
					Dw_Scroll(DW_ScrollType::kUser, "回線が切断されていました。: send()");
					clientAsync.lock()->SetEnableAsync(false);
					return false;
				}
			}
			else if (dataState == CommDataState::Success)
			{
				std::string portString = "[client] : port : " + std::to_string(clientAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, portString.c_str());
			}
		}
		return true;
	};
	// 非同期処理ここまで
	// ------------------------------------------------------------

	// 非同期関数生成
	WPtr<CGameObject> chara = std::make_shared<CGameObject>(GW.GetNowLevel()->GetNowSubLevel());
	if (!chara.lock()->InitFromJsonFile("data/User/Nisiguti/Json/GameObject/HeroGmObj.json")) return;
	chara.lock()->SetParent(GW.GetNowLevel()->GetNowSubLevel()->GetRootObject());
	SPtr<CNsmComponent_ServerController> controller = chara.lock()->GetComponent<CNsmComponent_ServerController>();
	chara.lock()->GetTransform()->pos.Set(rand() % 4 - 2, 0.0f, rand() % 4 - 2);
	controller->SetPortNo(newClientAsync->GetSocket()->GetAddr().sin_port);
	controller->SetServer(YsToSPtr(this));
	newClientAsync->SetAsync(communicateProc);
	m_clientVector.push_back(std::make_pair(newClientAsync, chara));
	std::string successMessage = "接続したクライアントがいます。ポート : " + std::to_string(_socket->GetAddr().sin_port);
	Dw_Scroll(DW_ScrollType::kUser, successMessage.c_str());


}