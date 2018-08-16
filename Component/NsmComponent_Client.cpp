#include"../Network/NsmCommunicationProtocol.h"
#include"../MemberClasses/NsmCommunicationAsync.h"

#include "NsmComponent_Client.h"



RTTI_IMPL(CNsmComponent_Client, IComponent_Base)

CNsmComponent_Client::CNsmComponent_Client()
{
	NsmCommunicationProtocol::Init();
}

CNsmComponent_Client::~CNsmComponent_Client()
{
	// �I���t���O�𗧂āA���񏈗��̏I����҂�
	m_isTerminateApp = true;
	if (m_commAsync != nullptr)
	{
		m_commAsync->SetEnableAsync(false);
		m_commAsync->WaitEndAsync();
	}

}

void CNsmComponent_Client::InitFromJson(const json11::Json& _rJsonObj)
{
	IComponent_Base::InitFromJson(_rJsonObj);

	auto jsonPortNo = _rJsonObj["PortNo"];
	if (jsonPortNo.is_number())
	{
		m_portNo = jsonPortNo.int_value();
	}
}

void CNsmComponent_Client::Start()
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

}

void CNsmComponent_Client::Update(void)
{
	// �ڑ��̎��s
	if (m_connectionState == ConnectionState::NotConnection)
	{
		if (INPUT.KeyCheck(VK_RETURN))
		{
			if(BeginConnect());
		}
		return;
	}
	// �ڑ��ҋ@
	if (m_connectionState == ConnectionState::WaitConnection) 
	{
		WaitConnect();
		return;
	}



	// �ڑ����I������
	if (!m_commAsync->IsEnableAsync()) m_commAsync->WaitEndAsync();
	
}

void CNsmComponent_Client::LateUpdate(void)
{
}

void CNsmComponent_Client::Draw(void)
{
}

void CNsmComponent_Client::Release(void)
{
}

bool CNsmComponent_Client::BeginConnect()
{
	if(m_commAsync == nullptr) m_commAsync = std::make_shared<CommunicationAsync>();

	if (!m_commAsync->PrepareConnectSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, AF_INET, SocketUtil::TransHostAddr("127.0.0.1"), m_portNo))
	{
		Dw_Scroll(DW_ScrollType::kUser, "�\�P�b�g�̏����Ɏ��s���܂����B");
		return false;
	}
	Dw_Scroll(DW_ScrollType::kUser, "�ڑ��J�n");
	m_connectionState = ConnectionState::WaitConnection;
	return true;
}

void CNsmComponent_Client::AddInputDataBuffer(ControllerData& _inputData)
{
	if (m_commAsync == nullptr) return;
	m_commAsync->AddSendBuffer(_inputData);
}

void CNsmComponent_Client::WaitConnect()
{
	if (Connect()) return;
	// ���ڑ��̃X���[�Ȃ疳��
	if (m_connectionState == ConnectionState::WaitConnection) return;
	// ���s�����ꍇ...
	m_commAsync->Clear();
	m_connectionState = ConnectionState::NotConnection;
	
}

bool CNsmComponent_Client::Connect()
{
	SPtr<Socket> commSocket = m_commAsync->GetSocket();
	// �ڑ������݂�
	if (!commSocket->Connect())
	{
		int errorCode = m_commAsync->UpdateErrorCode();
		// �G���[���m���u���b�L���O�ɂ��X���[�ł���΍Ď��s
		if (errorCode == WSAEWOULDBLOCK) return false;
		// connect���s�̌�ɐڑ����ꂽ�ꍇ�A�����Ƃ���B
		if (errorCode != WSAEISCONN)
		{
			std::string errorString = "�ڑ����s�B�G���[�R�[�h : " + std::to_string(errorCode);
			Dw_Scroll(DW_ScrollType::kUser, errorString.c_str());
			m_connectionState = ConnectionState::FailedConnection;
			return false;
		}
	}
	// �ڑ��ɐ�������Δ񓯊��������J�n����B
	CreateCommunicateAsync();
	Dw_Scroll(DW_ScrollType::kUser, "�ڑ�����");
	m_connectionState = ConnectionState::Connection;
	return true;
}

void CNsmComponent_Client::CreateCommunicateAsync()
{
	bool& refIsTerminateApp = m_isTerminateApp;
	WPtr<CommunicationAsync> commAsync = m_commAsync;
	// ��b�X���b�h //--------------------------------------------------
	std::function<bool()> communicationAsync = [this, commAsync, &refIsTerminateApp]()
	{
		while (true)
		{
			// �T�[�o�̏I���A�������͔񓯊������̏I���t���O�����ĂΏI����
			if (refIsTerminateApp || !commAsync.lock()->IsEnableAsync()) return true;

			CommDataState dataState = commAsync.lock()->Receive();
			// �F���b�Z�[�W��M
			if (dataState == CommDataState::Success)
			{
				std::string recvData = "��M[server] port : " + std::to_string(commAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, recvData.c_str());
			}
			else if(dataState == CommDataState::Failed)
			{
				int errorCode = commAsync.lock()->UpdateErrorCode();
				if (errorCode != WSAEWOULDBLOCK)
				{
					std::string errorMessage = "recv()���A������ؒf����Ă��܂����B�G���[�R�[�h : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorMessage.c_str());
					// �I���t���O
					commAsync.lock()->SetEnableAsync(false);
					return false;
				}
			}

			dataState = commAsync.lock()->Send();
			if (dataState == CommDataState::Success)
			{
				std::string recvData = "���M[server] port : " + std::to_string(commAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, recvData.c_str());
			}
			else if (dataState == CommDataState::Failed)
			{
				int errorCode = commAsync.lock()->UpdateErrorCode();
				if (errorCode != WSAEWOULDBLOCK)
				{
					std::string errorMessage = "send()���A������ؒf����Ă��܂����B�G���[�R�[�h : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorMessage.c_str());
					// �I���t���O
					commAsync.lock()->SetEnableAsync(false);
					return false;
				}
			}

		}
		return true;
	};
	// ��b�X���b�h�I�� //----------------------------------------------
	m_commAsync->SetAsync(communicationAsync);

}

ConnectionState CNsmComponent_Client::GetConnectionState()
{
	return m_connectionState;
}

