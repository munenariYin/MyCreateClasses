#include "../../Nisiguti.h"

RTTI_IMPL(CNsmComponent_ClientController, CComponent_BaseController)

CNsmComponent_ClientController::CNsmComponent_ClientController()
{
	m_keyVector.push_back('Z');
	m_keyVector.push_back('X');
	m_keyVector.push_back('C');
	m_keyVector.push_back('V');

}

CNsmComponent_ClientController::~CNsmComponent_ClientController()
{
}

void CNsmComponent_ClientController::InitFromJson(const json11::Json & _rJsonObj)
{
	//InitFromJson(_rJsonObj);
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

void CNsmComponent_ClientController::Start(void)
{
	CSubLevel* subLevel = GW.GetNowLevel()->GetSubLevel("Client");
	m_refClient = GW.GetNowLevel()->GetSubLevel("Client")->GetRootObject()->GetComponent<CNsmComponent_Client>();
}

void CNsmComponent_ClientController::Update(void)
{
	//int keyCnt = 0;
	//for (auto& button : m_ButtonMap)
	//{
	//	if (keyCnt >= m_keyVector.size()) break;
	//	if (INPUT.KeyCheck(m_keyVector[keyCnt]))
	//	{

	//	}
	//	// 入力されていればビットを立てる
	//	if(button.second) m_inputData.buttons += buttonBit;
	//	buttonBit *= 2;
	//	keyCnt++;
	//}
	//
	m_inputData.rightAxis = YsVec2::Zero;
	if (INPUT.KeyCheck(VK_UP))		m_inputData.leftAxis.y = 1.0f;
	if (INPUT.KeyCheck(VK_DOWN))	m_inputData.rightAxis.y = -1.0f;
	if (INPUT.KeyCheck(VK_LEFT))	m_inputData.rightAxis.x = -1.0f;
	if (INPUT.KeyCheck(VK_RIGHT))	m_inputData.rightAxis.x = 1.0f;
	
	m_inputData.leftAxis = YsVec2::Zero;
	if (INPUT.KeyCheck('W'))		m_inputData.leftAxis.y = 1.0f;
	if (INPUT.KeyCheck('S'))		m_inputData.leftAxis.y = -1.0f;
	if (INPUT.KeyCheck('A'))		m_inputData.leftAxis.x = -1.0f;
	if (INPUT.KeyCheck('D'))		m_inputData.leftAxis.x = 1.0f;

	m_inputData.buttons = 0;
	if (INPUT.KeyCheck('Z', INPUT.KEY_Enter))	m_inputData.buttons += 1;
	if (INPUT.KeyCheck('X', INPUT.KEY_Enter))	m_inputData.buttons += 2;
	if (INPUT.KeyCheck('C', INPUT.KEY_Enter))	m_inputData.buttons += 4;
	if (INPUT.KeyCheck('V', INPUT.KEY_Enter))	m_inputData.buttons += 8;

}

void CNsmComponent_ClientController::LateUpdate(void)
{
	//m_inputData.buttons = 0;
	//int buttonBit = 1;
	//for (auto& button : m_ButtonMap)
	//{
	//	// 入力されていればビットを立てる
	//	if(button.second) m_inputData.buttons += buttonBit;
	//	buttonBit *= 2;
	//}
	// 入力情報の送信
	if (m_inputData.buttons == 0 && m_inputData.leftAxis == YsVec2::Zero && m_inputData.rightAxis == YsVec2::Zero) return;
	m_refClient->AddInputDataBuffer(m_inputData);
}
