#include"../Nisiguti.h"
#include"../MemberClasses/NsmCommunicationAsync.h"


// ===============================================================
// class:CommunicationAsync
// ===============================================================
CommunicationAsync::CommunicationAsync()
{
	m_socket = std::make_shared<Socket>();
	m_errorCode = 0;
	m_isEnableAsync = false;
}

CommunicationAsync::~CommunicationAsync()
{
	m_socket->Release();
	m_socket = nullptr;
}

// Setter
void CommunicationAsync::SetAsync(std::function<bool()>& _function)
{
	// ラムダを非同期用に変換してunique_ptrに対応させる。
	m_async = std::async(std::launch::async, _function);
	m_isEnableAsync = true;
}

void CommunicationAsync::SetAsync(std::future<bool>& _async)
{
	m_async = std::move(_async);
	m_isEnableAsync = true;
}

void CommunicationAsync::SetEnableAsync(bool _isEnable)
{
	m_isEnableAsync = _isEnable;
}

void CommunicationAsync::SetSocket(SPtr<Socket>& _socket)
{
	m_socket = _socket;
}


int CommunicationAsync::UpdateErrorCode()
{
	m_errorCode = WSAGetLastError();
	return m_errorCode;
}

// Getter
SPtr<Socket>& CommunicationAsync::GetSocket()
{
	return m_socket;
}


int CommunicationAsync::GetErrorCode()
{
	return m_errorCode;
}

bool CommunicationAsync::IsEnableAsync()
{
	return m_isEnableAsync;
}

bool CommunicationAsync::IsValidAsync()
{
	return m_async.valid();
}

void CommunicationAsync::AddBuffer(
	std::vector<char>& _buffer, const char* _data, size_t _dataSize)
{
	// 現バッファ分と次に追加するデータ分のサイズに
	// リサイズしてデータを追加する。
	size_t bufferSize = _buffer.size();
	_buffer.resize(bufferSize + _dataSize);
	::memcpy(&_buffer[bufferSize], _data, _dataSize);
}
bool CommunicationAsync::WaitEndAsync()
{
	m_isEnableAsync = false;
	if(m_async.valid())	return m_async.get();
	return true;
}

bool CommunicationAsync::PrepareAcceptSocket(int _protocolFamily, int _type, int _protocol, short _addressFamily, unsigned long _inAddr, unsigned short _portNo)
{
	if (!m_socket->CreateSocket(_protocolFamily, _type, _protocol))return false;
	m_socket->SetAddr(_addressFamily, _inAddr, _portNo);
	if (!m_socket->Bind())return false;
	if (!m_socket->Listen(SOMAXCONN))return false;
	if (!m_socket->SetBlocking(false))return false;
	return true;
}

bool CommunicationAsync::PrepareConnectSocket(int _protocolFamily, int _type, int _protocol, short _addressFamily, unsigned long _inAddr, unsigned short _portNo)
{
	if (!m_socket->CreateSocket(_protocolFamily, _type, _protocol))return false;
	m_socket->SetAddr(_addressFamily, _inAddr, _portNo);
	if (!m_socket->SetBlocking(false))return false;

	return true;
}

void CommunicationAsync::Clear()
{
	// ソケットの終了
	m_socket->Release();

	m_errorCode = 0;
	m_isEnableAsync = false;
}

bool CommunicationAsync::ClearSendBuffer()
{
	if (m_isLockSend) return false;
	m_isLockSend = false;
	m_sendBuffer.clear();
	return true;
}

bool CommunicationAsync::ClearRecvBuffer()
{
	if (m_isLockRecv) return false;
	m_isLockRecv = false;
	m_recvBuffer.clear();
	return true;
}

void CommunicationAsync::MoveRecvData(std::vector<char>& _destBuffer)
{
	std::vector<char> moveBuffer;
	size_t headerSize = sizeof(HeaderData);
	while (true)
	{
		size_t bufferSize = m_recvBuffer.size();
		// ヘッダのサイズ以下であれば、まずデータと呼べるものは存在しない。
		if (bufferSize <= headerSize) break;
		
		// ヘッダからサイズを確認する
		HeaderData headerData;
		::memcpy(reinterpret_cast<char*>(&headerData), &m_recvBuffer[0], headerSize);
		size_t dataSize = headerSize + headerData.dataSize;
		// ヘッダ部 ＋ データ部のサイズに足りなければデータ部がまだ欠けている。
		if (bufferSize < dataSize) break;
		
		// 確定したデータ分の移動
		AddBuffer(_destBuffer, &m_recvBuffer[0], dataSize);
		//size_t destBufferSize = _destBuffer.size();
		//_destBuffer.resize(destBufferSize + dataSize);
		//::memcpy(&_destBuffer[destBufferSize], &m_recvBuffer[0], dataSize);
		// 移動した分だけ元の方は削除する
		m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + dataSize);
	}
}

CommDataState CommunicationAsync::Send()
{
	// バッファに何もない、もしくはソケット自体が存在しない時は何もしない。
	if (m_socket == nullptr) return CommDataState::Failed;
	if (m_sendBuffer.empty()) return CommDataState::NoneData;

	// バッファからデータを送り、送った分だけのサイズをバッファから消す
	int sendSize = m_socket->Send(&m_sendBuffer[0], m_sendBuffer.size());
	if (sendSize > 0)
	{
		m_sendBuffer.erase(m_sendBuffer.begin(), m_sendBuffer.begin() + sendSize);
		return CommDataState::Success;
	}
	// 送信に失敗した
	if(sendSize == SOCKET_ERROR) return CommDataState::Failed;
	// 送信するデータが無かった
	return CommDataState::NoneData;
}

CommDataState CommunicationAsync::Receive()
{
	int recvSize = 0;
	int sumRecvSize = 0;
	char recvData[Socket::MSG_MAX];
	// 受け取るサイズが0になるまでrecvを繰り返す
	while (true)
	{
		recvSize = m_socket->Receive(recvData, Socket::MSG_MAX);
		// 受け取ったサイズが0なら何も受け取っていないのでループを抜ける
		if (recvSize <= 0) break;
		sumRecvSize += recvSize;
		// 受け取ったサイズの分だけバッファの後ろに追加する
		AddBuffer(m_recvBuffer, recvData, recvSize);
	}
	// エラーが出ていれば失敗
	if (recvSize == SOCKET_ERROR) return CommDataState::Failed;
	// 何も受信してない場合
	if (sumRecvSize == 0) return CommDataState::NoneData;
	return CommDataState::Success;
}
