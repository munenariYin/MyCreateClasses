#include"../Nisiguti.h"
bool NsmCommunicationProtocol::m_isInitialized = false;
std::map<CommunicateDataType, size_t> NsmCommunicationProtocol::m_rttiMap = {};

void NsmCommunicationProtocol::Init()
{
	if (m_isInitialized) return;

	m_rttiMap.emplace(CommunicateDataType::Void, typeid(void).hash_code());
	m_rttiMap.emplace(CommunicateDataType::Int, typeid(int).hash_code());
	m_rttiMap.emplace(CommunicateDataType::Float, typeid(float).hash_code());
	m_rttiMap.emplace(CommunicateDataType::Char, typeid(char).hash_code());
	m_rttiMap.emplace(CommunicateDataType::Long, typeid(long).hash_code());
	m_rttiMap.emplace(CommunicateDataType::Double, typeid(double).hash_code());
	m_rttiMap.emplace(CommunicateDataType::ControllerData, typeid(ControllerData).hash_code());

	m_isInitialized = true;
}
