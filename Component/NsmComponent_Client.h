#pragma once

class CommunicationAsync;

enum class ConnectionState
{
	NotConnection,
	WaitConnection,
	FailedConnection,
	Connection,
	Communication
};
class CNsmComponent_Client : public IComponent_Base
{
	/*============================*/
	// RTTI
	/*============================*/
	RTTI_DECL

public:
	CNsmComponent_Client();
	~CNsmComponent_Client();

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

	void Draw(void);

	void Release(void);

	///<summary>
	/// デバッグ名
	///</summary>
	std::string GetDebugName(void)
	{
		return "Client";
	}

public:
	bool BeginConnect();
	void AddInputDataBuffer(ControllerData& _inputData);
private:

private:
	void WaitConnect();
	bool Connect();
	void CreateCommunicateAsync();
	ConnectionState GetConnectionState();

private:
	WSADATA m_wsaData;
	SPtr<CommunicationAsync> m_commAsync;
	unsigned short m_portNo = 0;

	ConnectionState m_connectionState = ConnectionState::NotConnection;
	bool m_isTerminateApp = false;

	std::vector<char> m_recvBuffer;
};