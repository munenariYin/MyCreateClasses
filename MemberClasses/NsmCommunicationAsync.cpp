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
	// �����_��񓯊��p�ɕϊ�����unique_ptr�ɑΉ�������B
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
	// ���o�b�t�@���Ǝ��ɒǉ�����f�[�^���̃T�C�Y��
	// ���T�C�Y���ăf�[�^��ǉ�����B
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
	// �\�P�b�g�̏I��
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
		// �w�b�_�̃T�C�Y�ȉ��ł���΁A�܂��f�[�^�ƌĂׂ���̂͑��݂��Ȃ��B
		if (bufferSize <= headerSize) break;
		
		// �w�b�_����T�C�Y���m�F����
		HeaderData headerData;
		::memcpy(reinterpret_cast<char*>(&headerData), &m_recvBuffer[0], headerSize);
		size_t dataSize = headerSize + headerData.dataSize;
		// �w�b�_�� �{ �f�[�^���̃T�C�Y�ɑ���Ȃ���΃f�[�^�����܂������Ă���B
		if (bufferSize < dataSize) break;
		
		// �m�肵���f�[�^���̈ړ�
		AddBuffer(_destBuffer, &m_recvBuffer[0], dataSize);
		//size_t destBufferSize = _destBuffer.size();
		//_destBuffer.resize(destBufferSize + dataSize);
		//::memcpy(&_destBuffer[destBufferSize], &m_recvBuffer[0], dataSize);
		// �ړ��������������̕��͍폜����
		m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + dataSize);
	}
}

CommDataState CommunicationAsync::Send()
{
	// �o�b�t�@�ɉ����Ȃ��A�������̓\�P�b�g���̂����݂��Ȃ����͉������Ȃ��B
	if (m_socket == nullptr) return CommDataState::Failed;
	if (m_sendBuffer.empty()) return CommDataState::NoneData;

	// �o�b�t�@����f�[�^�𑗂�A�������������̃T�C�Y���o�b�t�@�������
	int sendSize = m_socket->Send(&m_sendBuffer[0], m_sendBuffer.size());
	if (sendSize > 0)
	{
		m_sendBuffer.erase(m_sendBuffer.begin(), m_sendBuffer.begin() + sendSize);
		return CommDataState::Success;
	}
	// ���M�Ɏ��s����
	if(sendSize == SOCKET_ERROR) return CommDataState::Failed;
	// ���M����f�[�^����������
	return CommDataState::NoneData;
}

CommDataState CommunicationAsync::Receive()
{
	int recvSize = 0;
	int sumRecvSize = 0;
	char recvData[Socket::MSG_MAX];
	// �󂯎��T�C�Y��0�ɂȂ�܂�recv���J��Ԃ�
	while (true)
	{
		recvSize = m_socket->Receive(recvData, Socket::MSG_MAX);
		// �󂯎�����T�C�Y��0�Ȃ牽���󂯎���Ă��Ȃ��̂Ń��[�v�𔲂���
		if (recvSize <= 0) break;
		sumRecvSize += recvSize;
		// �󂯎�����T�C�Y�̕������o�b�t�@�̌��ɒǉ�����
		AddBuffer(m_recvBuffer, recvData, recvSize);
	}
	// �G���[���o�Ă���Ύ��s
	if (recvSize == SOCKET_ERROR) return CommDataState::Failed;
	// ������M���ĂȂ��ꍇ
	if (sumRecvSize == 0) return CommDataState::NoneData;
	return CommDataState::Success;
}
