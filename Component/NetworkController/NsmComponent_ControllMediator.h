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
	/// Json データからの設定
	///</summary>
	void InitFromJson(const json11::Json& _rJsonObj);

	///<summary>
	/// 初回の Update の直前で一度だけ実行される
	///</summary>
	void Start(void);

	///<summary>
	/// 更新
	///</summary>
	void Update(void);

	///<summary>
	/// 全てのタスクのUpdate実行後に呼ぶ更新
	///</summary>
	void LateUpdate(void);

	void Release(void);

	///<summary>
	/// デバッグ名
	///</summary>
	std::string GetDebugName(void)
	{
		return "ControllMediator";
	}

private:
	SPtr<CComponent_BaseController> m_controller;
	ControllerData m_controllData;
};