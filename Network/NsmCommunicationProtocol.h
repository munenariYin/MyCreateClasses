#pragma once

// =======================================================
// 通信の約束事を決める。
// 誰に、どんな型で、どんな値を送るのか。
// これを文字列に変換して送受信する。
// =======================================================

//
enum class CommunicateDataType : unsigned char
{
	Void,	// void型
	Int,
	Float,
	Char,
	Long,
	Double,
	Matrix,
	ControllerData
};

// 送信データの先頭に付与する情報
struct HeaderData
{
	unsigned int dataSize;
	unsigned short dataType;
	unsigned short target;


	HeaderData()
	{
		dataSize = 0;
		dataType = 0;
		target = 0;
	}
};

class ProtocolData
{
	ProtocolData(){}
	~ProtocolData(){}

	
};

// 入力データ
struct ControllerData
{
	unsigned int buttons;
	YsVec2 rightAxis;
	YsVec2 leftAxis;

	ControllerData()
	{
		buttons = 0;
		rightAxis = YsVec2::Zero;
		leftAxis = YsVec2::Zero;
	}
};

struct PlayerData
{
	float frame;
	YsMatrix cameraMatrix;
};

struct ObjectData
{
	unsigned int	id;
	YsMatrix		matrix;

	ObjectData()
	{
		id = 0;
		matrix = YsMatrix::Identity;
	}
};

class NsmCommunicationProtocol
{
public:
	NsmCommunicationProtocol(){}
	~NsmCommunicationProtocol(){}

	static void Init();

	template<typename T>
	static void MakeHeaderData(const T& _data, HeaderData& _outHeaderData)
	{
		if (!m_isInitialized) Init();

		// ヘッダのサイズを取得する
		_outHeaderData.dataSize = sizeof(_data);

		// 送信したいデータの型を取得する
		size_t dataType = typeid(_data).hash_code();
		// mapの中から該当する型を探す
		for (auto it = m_rttiMap.begin(); it != m_rttiMap.end(); ++it)
		{
			if (dataType == it->second)
			{
				_outHeaderData.dataType = static_cast<unsigned char>(it->first);
				return;
			}
		}
	}

private:
	// 型名を集めたもの
	static std::map<CommunicateDataType, size_t> m_rttiMap;
	static bool m_isInitialized;
};

