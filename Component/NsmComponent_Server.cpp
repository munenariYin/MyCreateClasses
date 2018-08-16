#include"../Network/NsmCommunicationProtocol.h"
#include"../MemberClasses/NsmCommunicationAsync.h"

#include "NsmComponent_Server.h"

// RTTI
RTTI_IMPL(CNsmComponent_Server, IComponent_Base)

CNsmComponent_Server::CNsmComponent_Server()
{
	m_isAccepting = false;
	m_isEndServer = false;
	NsmCommunicationProtocol::Init();
}

CNsmComponent_Server::~CNsmComponent_Server()
{
	m_isEndServer = true;
	m_isAccepting = false;

	if (m_acceptAsync != nullptr)
	{
		m_acceptAsync->WaitEndAsync();
	}
	m_acceptAsync->Clear();
	m_acceptAsync = nullptr;

	for (auto client : m_clientVector)
	{
		client.first->SetEnableAsync(false);
	}
	for (auto client : m_clientVector)
	{
		client.first->WaitEndAsync();
		client.first = nullptr;
	}
	m_clientVector.clear();
	WSACleanup();
}

void CNsmComponent_Server::InitFromJson(const json11::Json& _rJsonObj)
{
	IComponent_Base::InitFromJson(_rJsonObj);

	auto jsonPortNo= _rJsonObj["PortNo"];
	if (jsonPortNo.is_number())
	{
		m_portNo = static_cast<int>(jsonPortNo.number_value());
	}

	auto jsonClientMax = _rJsonObj["ClientMax"];
	if (jsonClientMax.is_number())
	{
		m_clientCntMax = static_cast<unsigned int>(jsonClientMax.number_value());
	}
}

void CNsmComponent_Server::Start(void)
{
	// WinsockAPI�g�p����
	unsigned short version = MAKEWORD(1, 1);
	if (WSAStartup(version, &m_wsaData) != 0)
	{
		// WinSockAPI�������Ɏ��s������A�R���|�[�l���g���̂��폜����B
		SetIsDeleteComponent(true);
		Dw_Scroll(DW_ScrollType::kUser, "WinSockAPI�̏������Ɏ��s���܂����B");
		WSACleanup();
		return;
	}

	BeginAccept();
}

void CNsmComponent_Server::Update(void)
{
	for (auto client : m_clientVector)
	{
		// �܂��N���C�A���g����󂯎�����f�[�^���W�߂�
		client.first->MoveRecvData(m_recvBuffer);

		// ��M�o�b�t�@����R���g���[���f�[�^�����o��
		size_t controllerSize = sizeof(ControllerData);
		if (m_recvBuffer.size() < controllerSize) continue;

		// �w�b�_��������
		size_t headerSize = sizeof(HeaderData);
		m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + headerSize);

		// ���͏��𓾂���A���̃f�[�^��������
		ControllerData controllData;
		::memcpy(reinterpret_cast<char*>(&controllData), &m_recvBuffer[0], controllerSize);
		m_playersControllerData.push_back(std::pair<unsigned short, ControllerData>(client.first->GetSocket()->GetAddr().sin_port, controllData));
		m_recvBuffer.erase(m_recvBuffer.begin(), m_recvBuffer.begin() + controllerSize);
		
		//for (auto client : m_clientVector)
		//{
		//	for(auto object :  client.second.lock()->GetSubLevel()->spObjectRoot->ChildList)

		//}
		
		for (auto subLevel : GW.GetNowLevel()->GetSubLevelList())
		{
			for (auto child : subLevel.second.spObjectRoot->GetChildList())
			{
				child->GetChildList();
			}
		}
	}
}

void CNsmComponent_Server::LateUpdate(void)
{
	if (GetAsyncKeyState('Q') & 0x8000)
	{
		m_isEndServer = true;
	}

	// �N���C�A���g�̏I���̌��m��������폜����B
	for (int i = m_clientVector.size() - 1; i >= 0; i--)
	{
		// �N���C�A���g�̔񓯊������̏I�������m������I�������Ĕz�񂩂�O��
		if (m_clientVector[i].first->IsEnableAsync())continue;
		// �X���b�h�̏I����҂�����Ƀ��X�g����O���B
		m_clientVector[i].first->WaitEndAsync();
		m_clientVector[i].first = nullptr;
		m_clientVector.erase(m_clientVector.begin() + i);
	}
	if (m_isEndServer) return;
}

void CNsmComponent_Server::Draw(void)
{
}

void CNsmComponent_Server::Release(void)
{

}

