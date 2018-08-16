#include"NsmComponent_Hero.h"

CNsmComponent_Hero::CNsmComponent_Hero()
{
}

CNsmComponent_Hero::~CNsmComponent_Hero()
{
}

void CNsmComponent_Hero::InitFromJson(const json11::Json & _rJsonObj)
{
}

void CNsmComponent_Hero::Start(void)
{
	m_refController = GetGameObject()->GetComponent<CNsmComponent_ServerController>();
	m_transform = GetGameObject()->GetComponent<CComponent_Transform>();
}

void CNsmComponent_Hero::Update(void)
{
	ControllerData& refInputData = m_refController.lock()->GetInputData();
	YsVec3 pos = m_transform->GetPos();
	pos.x += refInputData.leftAxis.x;
	pos.z += refInputData.leftAxis.y;
	m_transform->SetPos(pos);
}

void CNsmComponent_Hero::LateUpdate(void)
{
}

void CNsmComponent_Hero::Release(void)
{
}
