#ifndef __GUIMAIN_H__
#define __GUIMAIN_H__

const UINT NO_TIMERID = 0;

#include <winsock2.h>


class CGuiMain
{
public:
	CGuiMain()
		:m_hDialog(NULL)
		,m_hEditOut(NULL)
		,m_hEditMsg(NULL)
		,m_hIpAddr(NULL)
		,m_hEditMyIp(NULL)
		{}
	~CGuiMain() {}

	void Initialize(HWND hDlg);
	bool DispatchMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

	static CGuiMain * GetGuiMain();

	void OutputResult(const char * pResult);

	// タイマ開始
	void StartTimer(int thKind, unsigned int timerid, unsigned int msec);
	// タイマ停止
	void StopTimer(int thKind, unsigned int timerid);

private:
	bool OnCommand(WPARAM wParam, LPARAM lParam);
	bool OnTimerTimeout(WPARAM wParam, LPARAM lParam);

	const char * GetMyIpAddr();

	bool CreateInternalSocket(int thKind);
	bool StartThread(int thKind, unsigned (__stdcall *pThreadMain)(void *));
	bool SendInternalMessage(int thKind, const void * pData, unsigned int size);

private:
	// ウィンドウハンドル
	HWND m_hDialog;			// ダイアログ
	HWND m_hEditOut;		// 出力エリア
	HWND m_hEditMsg;		// 送信データ
	HWND m_hIpAddr;			// IPアドレス
	HWND m_hEditMyIp;		// 自IPアドレス

	enum
	{	 TH_SEND
		,TH_RECV
		,TH_MAX
	};

	// スレッド情報
	struct ThreadInfo
	{
		bool bInUse;						// true=使用中
		HANDLE hThread;						// スレッドハンドル
		unsigned (__stdcall *pThreadMain)(void *);		// エントリポイント
		SOCKET InternalSock;				// 内部通信用ソケット
		struct sockaddr_in InternalAddr;	// 内部通信アドレス情報
		unsigned int TimerId;				// タイマID

		// 初期化
		void Init()
		{	bInUse = false;
			hThread = NULL;
			pThreadMain = NULL;
			InternalSock = INVALID_SOCKET;
			memset(&InternalAddr, 0, sizeof(InternalAddr));
			TimerId = NO_TIMERID;
		}
	};

	struct ThreadInfo m_ThreadInfo[TH_MAX];		// スレッド情報

};


#endif // __GUIMAIN_H__