bool CNsmComponent_Server::BeginAccept()
{
	m_acceptAsync = std::make_shared<CommunicationAsync>();
	// accept�p�\�P�b�g�̍쐬
	if (!m_acceptAsync->PrepareAcceptSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, AF_INET, htonl(INADDR_ANY), m_portNo))
	{
		Dw_Scroll(DW_ScrollType::kUser, "Accept�̊J�n�Ɏ��s���܂����B");
		return false;
	}
	// ��t�J�n
	m_isAccepting = true;
	// ---------------------------------------------------------------------
	// ��t�p�̃X���b�h�����
	// �L���v�`���ɗp����ϐ����Q�Ƒ̂ɓ����
	bool& refIsEnd = m_isEndServer;	// �T�[�o���̂̏I�����m
	bool& refIsAccepting = m_isAccepting;	// ��t�̒��~���m
	WPtr<CommunicationAsync> acceptAsync = m_acceptAsync;

	std::function<bool()> acceptProc = [this, &refIsEnd, &refIsAccepting, acceptAsync]()
	{
		while (true)
		{
			// ��t�I�����`����ꂽ��񓯊��������I������
			if (!refIsAccepting) return true;
			SPtr<Socket> newClient = std::make_shared<Socket>();
			// �ڑ��v��������Ύ󂯕t����
			if (!acceptAsync.lock()->GetSocket()->Accept(*newClient))
			{
				int errorCode = acceptAsync.lock()->UpdateErrorCode();
				// �G���[�̊m�F�����āA�m���u���b�L���O�ɂ��X���[�������Ȃ瑱�s
				if (errorCode == WSAEWOULDBLOCK) continue;
				std::string errorString = "�G���[�R�[�h : " + std::to_string(errorCode);
				Dw_Scroll(DW_ScrollType::kUser, errorString.c_str());
				refIsAccepting = false;
				return false;
			}
			// �V���ȃN���C�A���g�Ƃ��ēo�^
			this->CreateClientAsync(newClient);
		}
		return true;
	};
	// ��t�֐������܂�
	// -----------------------------------------------------------------------

	// ��t�X���b�h�̊J�n
	m_acceptAsync->SetAsync(acceptProc);
	Dw_Scroll(DW_ScrollType::kUser, "Accept�J�n");
	return true;
}

void CNsmComponent_Server::InputControllerData(ControllerData& _outControllData, short _portNo)
{
	size_t inputCnt = m_playersControllerData.size();
	for (size_t i = 0; i < inputCnt; i++)
	{
		if (m_playersControllerData[i].first != _portNo) continue;
		_outControllData = m_playersControllerData[i].second;
		m_playersControllerData.erase(m_playersControllerData.begin() + i);
		return;
	}
}

void CNsmComponent_Server::CreateClientAsync(SPtr<Socket>& _socket)
{
	// �L���v�`���ɗp����ϐ��̗p��
	// �V�����N���C�A���g�p�̃\�P�b�g���m��
	SPtr<CommunicationAsync> newClientAsync = std::make_shared<CommunicationAsync>();
	newClientAsync->SetSocket(_socket);
	WPtr<CommunicationAsync> clientAsync = newClientAsync;

	// �T�[�o���̂��I�����������m�F���邽�߂̂���
	bool& refIsEndServer = m_isEndServer;
	// ----------------------------------------------------------------------------------------
	// �X���b�h�œ������֐�
	std::function<bool()> communicateProc = [&refIsEndServer, clientAsync]()
	{
		while (true)
		{
			// �T�[�o���̂��I�����悤�Ƃ��Ă�����A�N���C�A���g�𐳏�ɏI��������悤�ɂ���B
			if (refIsEndServer || !clientAsync.lock()->IsEnableAsync()) return false;

			// ��M
			CommDataState dataState = clientAsync.lock()->Receive();
			if (dataState == CommDataState::Failed)
			{
				int errorNo = clientAsync.lock()->UpdateErrorCode();
				// �m���u���b�L���O�������̃G���[�Ȃ�X���[
				if (errorNo != WSAEWOULDBLOCK)
				{
					Dw_Scroll(DW_ScrollType::kUser, "������ؒf����Ă��܂����B: recv()");
					clientAsync.lock()->SetEnableAsync(false);
					return false;
				}
			}
			
			// ���M
			dataState = clientAsync.lock()->Send();
			if (dataState == CommDataState::Failed)
			{
				if (clientAsync.lock()->UpdateErrorCode() != WSAEWOULDBLOCK)
				{
					Dw_Scroll(DW_ScrollType::kUser, "������ؒf����Ă��܂����B: send()");
					clientAsync.lock()->SetEnableAsync(false);
					return false;
				}
			}
			else if (dataState == CommDataState::Success)
			{
				std::string portString = "[client] : port : " + std::to_string(clientAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, portString.c_str());
			}
		}
		return true;
	};
	// �񓯊����������܂�
	// ------------------------------------------------------------

	// �񓯊��֐�����
	WPtr<CGameObject> chara = std::make_shared<CGameObject>(GW.GetNowLevel()->GetNowSubLevel());
	if (!chara.lock()->InitFromJsonFile("data/User/Nisiguti/Json/GameObject/HeroGmObj.json")) return;
	chara.lock()->SetParent(GW.GetNowLevel()->GetNowSubLevel()->GetRootObject());
	SPtr<CNsmComponent_ServerController> controller = chara.lock()->GetComponent<CNsmComponent_ServerController>();
	chara.lock()->GetTransform()->pos.Set(rand() % 4 - 2, 0.0f, rand() % 4 - 2);
	controller->SetPortNo(newClientAsync->GetSocket()->GetAddr().sin_port);
	controller->SetServer(YsToSPtr(this));
	newClientAsync->SetAsync(communicateProc);
	m_clientVector.push_back(std::make_pair(newClientAsync, chara));
	std::string successMessage = "�ڑ������N���C�A���g�����܂��B�|�[�g : " + std::to_string(_socket->GetAddr().sin_port);
	Dw_Scroll(DW_ScrollType::kUser, successMessage.c_str());


}