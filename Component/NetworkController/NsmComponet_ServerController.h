#pragma once

class CommunicationAsync;

class CNsmComponent_ServerController : public CComponent_BaseController
{
	/*============================*/
	// RTTI
	/*============================*/
	RTTI_DECL

public:
	CNsmComponent_ServerController();
	~CNsmComponent_ServerController();

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

	void Release(void);

	///<summary>
	/// �f�o�b�O��
	///</summary>
	std::string GetDebugName(void)
	{
		return "ServerController";
	}

public:
	void SetPortNo(unsigned short _portNo);
	void SetServer(const SPtr<CNsmComponent_Server>& _server);
	ControllerData& GetInputData();
private:

private:

private:
	unsigned short m_playerPortNo;
	ControllerData m_inputData;
	WPtr<CNsmComponent_Server> m_refServer;
};