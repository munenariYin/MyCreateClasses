#include"..\MemberClasses\NsmCommunicationAsync.h"
#include"..\Nisiguti.h"

CNsmNetworkServer::CNsmNetworkServer(){}

CNsmNetworkServer::~CNsmNetworkServer(){}

bool CNsmNetworkServer::StartUp()
{
	// 既に初期化されていれば何もしない
	if (m_enableServer)
	{
		Dw_Scroll(DW_ScrollType::kUser, "既にStartupは行われています。");
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
	m_enableServer = true;
	return true;
}

bool CNsmNetworkServer::Listen(unsigned long _inAddr, unsigned short _port)
{
	if (!m_enableServer) return false;
	if (m_acceptAsync != nullptr && m_acceptAsync->IsEnableAsync()) return false;

	m_acceptAsync = std::make_shared<CommunicationAsync>();
	if (!m_acceptAsync->PrepareAcceptSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, AF_INET, htonl(_inAddr), _port))
	{
		Dw_Scroll(DW_ScrollType::kUser, "Acceptの開始に失敗しました。");
		return false;
	}

	m_portNo = _port;
	// 受付開始
	m_enableAccept = true;
	// ---------------------------------------------------------------------
	// 受付用のスレッドを作る
	// キャプチャに用いる変数を参照体に入れる
	bool& refEnableServer = m_enableServer;	// サーバ自体の終了感知
	bool& refEnableAccept = m_enableAccept;	// 受付の中止感知
	WPtr<CommunicationAsync> acceptAsync = m_acceptAsync;

	std::function<bool()> acceptProc = [this, &refEnableServer, &refEnableAccept, acceptAsync]()
	{
		while (true)
		{
			if (acceptAsync.expired()) return false;
			// 受付終了が伝えられたら非同期処理を終了する
			if (!refEnableAccept || !refEnableServer || !acceptAsync.lock()->IsEnableAsync()) break;
			SPtr<Socket> newClient = std::make_shared<Socket>();
			// 接続要求があれば受け付ける
			if (!acceptAsync.lock()->GetSocket()->Accept(*newClient))
			{
				int errorCode = acceptAsync.lock()->UpdateErrorCode();
				// エラーの確認をして、ノンブロッキングによるスルーが原因なら続行
				if (errorCode == WSAEWOULDBLOCK) continue;
				std::string errorString = "エラーコード : " + std::to_string(errorCode);
				Dw_Scroll(DW_ScrollType::kUser, errorString.c_str());
				refEnableAccept = false;
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

void CNsmNetworkServer::StopListen()
{
	m_enableAccept = false;
	if (m_acceptAsync == nullptr) return;
	m_acceptAsync->WaitEndAsync();
	m_acceptAsync->Clear();
	m_acceptAsync = nullptr;
}

void CNsmNetworkServer::ConnectAllPendClients()
{
	for (int i = m_pendClientVector.size() - 1; i >= 0; i--)
	{
		m_clientVector.push_back(m_pendClientVector[i]);
		m_pendClientVector.erase(m_pendClientVector.begin() + i);
	}
}

void CNsmNetworkServer::RemoveDisconnectedClients()
{
	// 必ずスレッドを終了させてからクライアントを消す
	for (int i = m_clientVector.size() - 1; i >= 0; i--)
	{
		// まだ終了していないクライアントは消さない
		if (!m_clientVector[i]->IsEnableAsync()) continue;
		m_clientVector[i]->WaitEndAsync();
		m_clientVector[i]->Clear();
		m_clientVector[i] = nullptr;
		m_clientVector.erase(m_clientVector.begin() + i);
	}

	// 必ずスレッドを終了させてからクライアントを消す
	for (int i = m_pendClientVector.size() - 1; i >= 0; i--)
	{
		// まだ終了していないクライアントは消さない
		if (!m_pendClientVector[i]->IsEnableAsync()) continue;
		m_pendClientVector[i]->WaitEndAsync();
		m_pendClientVector[i]->Clear();
		m_pendClientVector[i] = nullptr;
		m_pendClientVector.erase(m_pendClientVector.begin() + i);
	}
}

void CNsmNetworkServer::DisconnectAll()
{
	// 必ずスレッドを終了させてからクライアントを消す
	for (int i = m_clientVector.size() - 1; i >= 0; i--)
	{
		m_clientVector[i]->SetEnableAsync(false);
		m_clientVector[i]->WaitEndAsync();
		m_clientVector[i]->Clear();
		m_clientVector[i] = nullptr;
	}
	m_clientVector.clear();

	for (int i = m_pendClientVector.size() - 1; i >= 0; i--)
	{
		m_pendClientVector[i]->SetEnableAsync(false);
		m_pendClientVector[i]->WaitEndAsync();
		m_pendClientVector[i]->Clear();
		m_pendClientVector[i] = nullptr;
	}
	m_pendClientVector.clear();

}

void CNsmNetworkServer::CreateClientAsync(SPtr<Socket>& _socket)
{
	// キャプチャに用いる変数の用意
	// 新しいクライアント用のソケットを確保
	SPtr<CommunicationAsync> newClientAsync = std::make_shared<CommunicationAsync>();
	newClientAsync->SetSocket(_socket);
	WPtr<CommunicationAsync> clientAsync = newClientAsync;

	// サーバ自体が終了したかを確認するためのもの
	bool& refEnableServer = m_enableServer;

	// ----------------------------------------------------------------------------------------
	// スレッドで動かす関数
	std::function<bool()> communicateProc = [&refEnableServer, clientAsync]()
	{
		while (true)
		{
			// サーバ自体が終了しようとしていたら、クライアントを正常に終了させるようにする。ポインタが消えてしまっていた場合も同様
			if (refEnableServer || clientAsync.expired() || !clientAsync.lock()->IsEnableAsync()) break;

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

	//WPtr<CGameObject> chara = std::make_shared<CGameObject>(GW.GetNowLevel()->GetNowSubLevel());
	//if (!chara.lock()->InitFromJsonFile("data/User/Nisiguti/Json/GameObject/HeroGmObj.json")) return;
	//chara.lock()->SetParent(GW.GetNowLevel()->GetNowSubLevel()->GetRootObject());
	//SPtr<CNsmComponent_ServerController> controller = chara.lock()->GetComponent<CNsmComponent_ServerController>();
	//chara.lock()->GetTransform()->pos.Set(rand() % 4 - 2, 0.0f, rand() % 4 - 2);
	//controller->SetPortNo(newClientAsync->GetSocket()->GetAddr().sin_port);
	//controller->SetServer(YsToSPtr(this));

	newClientAsync->SetAsync(communicateProc);
	m_pendClientVector.push_back(newClientAsync);
	
	std::string addr(inet_ntoa(_socket->GetAddr().sin_addr));
	std::string successMessage = "接続したクライアントがいます。\nポート : " + std::to_string(ntohs(_socket->GetAddr().sin_port)) + "\nアドレス : " + addr;
	Dw_Scroll(DW_ScrollType::kUser, successMessage.c_str());
}

void CNsmNetworkServer::EndServer()
{
	m_sendBuffer.clear();
	m_recvBuffer.clear();
	StopListen();
	DisconnectAll();

	if (!m_enableServer)return;
	m_enableServer = false;
	WSACleanup();
}


