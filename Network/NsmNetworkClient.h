#pragma once


class CommunicationAsync;

class CNsmNetworkClient
{
private:
	CNsmNetworkClient();
	~CNsmNetworkClient();

public:
	// 必ずこの二つの関数は使わなければならない
	bool StartUp();
	void EndClient();

	// 接続の開始
	bool BeginConnect(const std::string& _serverlp, unsigned short _portNo);
	// 接続を完了させる
	bool CompleteConnecting();

	// ソケットの切断
	void DisConnect();

	ConnectionState GetConnectionState();
private:
	// 通信の開始
	bool BeginCommunication();

private:
	WSADATA m_wsaData;
	SPtr<CommunicationAsync> m_commAsync;
	std::vector<char> m_recvBuffer;
	std::vector<char> m_sendBuffer;

	ConnectionState m_connState;
	bool m_enableClient;

public:
	// プロパティ
	PROPERTY_G(GetConnectionState) ConnectionState connState;

	// シングルトン
public:
	static CNsmNetworkClient & GetInstance()
	{
		static CNsmNetworkClient instance;
		return instance;
	}
	CNsmNetworkClient(const CNsmNetworkClient& _instance){}
	void operator=(const CNsmNetworkClient& _instance){}
};

#define Client CNsmNetworkClient::GetInstance()