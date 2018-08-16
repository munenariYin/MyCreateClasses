#pragma once

class CommunicationAsync;

class CNsmComponent_Server : public IComponent_Base
{


	/*============================*/
	// RTTI
	/*============================*/
	RTTI_DECL

public:

	CNsmComponent_Server();
	virtual ~CNsmComponent_Server();

	///<summary>
	/// Json �f�[�^����̐ݒ�
	///</summary>
	void InitFromJson(const json11::Json& _rJsonObj);

	///<summary>
	/// ����� Update �̒��O�ň�x�������s�����
	///</summary>
	void Start(void);

	///<summary>
	/// �X�V
	///</summary>
	void Update(void);

	///<summary>
	/// �S�Ẵ^�X�N��Update���s��ɌĂԍX�V
	///</summary>
	void LateUpdate(void);

	void Draw(void);

	void Release(void);

	///<summary>
	/// �f�o�b�O��
	///</summary>
	std::string GetDebugName(void)
	{
		return "Server";
	}

public:	
	bool BeginAccept();
	void InputControllerData(ControllerData& _outControllData, short _portNo);
private:
	void CreateClientAsync(SPtr<Socket>& _socket);

public:

private:
	unsigned int m_frameCnt;
	WSADATA m_wsaData;
	SPtr<CommunicationAsync> m_acceptAsync;
	std::vector<std::pair<SPtr<CommunicationAsync>, WPtr<CGameObject>>> m_clientVector;
	unsigned int m_clientCntMax = 5;
	unsigned short m_portNo = 50000;

	bool m_isEndServer;
	bool m_isAccepting;

	std::vector<char> m_recvBuffer;
	std::vector<char> m_sendBuffer;
	// �|�[�g�ԍ��ƃR���g���[���f�[�^��vector�B
	// �\�[�g�����ƍ���̂ŁA�o�^���ꂽ������������vector���g�p�B
	std::vector<std::pair<unsigned short, ControllerData>> m_playersControllerData;
};