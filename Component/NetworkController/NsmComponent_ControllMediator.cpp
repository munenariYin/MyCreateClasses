#include"..\..\Nisiguti.h"

RTTI_IMPL(CNsmComponent_ControllMediator, IComponent_Base)

CNsmComponent_ControllMediator::CNsmComponent_ControllMediator()
{
}
CNsmComponent_ControllMediator::~CNsmComponent_ControllMediator()
{
}

void CNsmComponent_ControllMediator::InitFromJson(const json11::Json & _rJsonObj)
{
}

void CNsmComponent_ControllMediator::Start(void)
{
	m_controller = GetGameObject()->GetComponent<CComponent_BaseController>();
}

void CNsmComponent_ControllMediator::Update(void)
{
}

void CNsmComponent_ControllMediator::LateUpdate(void)
{
}

void CNsmComponent_ControllMediator::Release(void)
{
}
