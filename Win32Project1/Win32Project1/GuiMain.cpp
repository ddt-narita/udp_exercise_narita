#include "stdafx.h"
#include "Win32Project1.h"
#include <stdio.h>
#include <process.h>
//#include <ws2tcpip.h>
//#include <iphlpapi.h>

#include "GuiMain.h"
#include "GuiIf.h"
#include "ThreadBase.h"

static CGuiMain * pGuiMain;

// --------------------------------------------------------------
/*!
 * @brief ������
 *
 * �_�C�A���O�̃n���h������e�R���g���[���̃n���h�����擾��������
 * ���s���B
 *
 * @param[in] hDlg �_�C�A���O�̃E�B���h�E�n���h��
 */
void CGuiMain::Initialize(HWND hDlg)
{
	pGuiMain = this;
	m_hDialog = hDlg;

	// Winsock2������
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	// �R���g���[���̃E�B���h�E�n���h�����擾
	m_hEditOut = GetDlgItem(m_hDialog, IDC_EDIT1);
	m_hEditMsg = GetDlgItem(m_hDialog, IDC_EDIT2);
	m_hIpAddr  = GetDlgItem(m_hDialog, IDC_IPCTL1);
	m_hEditMyIp = GetDlgItem(m_hDialog, IDC_EDIT3);

	// �G�f�B�b�g�R���g���[���̕������ݒ�
	SendMessage(m_hEditMsg, EM_SETLIMITTEXT, (WPARAM)SEND_DATA_CHAR_MAX, 0);

	// ��IP�A�h���X�ݒ�
	SetWindowText(m_hEditMyIp, GetMyIpAddr());

	// �X���b�h��񏉊���
	int i;
	for (i=0; i<TH_MAX; i ++)
	{
		m_ThreadInfo[i].Init();
	}

	// �����ʐM�p�\�P�b�g�쐬
	CreateInternalSocket(TH_SEND);

	extern unsigned int __stdcall ThreadSendMain(PVOID param);
	extern unsigned int __stdcall ThreadRecvMain(PVOID param);

	// �X���b�h�J�n
	StartThread(TH_SEND, ThreadSendMain);
}

// --------------------------------------------------------------
/*!
 * @brief ���b�Z�[�W���z
 *
 * ���b�Z�[�W�ɉ������������N������B
 *
 * @param[in] uMsg   ���b�Z�[�WID
 * @param[in] wParam WPARAM
 * @param[in] lParam LPARAM
 * 
 * @return �����Ftrue�A���s�Ffalse
 */
bool CGuiMain::DispatchMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		if (! OnCommand(wParam, lParam)) return false;
		break;

	case WM_TIMER:
		// �����V���b�g�ɂ��Ă��邽�ߒ�~
		KillTimer(m_hDialog, wParam);
		// �^�C���A�E�g�����N��
		OnTimerTimeout(wParam, lParam);
		break;
	}

	return true;
}

// --------------------------------------------------------------
/*!
 * @brief WM_COMMAND��M����
 *
 * ��M�����R�}���h�ɉ����ď������s���B
 *
 * @param[in] wParam WPARAM
 * @param[in] lParam LPARAM
 * 
 * @return �R�}���h�����������ꍇtrue�A�������Ȃ������ꍇfalse��Ԃ��B
 */
bool CGuiMain::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int i;

	switch (LOWORD(wParam))
	{
	case IDOK:
		{
			struct SendDataNotify aData = {0};

			// ���M��IP�擾
			GetWindowText(m_hIpAddr, aData.ipaddr, sizeof(aData.ipaddr));
			aData.iplen = strlen(aData.ipaddr);
			// ���M�f�[�^�擾
			GetWindowText(m_hEditMsg, aData.data, sizeof(aData.data));
			aData.datalen = strlen(aData.ipaddr);

			// ���M�f�[�^�ʒm
			aData.header.msgid = MSGID_SENDDATA_NOTIFY;
			SendInternalMessage(TH_SEND, &aData, sizeof(aData));
		}
		break;

	case IDCANCEL:
		// �X���b�h�֏I����ʒm
		{
		struct TerminateNotify msg;
		msg.header.msgid = MSGID_TERMINATE_NOTIFY;
		for (i=0; i<TH_MAX; i ++)
		{
			if (NULL != m_ThreadInfo[i].hThread)
			{	SendInternalMessage(i, &msg, sizeof(msg)); }
		}
		}

		for (i=0; i<TH_MAX; i ++)
		{
			// �����ʐM�p�\�P�b�g�N���[�Y
			if (INVALID_SOCKET != m_ThreadInfo[i].InternalSock)
			{	closesocket(m_ThreadInfo[i].InternalSock); }
			// �n���h���N���[�Y
			if (NULL != m_ThreadInfo[i].hThread)
			{	CloseHandle(m_ThreadInfo[i].hThread); }
		}

		// 
		WSACleanup();
		EndDialog(m_hDialog, 0);
		break;

	default:
		return false;
	}
	return true;
}

// --------------------------------------------------------------
/*!
 * @brief WM_TIMER��M����
 *
 * �^�C�}�N�����X���b�h�փ^�C���A�E�g��ʒm����B
 *
 * @param[in] wParam �^�C�}ID
 * @param[in] lParam ���g�p
 * 
 * @return ���true
 */
bool CGuiMain::OnTimerTimeout(WPARAM wParam, LPARAM lParam)
{
	// �X���b�h�փ^�C���A�E�g��ʒm
	struct TimeoutNotify timeout;
	timeout.header.msgid = MSGID_TIMEOUT_NOTIFY;
	timeout.timerid = wParam;

	for (int i=0; i<TH_MAX; i ++)
	{
		if (m_ThreadInfo[i].bInUse)
		{
			if (wParam == m_ThreadInfo[i].TimerId)
			{
				return SendInternalMessage(i, &timeout, sizeof(timeout));
			}
		}
	}
	return false;
}

