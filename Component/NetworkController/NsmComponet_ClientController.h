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