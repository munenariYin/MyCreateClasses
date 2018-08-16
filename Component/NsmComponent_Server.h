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
	// ポート番号とコントローラデータのvector。
	// ソートされると困るので、登録された順序が分かるvectorを使用。
	std::vector<std::pair<unsigned short, ControllerData>> m_playersControllerData;
};