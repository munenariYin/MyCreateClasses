#include"../Nisiguti.h"
// ======================================================
// Socketクラス
// ======================================================
Socket::Socket()
{
	// SOCKET生成時、初めはブロッキングされていないのでfalse。
	m_isBlocking = false;
	m_socket = INVALID_SOCKET;
}

Socket::~Socket()
{
	Release();
}

bool Socket::CreateSocket(int _protocolFamily, int _type, int _protocol)
{
	// プロトコルファミリ、ソケットタイプ、プロトコルの設定をする。
	m_socket = socket(_protocolFamily, _type, _protocol);
	// ソケットが与えられなければ失敗。
	if (m_socket == INVALID_SOCKET) return false;
	return true;
}

bool Socket::Bind()
{
	// ソケットに関連付ける
	if (bind(m_socket, (struct sockaddr*)& m_addr, sizeof m_addr) == SOCKET_ERROR) return false;
	return true;
}

bool Socket::Listen(int _backLog)
{
	// バックログ(待ち受けるクライアント)の数を決める。
	if (listen(m_socket, _backLog) == SOCKET_ERROR) return false;
	return true;
}

bool Socket::Connect()
{
	// ③サーバに接続を要求
	if (connect(m_socket, (struct sockaddr*)& m_addr, sizeof m_addr) == SOCKET_ERROR) return false;
	return true;
}

bool Socket::Accept(Socket & _targetSock)
{
	sockaddr_in clientAddr;
	int len = sizeof clientAddr;

	// 接続を試行。
	_targetSock.m_socket = accept(m_socket, (struct sockaddr*)&clientAddr, &len);
	// ソケットが渡されなければエラーであるということ。falseを返す。
	if (_targetSock.m_socket == INVALID_SOCKET)	return false;

	// acceptで得たクライアントのアドレスを入れて返す。
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
	// ソケットの終了
	shutdown(m_socket, SD_BOTH);
	closesocket(m_socket);
	m_isBlocking = false;
}

// ================================================
// Getter
// ================================================
// ソケット
SOCKET Socket::GetSocket()
{
	return m_socket;
}

// アドレス
sockaddr_in & Socket::GetAddr()
{
	return m_addr;
}

// ================================================
// Setter
// ================================================
// ソケット
void Socket::SetSocket(SOCKET & _socket)
{
	m_socket = _socket;
}

// アドレス
// 詳細に設定
void Socket::SetAddr(short addressFamily, unsigned long _inAddr, unsigned short _portNo)
{
	memset(&m_addr, 0, sizeof m_addr);		// ゼロクリア
	m_addr.sin_family = addressFamily;		// アドレスファミリ
	m_addr.sin_addr.s_addr = _inAddr;		// IPアドレス
	m_addr.sin_port = htons(_portNo);		// ポート番号
}

// 既存のものをセット
void Socket::SetAddr(sockaddr_in & _addr)
{
	m_addr = _addr;
}

// ブロッキングの可否をセット
bool Socket::SetBlocking(bool _isBlocking)
{
	unsigned long val;
	// ブロッキングすると0に、そうでなければ1に
	_isBlocking ? val = 0 : val = 1;
	// エラーが返されれば失敗。
	if (ioctlsocket(m_socket, FIONBIO, &val) == SOCKET_ERROR) return false;

	this->m_isBlocking = _isBlocking;
	return true;
}

// ===================================================
// SocketUtilクラス
// ===================================================
void SocketUtil::SockError(Socket& _errorSocket, const std::string& _funcName)
{
	fprintf(stderr, "Error Code = %d", WSAGetLastError());	// エラーコードの表示
	fprintf(stderr, _funcName.c_str(), _funcName);	// エラー関数の表示
}

unsigned long SocketUtil::TransHostAddr(const char* _hostInfo)
{
	struct hostent *phe;
	unsigned long ipAddr = inet_addr(_hostInfo);
	if (ipAddr == INADDR_NONE)		// INADDR_NONEはアドレスではないことを示す。
	{
		phe = gethostbyname(_hostInfo);	// ホスト名として処理
	}
	else // ホストアドレスだった場合
	{
		phe = gethostbyaddr((char*)&ipAddr, 4, AF_INET);	// addressとして処理
	}
	if (phe == NULL)
	{
		return 0UL;
	}
#ifdef _DEBUG
	//printf("HostName\t: %s\n", phe->h_name);						// 公式名
	//for (int i = 0; phe->h_aliases[i] != NULL; i++)
	//{
	//	printf("Aliase Name[%d]\t: %hu\n", i, phe->h_aliases[i]);	// 別名
	//}
	//printf("Address Type\t: %hu", phe->h_addrtype);					// アドレス型
	//printf("Address Length\t %hu\n", phe->h_length);				// アドレス長
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