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

	// �^�C�}�J�n
	void StartTimer(int thKind, unsigned int timerid, unsigned int msec);
	// �^�C�}��~
	void StopTimer(int thKind, unsigned int timerid);

private:
	bool OnCommand(WPARAM wParam, LPARAM lParam);
	bool OnTimerTimeout(WPARAM wParam, LPARAM lParam);

	const char * GetMyIpAddr();

	bool CreateInternalSocket(int thKind);
	bool StartThread(int thKind, unsigned (__stdcall *pThreadMain)(void *));
	bool SendInternalMessage(int thKind, const void * pData, unsigned int size);

private:
	// �E�B���h�E�n���h��
	HWND m_hDialog;			// �_�C�A���O
	HWND m_hEditOut;		// �o�̓G���A
	HWND m_hEditMsg;		// ���M�f�[�^
	HWND m_hIpAddr;			// IP�A�h���X
	HWND m_hEditMyIp;		// ��IP�A�h���X

	enum
	{	 TH_SEND
		,TH_RECV
		,TH_MAX
	};

	// �X���b�h���
	struct ThreadInfo
	{
		bool bInUse;						// true=�g�p��
		HANDLE hThread;						// �X���b�h�n���h��
		unsigned (__stdcall *pThreadMain)(void *);		// �G���g���|�C���g
		SOCKET InternalSock;				// �����ʐM�p�\�P�b�g
		struct sockaddr_in InternalAddr;	// �����ʐM�A�h���X���
		unsigned int TimerId;				// �^�C�}ID

		// ������
		void Init()
		{	bInUse = false;
			hThread = NULL;
			pThreadMain = NULL;
			InternalSock = INVALID_SOCKET;
			memset(&InternalAddr, 0, sizeof(InternalAddr));
			TimerId = NO_TIMERID;
		}
	};

	struct ThreadInfo m_ThreadInfo[TH_MAX];		// �X���b�h���

};


#endif // __GUIMAIN_H__

