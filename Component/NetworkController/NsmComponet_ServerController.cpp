#include"../../Nisiguti.h"
#include"../../MemberClasses/NsmCommunicationAsync.h"

RTTI_IMPL(CNsmComponent_ServerController, CComponent_BaseController)

CNsmComponent_ServerController::CNsmComponent_ServerController()
{
	m_inputData = ControllerData();
}

CNsmComponent_ServerController::~CNsmComponent_ServerController()
{
}

void CNsmComponent_ServerController::InitFromJson(const json11::Json & _rJsonObj)
{
	//IComponent_Base::InitFromJson(_rJsonObj);
	//auto jsonAxis = _rJsonObj["AxisList"];
	//auto jsonButtons = _rJsonObj["ButtonList"];
	//if (!jsonAxis.is_array() || !jsonButtons.is_array()) return;

	//std::vector<std::string> axisVector;
	//std::vector<std::string> buttonVector;
	//for (auto axis : jsonAxis.array_items())
	//{
	//	axisVector.push_back(axis.string_value());
	//}
	//for (auto button : jsonButtons.array_items())
	//{
	//	axisVector.push_back(button.string_value());
	//}
	//Create(axisVector, buttonVector);
}

void CNsmComponent_ServerController::Start(void)
{
}

void CNsmComponent_ServerController::Update(void)
{
	if (m_refServer.expired()) return;
	m_inputData = ControllerData();
	m_refServer.lock()->InputControllerData(m_inputData, m_playerPortNo);

}

void CNsmComponent_ServerController::LateUpdate(void)
{
	
}

void CNsmComponent_ServerController::Release(void)
{
}

void CNsmComponent_ServerController::SetPortNo(unsigned short _portNo)
{
	m_playerPortNo = _portNo;
}

void CNsmComponent_ServerController::SetServer(const SPtr<CNsmComponent_Server>& _server)
{
	m_refServer = _server;
}

ControllerData & CNsmComponent_ServerController::GetInputData()
{
	return m_inputData;
}
