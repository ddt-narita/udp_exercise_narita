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
 * @brief 初期化
 *
 * ダイアログのハンドルから各コントロールのハンドルを取得し初期化
 * を行う。
 *
 * @param[in] hDlg ダイアログのウィンドウハンドル
 */
void CGuiMain::Initialize(HWND hDlg)
{
	pGuiMain = this;
	m_hDialog = hDlg;

	// Winsock2初期化
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	// コントロールのウィンドウハンドルを取得
	m_hEditOut = GetDlgItem(m_hDialog, IDC_EDIT1);
	m_hEditMsg = GetDlgItem(m_hDialog, IDC_EDIT2);
	m_hIpAddr  = GetDlgItem(m_hDialog, IDC_IPCTL1);
	m_hEditMyIp = GetDlgItem(m_hDialog, IDC_EDIT3);

	// エディットコントロールの文字数設定
	SendMessage(m_hEditMsg, EM_SETLIMITTEXT, (WPARAM)SEND_DATA_CHAR_MAX, 0);

	// 自IPアドレス設定
	SetWindowText(m_hEditMyIp, GetMyIpAddr());

	// スレッド情報初期化
	int i;
	for (i=0; i<TH_MAX; i ++)
	{
		m_ThreadInfo[i].Init();
	}

	// 内部通信用ソケット作成
	CreateInternalSocket(TH_SEND);

	extern unsigned int __stdcall ThreadSendMain(PVOID param);
	extern unsigned int __stdcall ThreadRecvMain(PVOID param);

	// スレッド開始
	StartThread(TH_SEND, ThreadSendMain);
}

// --------------------------------------------------------------
/*!
 * @brief メッセージ分配
 *
 * メッセージに応じた処理を起動する。
 *
 * @param[in] uMsg   メッセージID
 * @param[in] wParam WPARAM
 * @param[in] lParam LPARAM
 * 
 * @return 成功：true、失敗：false
 */
bool CGuiMain::DispatchMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		if (! OnCommand(wParam, lParam)) return false;
		break;

	case WM_TIMER:
		// ワンショットにしているため停止
		KillTimer(m_hDialog, wParam);
		// タイムアウト処理起動
		OnTimerTimeout(wParam, lParam);
		break;
	}

	return true;
}

// --------------------------------------------------------------
/*!
 * @brief WM_COMMAND受信処理
 *
 * 受信したコマンドに応じて処理を行う。
 *
 * @param[in] wParam WPARAM
 * @param[in] lParam LPARAM
 * 
 * @return コマンドを処理した場合true、処理しなかった場合falseを返す。
 */
bool CGuiMain::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int i;

	switch (LOWORD(wParam))
	{
	case IDOK:
		{
			struct SendDataNotify aData = {0};

			// 送信先IP取得
			GetWindowText(m_hIpAddr, aData.ipaddr, sizeof(aData.ipaddr));
			aData.iplen = strlen(aData.ipaddr);
			// 送信データ取得
			GetWindowText(m_hEditMsg, aData.data, sizeof(aData.data));
			aData.datalen = strlen(aData.ipaddr);

			// 送信データ通知
			aData.header.msgid = MSGID_SENDDATA_NOTIFY;
			SendInternalMessage(TH_SEND, &aData, sizeof(aData));
		}
		break;

	case IDCANCEL:
		// スレッドへ終了を通知
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
			// 内部通信用ソケットクローズ
			if (INVALID_SOCKET != m_ThreadInfo[i].InternalSock)
			{	closesocket(m_ThreadInfo[i].InternalSock); }
			// ハンドルクローズ
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
 * @brief WM_TIMER受信処理
 *
 * タイマ起動元スレッドへタイムアウトを通知する。
 *
 * @param[in] wParam タイマID
 * @param[in] lParam 未使用
 * 
 * @return 常にtrue
 */
bool CGuiMain::OnTimerTimeout(WPARAM wParam, LPARAM lParam)
{
	// スレッドへタイムアウトを通知
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
 * @brief GuiMain取得
 *
 * GuiMainを取得する。
 *
 * @return GuiMainを返す。
 */
CGuiMain * CGuiMain::GetGuiMain()
{
	return pGuiMain;
}

// --------------------------------------------------------------
/*!
 * @brief 結果出力処理
 *
 * 出力エリアへ通知された結果を出力する。
 *
 * @param[in] pResult 結果文字列（NULL終端）
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
 * @brief 自IPアドレス取得
 *
 * 自IPアドレスを取得する。
 *
 * @return IPアドレス文字列
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

		// 192.xxx.xxx.xxx か...
		if (0xC0 == (addr.S_un.S_un_b.s_b1))
		{
			return inet_ntoa(addr);
		}
	}
	return NULL;
}

// --------------------------------------------------------------
/*!
 * @brief 内部通信用ソケット作成
 *
 * 子スレッドとの通信用内部ソケットを作成する。
 *
 * @retval true  成功
 * @retval false 失敗
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
 * @brief 子スレッド開始
 *
 * 子スレッドを開始する。
 *
 * @retval true  成功
 * @retval false 失敗
 */
static struct ThreadParam threadParam;
bool CGuiMain::StartThread(int thKind, unsigned (__stdcall *pThreadMain)(void *))
{
	memset(&threadParam, 0, sizeof(threadParam));
	threadParam.thKind = thKind;
	threadParam.port = ntohs(m_ThreadInfo[thKind].InternalAddr.sin_port);

	// 自IPアドレス取得
	GetWindowText(m_hEditMyIp, threadParam.ipaddr, sizeof(threadParam.ipaddr));

	// 子スレッド開始
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
 * @brief 内部メッセージ送信
 *
 * 子スレッドへメッセージを送信する。
 *
 * @retval true  成功
 * @retval false 失敗
 */
bool CGuiMain::SendInternalMessage(int thKind, const void * pData, unsigned int size)
{
	if (INVALID_SOCKET == m_ThreadInfo[thKind].InternalSock) return false;
	if (NULL == pData) return false;
	if (0 == size) return false;

	// スレッドへ内部メッセージを送信
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
 * @brief タイマ開始
 *
 * タイマを開始する。
 *
 * @param[in] timerid タイマID
 * @param[in] msec    タイマ値
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
 * @brief タイマ停止
 *
 * タイマを停止する。
 *
 * @param[in] timerid タイマID
 */
void CGuiMain::StopTimer(int thKind, unsigned int timerid)
{
	KillTimer(m_hDialog, timerid);
	m_ThreadInfo[thKind].TimerId = NO_TIMERID;
}

