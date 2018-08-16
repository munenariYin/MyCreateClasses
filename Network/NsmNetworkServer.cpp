#include"..\MemberClasses\NsmCommunicationAsync.h"
#include"..\Nisiguti.h"

CNsmNetworkServer::CNsmNetworkServer(){}

CNsmNetworkServer::~CNsmNetworkServer(){}

bool CNsmNetworkServer::StartUp()
{
	// ���ɏ���������Ă���Ή������Ȃ�
	if (m_enableServer)
	{
		Dw_Scroll(DW_ScrollType::kUser, "����Startup�͍s���Ă��܂��B");
		return true;
	}

	// WinsockAPI�g�p����
	unsigned short version = MAKEWORD(1, 1);
	if (WSAStartup(version, &m_wsaData) != 0)
	{
		Dw_Scroll(DW_ScrollType::kUser, "WinSockAPI�̏������Ɏ��s���܂����B");
		WSACleanup();
		return false;
	}
	m_enableServer = true;
	return true;
}

bool CNsmNetworkServer::Listen(unsigned long _inAddr, unsigned short _port)
{
	if (!m_enableServer) return false;
	if (m_acceptAsync != nullptr && m_acceptAsync->IsEnableAsync()) return false;

	m_acceptAsync = std::make_shared<CommunicationAsync>();
	if (!m_acceptAsync->PrepareAcceptSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, AF_INET, htonl(_inAddr), _port))
	{
		Dw_Scroll(DW_ScrollType::kUser, "Accept�̊J�n�Ɏ��s���܂����B");
		return false;
	}

	m_portNo = _port;
	// ��t�J�n
	m_enableAccept = true;
	// ---------------------------------------------------------------------
	// ��t�p�̃X���b�h�����
	// �L���v�`���ɗp����ϐ����Q�Ƒ̂ɓ����
	bool& refEnableServer = m_enableServer;	// �T�[�o���̂̏I�����m
	bool& refEnableAccept = m_enableAccept;	// ��t�̒��~���m
	WPtr<CommunicationAsync> acceptAsync = m_acceptAsync;

	std::function<bool()> acceptProc = [this, &refEnableServer, &refEnableAccept, acceptAsync]()
	{
		while (true)
		{
			if (acceptAsync.expired()) return false;
			// ��t�I�����`����ꂽ��񓯊��������I������
			if (!refEnableAccept || !refEnableServer || !acceptAsync.lock()->IsEnableAsync()) break;
			SPtr<Socket> newClient = std::make_shared<Socket>();
			// �ڑ��v��������Ύ󂯕t����
			if (!acceptAsync.lock()->GetSocket()->Accept(*newClient))
			{
				int errorCode = acceptAsync.lock()->UpdateErrorCode();
				// �G���[�̊m�F�����āA�m���u���b�L���O�ɂ��X���[�������Ȃ瑱�s
				if (errorCode == WSAEWOULDBLOCK) continue;
				std::string errorString = "�G���[�R�[�h : " + std::to_string(errorCode);
				Dw_Scroll(DW_ScrollType::kUser, errorString.c_str());
				refEnableAccept = false;
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

void CNsmNetworkServer::StopListen()
{
	m_enableAccept = false;
	if (m_acceptAsync == nullptr) return;
	m_acceptAsync->WaitEndAsync();
	m_acceptAsync->Clear();
	m_acceptAsync = nullptr;
}

void CNsmNetworkServer::ConnectAllPendClients()
{
	for (int i = m_pendClientVector.size() - 1; i >= 0; i--)
	{
		m_clientVector.push_back(m_pendClientVector[i]);
		m_pendClientVector.erase(m_pendClientVector.begin() + i);
	}
}

void CNsmNetworkServer::RemoveDisconnectedClients()
{
	// �K���X���b�h���I�������Ă���N���C�A���g������
	for (int i = m_clientVector.size() - 1; i >= 0; i--)
	{
		// �܂��I�����Ă��Ȃ��N���C�A���g�͏����Ȃ�
		if (!m_clientVector[i]->IsEnableAsync()) continue;
		m_clientVector[i]->WaitEndAsync();
		m_clientVector[i]->Clear();
		m_clientVector[i] = nullptr;
		m_clientVector.erase(m_clientVector.begin() + i);
	}

	// �K���X���b�h���I�������Ă���N���C�A���g������
	for (int i = m_pendClientVector.size() - 1; i >= 0; i--)
	{
		// �܂��I�����Ă��Ȃ��N���C�A���g�͏����Ȃ�
		if (!m_pendClientVector[i]->IsEnableAsync()) continue;
		m_pendClientVector[i]->WaitEndAsync();
		m_pendClientVector[i]->Clear();
		m_pendClientVector[i] = nullptr;
		m_pendClientVector.erase(m_pendClientVector.begin() + i);
	}
}

void CNsmNetworkServer::DisconnectAll()
{
	// �K���X���b�h���I�������Ă���N���C�A���g������
	for (int i = m_clientVector.size() - 1; i >= 0; i--)
	{
		m_clientVector[i]->SetEnableAsync(false);
		m_clientVector[i]->WaitEndAsync();
		m_clientVector[i]->Clear();
		m_clientVector[i] = nullptr;
	}
	m_clientVector.clear();

	for (int i = m_pendClientVector.size() - 1; i >= 0; i--)
	{
		m_pendClientVector[i]->SetEnableAsync(false);
		m_pendClientVector[i]->WaitEndAsync();
		m_pendClientVector[i]->Clear();
		m_pendClientVector[i] = nullptr;
	}
	m_pendClientVector.clear();

}

void CNsmNetworkServer::CreateClientAsync(SPtr<Socket>& _socket)
{
	// �L���v�`���ɗp����ϐ��̗p��
	// �V�����N���C�A���g�p�̃\�P�b�g���m��
	SPtr<CommunicationAsync> newClientAsync = std::make_shared<CommunicationAsync>();
	newClientAsync->SetSocket(_socket);
	WPtr<CommunicationAsync> clientAsync = newClientAsync;

	// �T�[�o���̂��I�����������m�F���邽�߂̂���
	bool& refEnableServer = m_enableServer;

	// ----------------------------------------------------------------------------------------
	// �X���b�h�œ������֐�
	std::function<bool()> communicateProc = [&refEnableServer, clientAsync]()
	{
		while (true)
		{
			// �T�[�o���̂��I�����悤�Ƃ��Ă�����A�N���C�A���g�𐳏�ɏI��������悤�ɂ���B�|�C���^�������Ă��܂��Ă����ꍇ�����l
			if (refEnableServer || clientAsync.expired() || !clientAsync.lock()->IsEnableAsync()) break;

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

	//WPtr<CGameObject> chara = std::make_shared<CGameObject>(GW.GetNowLevel()->GetNowSubLevel());
	//if (!chara.lock()->InitFromJsonFile("data/User/Nisiguti/Json/GameObject/HeroGmObj.json")) return;
	//chara.lock()->SetParent(GW.GetNowLevel()->GetNowSubLevel()->GetRootObject());
	//SPtr<CNsmComponent_ServerController> controller = chara.lock()->GetComponent<CNsmComponent_ServerController>();
	//chara.lock()->GetTransform()->pos.Set(rand() % 4 - 2, 0.0f, rand() % 4 - 2);
	//controller->SetPortNo(newClientAsync->GetSocket()->GetAddr().sin_port);
	//controller->SetServer(YsToSPtr(this));

	newClientAsync->SetAsync(communicateProc);
	m_pendClientVector.push_back(newClientAsync);
	
	std::string addr(inet_ntoa(_socket->GetAddr().sin_addr));
	std::string successMessage = "�ڑ������N���C�A���g�����܂��B\n�|�[�g : " + std::to_string(ntohs(_socket->GetAddr().sin_port)) + "\n�A�h���X : " + addr;
	Dw_Scroll(DW_ScrollType::kUser, successMessage.c_str());
}

void CNsmNetworkServer::EndServer()
{
	m_sendBuffer.clear();
	m_recvBuffer.clear();
	StopListen();
	DisconnectAll();

	if (!m_enableServer)return;
	m_enableServer = false;
	WSACleanup();
}


