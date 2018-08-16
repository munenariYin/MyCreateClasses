#pragma once

class CommunicationAsync;

class CNsmNetworkServer
{
private:
	CNsmNetworkServer();
	~CNsmNetworkServer();
public:
	// �l�b�g���[�N���g�p����ۂɕK�����p���Ȃ���΂Ȃ�Ȃ�
	bool StartUp();
	void EndServer();

	// �V���ȃN���C�A���g�̎�t���s��
	bool Listen(unsigned long _inAddr, unsigned short _port);
	void StopListen();

	// �o�^�҂��N���C�A���g��S�ēo�^
	void ConnectAllPendClients();

	// �ڑ����؂ꂽ�N���C�A���g���O��
	void RemoveDisconnectedClients();
	// �S�ẴN���C�A���g�Ƃ̐ڑ���؂�
	void DisconnectAll();


private:
	// �o�^�҂��N���C�A���g���쐬
	void CreateClientAsync(SPtr<Socket>& _socket);

private:
	WSADATA m_wsaData;
	SPtr<CommunicationAsync> m_acceptAsync;
	std::vector<SPtr<CommunicationAsync>> m_pendClientVector;
	std::vector<SPtr<CommunicationAsync>> m_clientVector;
	unsigned short m_portNo = 0;

	// ����M����p�o�b�t�@
	std::vector<char> m_recvBuffer;
	std::vector<char> m_sendBuffer;

	bool m_enableAccept = false;
	bool m_enableServer = false;
public:
	// �V���O���g��
	static CNsmNetworkServer& GetInstance()
	{
		static CNsmNetworkServer intance;
		return intance;
	}
	CNsmNetworkServer(const CNsmNetworkServer& _instance){}
	void operator=(const CNsmNetworkServer& _instance){}


};

#define Server CNsmNetworkServer::GetInstance()