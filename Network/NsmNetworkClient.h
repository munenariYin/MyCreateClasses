#pragma once


class CommunicationAsync;

class CNsmNetworkClient
{
private:
	CNsmNetworkClient();
	~CNsmNetworkClient();

public:
	// �K�����̓�̊֐��͎g��Ȃ���΂Ȃ�Ȃ�
	bool StartUp();
	void EndClient();

	// �ڑ��̊J�n
	bool BeginConnect(const std::string& _serverlp, unsigned short _portNo);
	// �ڑ�������������
	bool CompleteConnecting();

	// �\�P�b�g�̐ؒf
	void DisConnect();

	ConnectionState GetConnectionState();
private:
	// �ʐM�̊J�n
	bool BeginCommunication();

private:
	WSADATA m_wsaData;
	SPtr<CommunicationAsync> m_commAsync;
	std::vector<char> m_recvBuffer;
	std::vector<char> m_sendBuffer;

	ConnectionState m_connState;
	bool m_enableClient;

public:
	// �v���p�e�B
	PROPERTY_G(GetConnectionState) ConnectionState connState;

	// �V���O���g��
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