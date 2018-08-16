#pragma once

//typedef struct ControllerData;

class CNsmComponent_ClientController : public CComponent_BaseController
{
	/*============================*/
	// RTTI
	/*============================*/
	RTTI_DECL

public:
	CNsmComponent_ClientController();
	~CNsmComponent_ClientController();

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
		return "ClientController";
	}
	
public:

private:

private:

private:
	ControllerData m_inputData;
	SPtr<CNsmComponent_Client> m_refClient;
	std::vector<char> m_keyVector;
};