// --------------------------------------------------------------
/*!
 * @brief GuiMain�擾
 *
 * GuiMain���擾����B
 *
 * @return GuiMain��Ԃ��B
 */
CGuiMain * CGuiMain::GetGuiMain()
{
	return pGuiMain;
}

// --------------------------------------------------------------
/*!
 * @brief ���ʏo�͏���
 *
 * �o�̓G���A�֒ʒm���ꂽ���ʂ��o�͂���B
 *
 * @param[in] pResult ���ʕ�����iNULL�I�[�j
 */
void CGuiMain::OutputResult(const char * pResult)
{
	HWND hPrev = SetFocus(m_hEditOut);
	SendMessage(m_hEditOut, EM_REPLACESEL, false, (LPARAM)pResult);
	SendMessage(m_hEditOut, EM_REPLACESEL, false, (LPARAM)"\r\n");
	SetFocus(hPrev);
}

// --------------------------------------------------------------
/*!
 * @brief ��IP�A�h���X�擾
 *
 * ��IP�A�h���X���擾����B
 *
 * @return IP�A�h���X������
 */
const char * CGuiMain::GetMyIpAddr()
{
	char ac[80];
	if (SOCKET_ERROR == gethostname(ac, sizeof(ac)))
	{
		return NULL;
	}

	struct hostent * phe = gethostbyname(ac);
	if (phe == 0)
	{
		return NULL;
	}

	for (int i=0; 0 != phe->h_addr_list[i]; i ++)
	{
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));

		// 192.xxx.xxx.xxx ��...
		if (0xC0 == (addr.S_un.S_un_b.s_b1))
		{
			return inet_ntoa(addr);
		}
	}
	return NULL;
}

// --------------------------------------------------------------
/*!
 * @brief �����ʐM�p�\�P�b�g�쐬
 *
 * �q�X���b�h�Ƃ̒ʐM�p�����\�P�b�g���쐬����B
 *
 * @retval true  ����
 * @retval false ���s
 */
bool CGuiMain::CreateInternalSocket(int thKind)
{
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == sock)
	{
		return false;
	}

	m_ThreadInfo[thKind].InternalSock = sock;
	m_ThreadInfo[thKind].InternalAddr.sin_family = AF_INET;
	m_ThreadInfo[thKind].InternalAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	m_ThreadInfo[thKind].InternalAddr.sin_port = htons(INTERNAL_PORT_ORIG + thKind);
	m_ThreadInfo[thKind].bInUse = true;
	return true;
}

// --------------------------------------------------------------
/*!
 * @brief �q�X���b�h�J�n
 *
 * �q�X���b�h���J�n����B
 *
 * @retval true  ����
 * @retval false ���s
 */
static struct ThreadParam threadParam;
bool CGuiMain::StartThread(int thKind, unsigned (__stdcall *pThreadMain)(void *))
{
	memset(&threadParam, 0, sizeof(threadParam));
	threadParam.thKind = thKind;
	threadParam.port = ntohs(m_ThreadInfo[thKind].InternalAddr.sin_port);

	// ��IP�A�h���X�擾
	GetWindowText(m_hEditMyIp, threadParam.ipaddr, sizeof(threadParam.ipaddr));

	// �q�X���b�h�J�n
	HANDLE hThread = (HANDLE)_beginthreadex
								(NULL				// security descripter
								,0					// stack size
								,pThreadMain		// start address
								,&threadParam		// arglist
								,0					// initial state. 0 - running
								,NULL);				// thread indentifier
	if (-1L == (long)hThread)
	{
		return false;
	}

	m_ThreadInfo[thKind].pThreadMain = pThreadMain;
	m_ThreadInfo[thKind].hThread = hThread;
	return true;
}

// --------------------------------------------------------------
/*!
 * @brief �������b�Z�[�W���M
 *
 * �q�X���b�h�փ��b�Z�[�W�𑗐M����B
 *
 * @retval true  ����
 * @retval false ���s
 */
bool CGuiMain::SendInternalMessage(int thKind, const void * pData, unsigned int size)
{
	if (INVALID_SOCKET == m_ThreadInfo[thKind].InternalSock) return false;
	if (NULL == pData) return false;
	if (0 == size) return false;

	// �X���b�h�֓������b�Z�[�W�𑗐M
	sendto
		(m_ThreadInfo[thKind].InternalSock
		,(char *)pData, size
		,0
		,(struct sockaddr *)&m_ThreadInfo[thKind].InternalAddr
		,sizeof(m_ThreadInfo[thKind].InternalAddr)
		);

	return true;
}

// --------------------------------------------------------------
/*!
 * @brief �^�C�}�J�n
 *
 * �^�C�}���J�n����B
 *
 * @param[in] timerid �^�C�}ID
 * @param[in] msec    �^�C�}�l
 */
void CGuiMain::StartTimer(int thKind, unsigned int timerid, unsigned int msec)
{
	if (NO_TIMERID != m_ThreadInfo[thKind].TimerId)
	{
		StopTimer(thKind, m_ThreadInfo[thKind].TimerId);
	}

	SetTimer(m_hDialog, timerid, (UINT)msec, NULL);
	m_ThreadInfo[thKind].TimerId = timerid;
}

// --------------------------------------------------------------
/*!
 * @brief �^�C�}��~
 *
 * �^�C�}���~����B
 *
 * @param[in] timerid �^�C�}ID
 */
void CGuiMain::StopTimer(int thKind, unsigned int timerid)
{
	KillTimer(m_hDialog, timerid);
	m_ThreadInfo[thKind].TimerId = NO_TIMERID;
}

