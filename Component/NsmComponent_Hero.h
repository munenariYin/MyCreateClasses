#pragma once

class CNsmComponent_Hero : public IComponent_Base
{
public:
	CNsmComponent_Hero();
	~CNsmComponent_Hero();
	
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
		return "NsmHero";
	}

private:
	WPtr<CNsmComponent_ServerController> m_refController;
	SPtr<CComponent_Transform> m_transform;
};