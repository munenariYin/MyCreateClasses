#pragma once

class CommunicationAsync;

class CNsmNetworkServer
{
private:
	CNsmNetworkServer();
	~CNsmNetworkServer();
public:
	// ネットワークを使用する際に必ず利用しなければならない
	bool StartUp();
	void EndServer();

	// 新たなクライアントの受付を行う
	bool Listen(unsigned long _inAddr, unsigned short _port);
	void StopListen();

	// 登録待ちクライアントを全て登録
	void ConnectAllPendClients();

	// 接続が切れたクライアントを外す
	void RemoveDisconnectedClients();
	// 全てのクライアントとの接続を切る
	void DisconnectAll();


private:
	// 登録待ちクライアントを作成
	void CreateClientAsync(SPtr<Socket>& _socket);

private:
	WSADATA m_wsaData;
	SPtr<CommunicationAsync> m_acceptAsync;
	std::vector<SPtr<CommunicationAsync>> m_pendClientVector;
	std::vector<SPtr<CommunicationAsync>> m_clientVector;
	unsigned short m_portNo = 0;

	// 送受信仲介用バッファ
	std::vector<char> m_recvBuffer;
	std::vector<char> m_sendBuffer;

	bool m_enableAccept = false;
	bool m_enableServer = false;
public:
	// シングルトン
	static CNsmNetworkServer& GetInstance()
	{
		static CNsmNetworkServer intance;
		return intance;
	}
	CNsmNetworkServer(const CNsmNetworkServer& _instance){}
	void operator=(const CNsmNetworkServer& _instance){}


};

#define Server CNsmNetworkServer::GetInstance()