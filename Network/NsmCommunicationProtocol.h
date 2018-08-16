#pragma once

// =======================================================
// �ʐM�̖񑩎������߂�B
// �N�ɁA�ǂ�Ȍ^�ŁA�ǂ�Ȓl�𑗂�̂��B
// ����𕶎���ɕϊ����đ���M����B
// =======================================================

//
enum class CommunicateDataType : unsigned char
{
	Void,	// void�^
	Int,
	Float,
	Char,
	Long,
	Double,
	Matrix,
	ControllerData
};

// ���M�f�[�^�̐擪�ɕt�^������
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

// ���̓f�[�^
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

		// �w�b�_�̃T�C�Y���擾����
		_outHeaderData.dataSize = sizeof(_data);

		// ���M�������f�[�^�̌^���擾����
		size_t dataType = typeid(_data).hash_code();
		// map�̒�����Y������^��T��
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
	// �^�����W�߂�����
	static std::map<CommunicateDataType, size_t> m_rttiMap;
	static bool m_isInitialized;
};

