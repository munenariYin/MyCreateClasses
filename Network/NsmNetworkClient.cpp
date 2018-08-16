#include"../MemberClasses/NsmCommunicationAsync.h"
#include "../Nisiguti.h"

CNsmNetworkClient::CNsmNetworkClient()
{
	m_connState = ConnectionState::NotConnection;
	m_enableClient = false;
	m_commAsync = nullptr;
}

CNsmNetworkClient::~CNsmNetworkClient()
{
}

bool CNsmNetworkClient::StartUp()
{
	if (m_enableClient)
	{
		Dw_Scroll(DW_ScrollType::kUser, "StartUp�͊��ɐ������Ă��܂�");
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
	m_enableClient = true;
	return true;
}

void CNsmNetworkClient::EndClient()
{
	m_sendBuffer.clear();
	m_recvBuffer.clear();

	DisConnect();

	if (!m_enableClient) return;
	m_enableClient = false;
	WSACleanup();
}

bool CNsmNetworkClient::BeginConnect(const std::string & _serverlp, unsigned short _portNo)
{
	// ���ڑ��ł���ꍇ�̂ݐڑ����J�n�ł���
	if (m_connState != ConnectionState::NotConnection)
	{
		Dw_Scroll(DW_ScrollType::kUser, "����ɐڑ����ł��Ȃ��\��������܂��B");
		return false;
	}
	if (m_commAsync == nullptr) m_commAsync = std::make_shared<CommunicationAsync>();
	if (m_commAsync->IsEnableAsync())
	{
		Dw_Scroll(DW_ScrollType::kUser, "���ɕʂ̔񓯊��������s���Ă��܂��B");
		return false;
	}
	if (!m_commAsync->PrepareConnectSocket(PF_INET, SOCK_STREAM, IPPROTO_TCP, AF_INET, SocketUtil::TransHostAddr("127.0.0.1"), _portNo))
	{
		Dw_Scroll(DW_ScrollType::kUser, "�\�P�b�g�̐����Ɏ��s���܂����B");
		return false;
	}

	ConnectionState& refConnState = m_connState;
	bool& refEnableClient = m_enableClient;
	WPtr<CommunicationAsync> connectionAsync = m_commAsync;

	std::function<bool()> connectionProc = [&refConnState, &refEnableClient, connectionAsync]()
	{
		while (true)
		{
			if (connectionAsync.expired() || !connectionAsync.lock()->IsEnableAsync() || !refEnableClient)
			{
				refConnState = ConnectionState::FailedConnection;
				Dw_Scroll(DW_ScrollType::kUser, "�X���b�h���I������܂����B");
				return false;
			}

			// �ڑ������݂�
			if (!connectionAsync.lock()->GetSocket()->Connect())
			{
				int errorCode = connectionAsync.lock()->UpdateErrorCode();
				// �G���[�̌��������m���u���b�L���O�ɂ��X���[�ł���΍Ď��s
				if (errorCode == WSAEWOULDBLOCK) continue;
				// connect���s�̌�ɐڑ����ꂽ�ꍇ�A�����Ƃ���B
				if (errorCode != WSAEISCONN)
				{
					std::string errorString = "�ڑ����s�B�G���[�R�[�h : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorString.c_str());
					refConnState = ConnectionState::FailedConnection;
					return false;
				}
			}
			// �ڑ���Ԃɂ��āA�X���b�h���I������B
			refConnState = ConnectionState::Connection;
			break;
		}
		return true;
	};

	// �ڑ��҂���Ԃɂ���
	m_connState = ConnectionState::WaitConnection;
	m_commAsync->SetAsync(connectionProc);
	return true;
}

bool CNsmNetworkClient::CompleteConnecting()
{
	if (m_connState != ConnectionState::Connection)
	{
		Dw_Scroll(DW_ScrollType::kUser, "���ɐڑ����������Ă��邩�A�ڑ����m���ł��܂���B");
		return false;
	}
	// �񓯊��̐ڑ��������I������B
	if (m_commAsync == nullptr) return false;
	m_commAsync->WaitEndAsync();

	return BeginCommunication();
}

bool CNsmNetworkClient::BeginCommunication()
{
	// ���̔񓯊��������܂��s���Ă���΁A�܂��ʐM���J�n�ł��Ȃ��B
	if (m_connState != ConnectionState::Connection || m_commAsync == nullptr)
	{
		Dw_Scroll(DW_ScrollType::kUser, "�ڑ����m������Ă��܂���");
		return false;
	}
	if (m_commAsync->IsValidAsync())
	{
		Dw_Scroll(DW_ScrollType::kUser, "���̃X���b�h���ғ����Ă��܂��B");
		return false;
	}
	
	bool& refEnableClient = m_enableClient;
	ConnectionState& refConnectState = m_connState;
	WPtr<CommunicationAsync> communicationAsync = m_commAsync;
	std::function<bool()> communicationProc = [&refEnableClient, &refConnectState, communicationAsync]()
	{
		while (true)
		{
			// �N���C�A���g���I�����Ă��邩�A�񓯊������̏I�������m������I��
			if (!refEnableClient || communicationAsync.expired() || communicationAsync.lock()->IsEnableAsync())
			{
				refConnectState = ConnectionState::FailedConnection;
				break;
			}
			CommDataState dataState = communicationAsync.lock()->Receive();
			// �F���b�Z�[�W��M
			if (dataState == CommDataState::Success)
			{
				std::string recvData = "��M[server] port : " + std::to_string(communicationAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, recvData.c_str());
			}
			else if (dataState == CommDataState::Failed)
			{
				int errorCode = communicationAsync.lock()->UpdateErrorCode();
				if (errorCode != WSAEWOULDBLOCK)
				{
					std::string errorMessage = "recv()���A������ؒf����Ă��܂����B�G���[�R�[�h : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorMessage.c_str());
					// �I���t���O
					refConnectState = ConnectionState::FailedConnection;
					return false;
				}
			}

			dataState = communicationAsync.lock()->Send();
			if (dataState == CommDataState::Success)
			{
				std::string recvData = "���M[server] port : " + std::to_string(communicationAsync.lock()->GetSocket()->GetAddr().sin_port);
				Dw_Scroll(DW_ScrollType::kUser, recvData.c_str());
			}
			else if (dataState == CommDataState::Failed)
			{
				int errorCode = communicationAsync.lock()->UpdateErrorCode();
				if (errorCode != WSAEWOULDBLOCK)
				{
					std::string errorMessage = "send()���A������ؒf����Ă��܂����B�G���[�R�[�h : " + std::to_string(errorCode);
					Dw_Scroll(DW_ScrollType::kUser, errorMessage.c_str());
					// �I���t���O
					refConnectState = ConnectionState::FailedConnection;
					return false;
				}
			}
		}
		return true;
	};

	m_connState = ConnectionState::Communication;
	m_commAsync->SetAsync(communicationProc);

	return true;
}

void CNsmNetworkClient::DisConnect()
{
	m_connState = ConnectionState::NotConnection;
	if (m_commAsync == nullptr) return;
	m_commAsync->WaitEndAsync();
	m_commAsync->Clear();
	m_commAsync->ClearRecvBuffer();
	m_commAsync->ClearSendBuffer();
	m_commAsync = nullptr;
}

ConnectionState CNsmNetworkClient::GetConnectionState()
{
	return m_connState;
}
