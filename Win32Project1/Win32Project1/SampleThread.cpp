#include "stdafx.h"
#include <stdio.h>
#include <process.h>

#include "GuiIf.h"
#include "SampleThread.h"


unsigned int __stdcall ThreadSendMain(PVOID param)
{
    CSampleThread aThread(param);

	aThread.Run();
	return 0;
}


// --------------------------------------------------------------
/*!
 * @brief コンストラクタ
 *
 */
CSampleThread::CSampleThread(PVOID param) : CThreadBase(param)
{
}

// --------------------------------------------------------------
/*!
 * @brief デストラクタ
 *
 */
CSampleThread::~CSampleThread()
{
    CThreadBase::~CThreadBase();
}

// --------------------------------------------------------------
/*!
 * @brief 初期化
 *
 * スレッド固有の初期化が必要な場合はここで行ってください。
 * 
 * アプリケーション起動時に呼び出される
 * 
 * @retval true  成功
 * @retval false 失敗
 *
 */
bool CSampleThread::Initialize()
{
   
    int status;

    udpsocket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in udpSockAddr;

    udpSockAddr.sin_family = AF_INET;
    udpSockAddr.sin_port = htons(12345);
    udpSockAddr.sin_addr.S_un.S_addr = inet_addr(GetMyIpAddress());
    
    status = bind(udpsocket, (struct sockaddr*) &udpSockAddr, sizeof(udpSockAddr));
    if (status == 0) {
        return true;
    }
    return false;
}

// --------------------------------------------------------------
/*!
 * @brief ソケット取得
 *
 * スレッドで作成したソケットがあればここで返してください。
 * 
 * threadbaseにソケットを渡してselectでそのソケットを使えるようにする
 * 引数のソケットにinitializeで作ったソケットをIPアドレスなどをbindして返却する
 * 
 *
 * @param[out] sock データ受信用のソケット
 *
 * @retval true  ソケットを設定した場合
 * @retval false ソケットを使用していない場合
 */
bool CSampleThread::GetSocket(SOCKET & sock)
{
    
    sock = udpsocket;
   
    if (INVALID_SOCKET != sock) {
        return true;
    }

	return false;	// sockをセットした場合はtrueを返す
}

// --------------------------------------------------------------
/*!
 * @brief タイムアウト通知
 *
 * タイムアウトが発生した場合に本メソッドが呼び出されます。
 *
 * @param[in] msg タイムアウトしたタイマID
 *
 */
void CSampleThread::OnTimeOut(unsigned int timerid)
{
	OutputResult("タイムアウト発生");
}

// --------------------------------------------------------------
/*!
 * @brief 送信データ通知
 *
 * 送信ボタンが押下された場合に本メソッドが呼び出されます。
 *
 * @param[in] iplen   送信先IPアドレス長
 * @param[in] pIpAddr 送信先IPアドレス
 * @param[in] datalen 送信データ長
 * @param[in] pData   送信データ
 *
 */
void CSampleThread::OnSendDataNotify
	(unsigned int iplen
	,const char * pIpAddr
	,unsigned int datalen
	,const char * pData
	)
{
	char temp[1024];
	snprintf(temp, sizeof(temp), "送信ボタン押下 IP=%s, データ=%s", pIpAddr, pData);
	OutputResult(temp);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    
    addr.sin_addr.S_un.S_addr = inet_addr(pIpAddr);
        
    int result=  sendto(udpsocket, pData, datalen, 0, (struct sockaddr*)&addr, sizeof(addr));

	if (IDYES == ::MessageBox(NULL, "タイマを開始しますか？", "", MB_ICONINFORMATION | MB_YESNO))
	{
		StartTimer(1,3000);
		OutputResult("タイマ（3秒）開始");
	}
}

// --------------------------------------------------------------
/*!
 * @brief 終了通知
 *
 * ダイアログが閉じる場合に本メソッドが呼び出されます。
 * スレッド固有の終了処理がある場合にはここで行ってください。
 *
 */
void CSampleThread::OnTerminateNotify()
{
	OutputResult("スレッド終了");

    WSACleanup();

}

// --------------------------------------------------------------
/*!
 * @brief データ受信通知
 *
 * GetSocket()で設定したソケットにデータを受信した場合に本メソッドが呼び出されます。
 *
 * @param[in] sock ソケット
 *
 */
void CSampleThread::OnRecieveData(SOCKET sock)
{
    char buff[2048];
    struct sockaddr_in senderinfo;
    senderinfo.sin_addr.S_un.S_addr = INADDR_ANY;
    senderinfo.sin_family = AF_INET;
    senderinfo.sin_port = htons(12345);
    int addrlen = sizeof(senderinfo);

    int byte = recvfrom(sock, buff, sizeof(buff), 0, (struct sockaddr*)&senderinfo, &addrlen);
    char temp[1024];
    snprintf(temp, sizeof(temp), "受信しました。IP[%s],メッセージ：%s", inet_ntoa(senderinfo.sin_addr), buff);
    OutputResult(temp);
    StopTimer(1);

    struct CommHeader* msgKind;
    msgKind = (struct CommHeader*) buff;
   
    if (SOCKET_ERROR != byte) {
        switch (msgKind->msgid) {
        case MSGID_ACK:
            OutputResult("反応あり");
            break;

        //メッセージ
        case MSGID_MSG:
            struct MsgNotify ack;
            ack.header.msgid = MSGID_MSG;
            SendData(udpsocket, &ack, senderinfo);
            snprintf(temp, sizeof(temp), "受信しました。IP[%s],メッセージ：%s", inet_ntoa(senderinfo.sin_addr), buff);
            OutputResult(temp);
            StopTimer(1);
        }
    }
}

bool CSampleThread::SendData(SOCKET sock, const void * pData, struct sockaddr_in add)
{
    sendto(sock, (const char*)pData, sizeof(pData), 0, (struct sockaddr*)&add, sizeof(add));
    return true;
}
