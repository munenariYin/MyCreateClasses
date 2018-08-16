#pragma once

// =================================================
// ソケットをラッピングしたクラス。
// =================================================
class Socket
{
public:
	Socket();
	~Socket();

	// ====================================================
	// ソケットの機能
	// ====================================================

	// socket()。プロトコルファミリ,ソケットタイプ,プロトコルを指定する。
	// TCPかUDPかは、この時点で指定する。
	bool CreateSocket(int _protocolFamily, int _type, int _protocol);
	// bind()。アドレスファミリ, 接続できるアドレスのマスク, ポート番号を指定する
	bool Bind();
	// listen()。バックログ(待ち行列)の作成
	bool Listen(int _backLog);
	// connect()。接続を待つ。
	bool Connect();
	// accept()。引数のソケットとの接続を受け付ける。
	bool Accept(Socket& _targetSock);
	// recv()。文字列の受け取り。
	int Receive(char* _recvData, int _recvSize, int _flags = 0);
	// send()。文字列の送信。
	int Send(const char* _sendData, int _sendSize, int _flags = 0);


	void Release();


	// ==================================
	// Getter
	// ==================================
	
	// ソケット
	SOCKET GetSocket();
	// アドレス
	sockaddr_in& GetAddr();


	// ==================================
	// Setter
	// ==================================
	
	// ソケット
	void SetSocket(SOCKET& _socket);

	// アドレス
	// 詳細にセットする場合
	void SetAddr(short addressFamily, unsigned long _inAddr, unsigned short _portNo);
	// 既存のものをセットする場合
	void SetAddr(sockaddr_in& _addr);

	// ブロッキングの可否を設定。falseならノンブロッキング、trueならブロッキング。
	// 最初はブロッキングされている。
	bool SetBlocking(bool _isBlocking);

public:
	static const int MSG_MAX = 1024;

private:
	SOCKET m_socket;
	sockaddr_in m_addr;
	bool m_isBlocking;
};

// =================================================================================
// ソケット関連の便利関数群
// =================================================================================
class SocketUtil
{
public:
	static void SockError(Socket& _errorSocket, const std::string& _funcName);

	// 受けた文字列がアドレスであっても、ホストネームであっても目的のアドレスに変換する。
	static unsigned long TransHostAddr(const char* _hostInfo);

	static std::vector<int> acceptErrorCodes;
	static std::vector<int> sendErrorCodes;
	static std::vector<int> recvErrorCodes;
	static std::vector<int> connectErrorCodes;

};

// エラーの致命度
enum class SockErrorLevel
{
	Continuation,			// 継続可能
	UnknownContinuation,	// 継続可能か不明
	NotContinuation			// 継続不可
};