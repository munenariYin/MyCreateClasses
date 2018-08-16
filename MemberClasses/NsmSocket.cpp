#include"../Nisiguti.h"
// ======================================================
// Socket�N���X
// ======================================================
Socket::Socket()
{
	// SOCKET�������A���߂̓u���b�L���O����Ă��Ȃ��̂�false�B
	m_isBlocking = false;
	m_socket = INVALID_SOCKET;
}

Socket::~Socket()
{
	Release();
}

bool Socket::CreateSocket(int _protocolFamily, int _type, int _protocol)
{
	// �v���g�R���t�@�~���A�\�P�b�g�^�C�v�A�v���g�R���̐ݒ������B
	m_socket = socket(_protocolFamily, _type, _protocol);
	// �\�P�b�g���^�����Ȃ���Ύ��s�B
	if (m_socket == INVALID_SOCKET) return false;
	return true;
}

bool Socket::Bind()
{
	// �\�P�b�g�Ɋ֘A�t����
	if (bind(m_socket, (struct sockaddr*)& m_addr, sizeof m_addr) == SOCKET_ERROR) return false;
	return true;
}

bool Socket::Listen(int _backLog)
{
	// �o�b�N���O(�҂��󂯂�N���C�A���g)�̐������߂�B
	if (listen(m_socket, _backLog) == SOCKET_ERROR) return false;
	return true;
}

bool Socket::Connect()
{
	// �B�T�[�o�ɐڑ���v��
	if (connect(m_socket, (struct sockaddr*)& m_addr, sizeof m_addr) == SOCKET_ERROR) return false;
	return true;
}

bool Socket::Accept(Socket & _targetSock)
{
	sockaddr_in clientAddr;
	int len = sizeof clientAddr;

	// �ڑ������s�B
	_targetSock.m_socket = accept(m_socket, (struct sockaddr*)&clientAddr, &len);
	// �\�P�b�g���n����Ȃ���΃G���[�ł���Ƃ������ƁBfalse��Ԃ��B
	if (_targetSock.m_socket == INVALID_SOCKET)	return false;

	// accept�œ����N���C�A���g�̃A�h���X�����ĕԂ��B
	_targetSock.m_addr = clientAddr;
	return true;
}

int Socket::Receive(char* _recvData, int _recvSize, int _flags)
{
	return recv(m_socket, _recvData, _recvSize, _flags);
}

int Socket::Send(const char* _sendData, int _sendSize, int _flags)
{
	return send(m_socket, _sendData, _sendSize, _flags);
}

void Socket::Release()
{
	// �\�P�b�g�̏I��
	shutdown(m_socket, SD_BOTH);
	closesocket(m_socket);
	m_isBlocking = false;
}

// ================================================
// Getter
// ================================================
// �\�P�b�g
SOCKET Socket::GetSocket()
{
	return m_socket;
}

// �A�h���X
sockaddr_in & Socket::GetAddr()
{
	return m_addr;
}

// ================================================
// Setter
// ================================================
// �\�P�b�g
void Socket::SetSocket(SOCKET & _socket)
{
	m_socket = _socket;
}

// �A�h���X
// �ڍׂɐݒ�
void Socket::SetAddr(short addressFamily, unsigned long _inAddr, unsigned short _portNo)
{
	memset(&m_addr, 0, sizeof m_addr);		// �[���N���A
	m_addr.sin_family = addressFamily;		// �A�h���X�t�@�~��
	m_addr.sin_addr.s_addr = _inAddr;		// IP�A�h���X
	m_addr.sin_port = htons(_portNo);		// �|�[�g�ԍ�
}

// �����̂��̂��Z�b�g
void Socket::SetAddr(sockaddr_in & _addr)
{
	m_addr = _addr;
}

// �u���b�L���O�̉ۂ��Z�b�g
bool Socket::SetBlocking(bool _isBlocking)
{
	unsigned long val;
	// �u���b�L���O�����0�ɁA�����łȂ����1��
	_isBlocking ? val = 0 : val = 1;
	// �G���[���Ԃ����Ύ��s�B
	if (ioctlsocket(m_socket, FIONBIO, &val) == SOCKET_ERROR) return false;

	this->m_isBlocking = _isBlocking;
	return true;
}

// ===================================================
// SocketUtil�N���X
// ===================================================
void SocketUtil::SockError(Socket& _errorSocket, const std::string& _funcName)
{
	fprintf(stderr, "Error Code = %d", WSAGetLastError());	// �G���[�R�[�h�̕\��
	fprintf(stderr, _funcName.c_str(), _funcName);	// �G���[�֐��̕\��
}

unsigned long SocketUtil::TransHostAddr(const char* _hostInfo)
{
	struct hostent *phe;
	unsigned long ipAddr = inet_addr(_hostInfo);
	if (ipAddr == INADDR_NONE)		// INADDR_NONE�̓A�h���X�ł͂Ȃ����Ƃ������B
	{
		phe = gethostbyname(_hostInfo);	// �z�X�g���Ƃ��ď���
	}
	else // �z�X�g�A�h���X�������ꍇ
	{
		phe = gethostbyaddr((char*)&ipAddr, 4, AF_INET);	// address�Ƃ��ď���
	}
	if (phe == NULL)
	{
		return 0UL;
	}
#ifdef _DEBUG
	//printf("HostName\t: %s\n", phe->h_name);						// ������
	//for (int i = 0; phe->h_aliases[i] != NULL; i++)
	//{
	//	printf("Aliase Name[%d]\t: %hu\n", i, phe->h_aliases[i]);	// �ʖ�
	//}
	//printf("Address Type\t: %hu", phe->h_addrtype);					// �A�h���X�^
	//printf("Address Length\t %hu\n", phe->h_length);				// �A�h���X��
	//for (int i = 0; phe->h_addr_list[i]; i++)
	//{
	//	printf("IP address[%d]\t: %s\n", i, inet_ntoa(*(struct in_addr*)phe->h_addr_list[i]));
	//}
#endif
	if (*phe->h_addr_list == NULL)
	{
		return 0UL;
	}
	return *(unsigned long*)*phe->h_addr_list;
}