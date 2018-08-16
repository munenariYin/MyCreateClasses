#pragma once

// =================================================
// �\�P�b�g�����b�s���O�����N���X�B
// =================================================
class Socket
{
public:
	Socket();
	~Socket();

	// ====================================================
	// �\�P�b�g�̋@�\
	// ====================================================

	// socket()�B�v���g�R���t�@�~��,�\�P�b�g�^�C�v,�v���g�R�����w�肷��B
	// TCP��UDP���́A���̎��_�Ŏw�肷��B
	bool CreateSocket(int _protocolFamily, int _type, int _protocol);
	// bind()�B�A�h���X�t�@�~��, �ڑ��ł���A�h���X�̃}�X�N, �|�[�g�ԍ����w�肷��
	bool Bind();
	// listen()�B�o�b�N���O(�҂��s��)�̍쐬
	bool Listen(int _backLog);
	// connect()�B�ڑ���҂B
	bool Connect();
	// accept()�B�����̃\�P�b�g�Ƃ̐ڑ����󂯕t����B
	bool Accept(Socket& _targetSock);
	// recv()�B������̎󂯎��B
	int Receive(char* _recvData, int _recvSize, int _flags = 0);
	// send()�B������̑��M�B
	int Send(const char* _sendData, int _sendSize, int _flags = 0);


	void Release();


	// ==================================
	// Getter
	// ==================================
	
	// �\�P�b�g
	SOCKET GetSocket();
	// �A�h���X
	sockaddr_in& GetAddr();


	// ==================================
	// Setter
	// ==================================
	
	// �\�P�b�g
	void SetSocket(SOCKET& _socket);

	// �A�h���X
	// �ڍׂɃZ�b�g����ꍇ
	void SetAddr(short addressFamily, unsigned long _inAddr, unsigned short _portNo);
	// �����̂��̂��Z�b�g����ꍇ
	void SetAddr(sockaddr_in& _addr);

	// �u���b�L���O�̉ۂ�ݒ�Bfalse�Ȃ�m���u���b�L���O�Atrue�Ȃ�u���b�L���O�B
	// �ŏ��̓u���b�L���O����Ă���B
	bool SetBlocking(bool _isBlocking);

public:
	static const int MSG_MAX = 1024;

private:
	SOCKET m_socket;
	sockaddr_in m_addr;
	bool m_isBlocking;
};

// =================================================================================
// �\�P�b�g�֘A�֗̕��֐��Q
// =================================================================================
class SocketUtil
{
public:
	static void SockError(Socket& _errorSocket, const std::string& _funcName);

	// �󂯂������񂪃A�h���X�ł����Ă��A�z�X�g�l�[���ł����Ă��ړI�̃A�h���X�ɕϊ�����B
	static unsigned long TransHostAddr(const char* _hostInfo);

	static std::vector<int> acceptErrorCodes;
	static std::vector<int> sendErrorCodes;
	static std::vector<int> recvErrorCodes;
	static std::vector<int> connectErrorCodes;

};

// �G���[�̒v���x
enum class SockErrorLevel
{
	Continuation,			// �p���\
	UnknownContinuation,	// �p���\���s��
	NotContinuation			// �p���s��
};