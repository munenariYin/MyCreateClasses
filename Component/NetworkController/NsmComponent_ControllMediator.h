#pragma once

class CNsmComponent_ControllMediator : public IComponent_Base
{
	/*============================*/
	// RTTI
	/*============================*/
	RTTI_DECL

public:
	CNsmComponent_ControllMediator();
	~CNsmComponent_ControllMediator();


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
		return "ControllMediator";
	}

private:
	SPtr<CComponent_BaseController> m_controller;
	ControllerData m_controllData;
};