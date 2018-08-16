#pragma once

enum class CommDataState
{
	NoneData,
	Success,
	Failed
};

// =============================================================
// ���b�v�N���X��Sokcet��p�����A�񓯊������ɑΉ��������N���X
// =============================================================

class CommunicationAsync
{
public:
	CommunicationAsync();
	~CommunicationAsync();

	// �ҋ@
	bool WaitEndAsync();
	// ��t�p�\�P�b�g�̗p��
	bool PrepareAcceptSocket(int _protocolFamily, int _type, int _protocol, short _addressFamily, unsigned long _inAddr, unsigned short _portNo);
	bool PrepareConnectSocket(int _protocolFamily, int _type, int _protocol, short _addressFamily, unsigned long _inAddr, unsigned short _portNo);

	void Clear();
	bool ClearSendBuffer();
	bool ClearRecvBuffer();


	// �擪��4�o�C�g�Ńf�[�^�T�C�Y���A����2�o�C�g�Ƀf�[�^�^������enum�l���A
	//���̌�Ƀf�[�^��t�^���Ċi�[����
	template<typename SendType>
	void AddSendBuffer(SendType& _sendData)
	{
		// �w�b�_���̍쐬
		HeaderData header;
		NsmCommunicationProtocol::MakeHeaderData(_sendData, header);
		char* headerData = reinterpret_cast<char*>(&header);
		AddBuffer(m_sendBuffer, headerData, sizeof(header));

		// �{�f�[�^�̊i�[
		char* data = reinterpret_cast<char*>(&_sendData);
		AddBuffer(m_sendBuffer, data, header.dataSize);
	}
	// �擪�Ƀw�b�_�̏�񂪊m���ɂ��邱�Ƃ��O��B
	// ���̃o�b�t�@�ցA���݃f�[�^�ł���Ɗm�F�ł�����e��S�Ĉڂ��B
	void MoveRecvData(std::vector<char>& _destBuffer);
	CommDataState Send();
	// �󂯎�����f�[�^���o�b�t�@�ɉ�����
	CommDataState Receive();
	// ============================================
	// Setter
	// ============================================
	// �\�P�b�g
	void SetSocket(SPtr<Socket>& _socket);
	// �񓯊�����
	// �����_������ēn��
	void SetAsync(std::function<bool()>& _function);
	// �����̔񓯊�������n��
	void SetAsync(std::future<bool>& _thread);
	// �I���̃Z�b�g
	void SetEnableAsync(bool _isEnable);
	// �G���[�R�[�h�̃Z�b�g
	int UpdateErrorCode();

	// ============================================
	// Getter
	// ============================================
	// �\�P�b�g
	SPtr<Socket>& GetSocket();
	// �G���[�R�[�h
	int GetErrorCode();
	// �񓯊��������I�����邩
	bool IsEnableAsync();
	// �񓯊����������݂��邩
	bool IsValidAsync();

private:
	// �o�b�t�@�Ƀf�[�^��ǉ�����
	void AddBuffer(std::vector<char>& _buffer, const char* _data, size_t _size);

private:
	// ���p����\�P�b�g
	SPtr<Socket> m_socket;
	int m_errorCode;

	// �񓯊������p
	std::future<bool> m_async;
	// �񓯊����O������I��������l�B�����_�̃L���v�`���ɂ�������邱��
	bool m_isEnableAsync;

	// ����M�o�b�t�@
	std::vector<char> m_sendBuffer;
	bool m_isLockSend;
	std::vector<char> m_recvBuffer;
	bool m_isLockRecv;
};
