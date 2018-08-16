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