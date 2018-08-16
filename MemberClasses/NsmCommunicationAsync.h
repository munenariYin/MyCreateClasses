#pragma once

enum class CommDataState
{
	NoneData,
	Success,
	Failed
};

// =============================================================
// ラップクラスのSokcetを用いた、非同期処理に対応させたクラス
// =============================================================

class CommunicationAsync
{
public:
	CommunicationAsync();
	~CommunicationAsync();

	// 待機
	bool WaitEndAsync();
	// 受付用ソケットの用意
	bool PrepareAcceptSocket(int _protocolFamily, int _type, int _protocol, short _addressFamily, unsigned long _inAddr, unsigned short _portNo);
	bool PrepareConnectSocket(int _protocolFamily, int _type, int _protocol, short _addressFamily, unsigned long _inAddr, unsigned short _portNo);

	void Clear();
	bool ClearSendBuffer();
	bool ClearRecvBuffer();


	// 先頭に4バイトでデータサイズを、次の2バイトにデータ型を示すenum値を、
	//その後にデータを付与して格納する
	template<typename SendType>
	void AddSendBuffer(SendType& _sendData)
	{
		// ヘッダ情報の作成
		HeaderData header;
		NsmCommunicationProtocol::MakeHeaderData(_sendData, header);
		char* headerData = reinterpret_cast<char*>(&header);
		AddBuffer(m_sendBuffer, headerData, sizeof(header));

		// 本データの格納
		char* data = reinterpret_cast<char*>(&_sendData);
		AddBuffer(m_sendBuffer, data, header.dataSize);
	}
	// 先頭にヘッダの情報が確実にあることが前提。
	// 他のバッファへ、現在データであると確認できる内容を全て移す。
	void MoveRecvData(std::vector<char>& _destBuffer);
	CommDataState Send();
	// 受け取ったデータをバッファに加える
	CommDataState Receive();
	// ============================================
	// Setter
	// ============================================
	// ソケット
	void SetSocket(SPtr<Socket>& _socket);
	// 非同期処理
	// ラムダを作って渡す
	void SetAsync(std::function<bool()>& _function);
	// 既存の非同期処理を渡す
	void SetAsync(std::future<bool>& _thread);
	// 終了のセット
	void SetEnableAsync(bool _isEnable);
	// エラーコードのセット
	int UpdateErrorCode();

	// ============================================
	// Getter
	// ============================================
	// ソケット
	SPtr<Socket>& GetSocket();
	// エラーコード
	int GetErrorCode();
	// 非同期処理を終了するか
	bool IsEnableAsync();
	// 非同期処理が存在するか
	bool IsValidAsync();

private:
	// バッファにデータを追加する
	void AddBuffer(std::vector<char>& _buffer, const char* _data, size_t _size);

private:
	// 利用するソケット
	SPtr<Socket> m_socket;
	int m_errorCode;

	// 非同期処理用
	std::future<bool> m_async;
	// 非同期を外部から終了させる値。ラムダのキャプチャにこれを入れること
	bool m_isEnableAsync;

	// 送受信バッファ
	std::vector<char> m_sendBuffer;
	bool m_isLockSend;
	std::vector<char> m_recvBuffer;
	bool m_isLockRecv;
};
