#include "stdafx.h"
#include <stdio.h>
#include <process.h>

#include "GuiIf.h"
#include "SampleThread.h"

#include <fstream>
#include <iostream>
#include <string>
#include <Shlwapi.h>

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
    nResendCounter = 0;
    msgData = { 0 };
    nResendCounter = 0;
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
 * スタートさせたタイマーについてタイムアウトが起こった時にwindowsからGUIへ、GUIからthreadBaseへと送られて
 * この関数が呼び出される。新たなタイマー処理を行う必要はない。
 * 
 * 
 * @param[in] msg タイムアウトしたタイマID
 *
 */
void CSampleThread::OnTimeOut(unsigned int timerid)
{
    SOCKET sock = socket(AF_INET,SOCK_DGRAM, 0);

    //メッセージで使ったタイマーについてのタイムアウトの時
    if (timerid == TIMERID_MSG) {
        //再送回数3回以内
        if (nResendCounter < 3) {
            OutputResult("< メッセージタイムアウト。再送します。");
            SendData(sock, &msgData.data, sizeof(msgData), sendAddr);
            StartTimer(TIMERID_MSG, 3000);
            nResendCounter++;
        }
        else {
            char temp[1024];
            snprintf(temp, sizeof(temp), "<! メッセージの送信に失敗しました。IP[%s]:%s", inet_ntoa(sendAddr.sin_addr), msgData.data);
            OutputResult(temp);
        }

    }//ファイルを送信後ACKが返却されずタイムアウトしたとき
    else if (timerid == TIMERID_FILE_SEND) {
        OutputResult("< 応答待ちタイムアウト。終了します。");

    }//ファイルを受信後次のデータが送信されずタイムアウトしたとき
    else if (timerid == TIMERID_FILE_RECIEVE) {
        OutputResult("> 受信待ちタイムアウト。終了します。");
    }

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
	
    
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = htons(12345);
    sendAddr.sin_addr.s_addr = inet_addr(pIpAddr);
    
    //ファイルの場合
    if (strstr(pData, "C:") != NULL) {
        
        struct FileData fileSendData = { 0 };
        fileSendData.header.msgid = MSGID_FILE;
        
        //ファイルの送信を行っていることを示す
        nSendStatus = SENDSTATUS_FILE;
        
        char* pathname = "C:\test\test.txt";
        
        if (PathFileExists(pathname)) {
            OutputResult("ファイルあります。");
        }

        FILE* fp;
        if ((fp = fopen("C:\test\test.txt", "rb")) == 0) {
            exit(0);
        }
        int nFileSize = GetFileSize(fp, NULL);


        char* temp = new char[nFileSize + 1];
        fread(temp, sizeof(char), 10000, fp);
        


        StartTimer(TIMERID_FILE_SEND, 5000);
         
    
    }//メッセージの場合
    else {
        char temp[1024];
        snprintf(temp, sizeof(temp), "< メッセージを送信しました。IP[%s]:%s", pIpAddr, pData);
        OutputResult(temp);

        nSendStatus = SENDSTATUS_MSG;
        
        //送るメッセージフォーマットをリセット
        memset(&msgData, 0, sizeof(msgData));

        //再送するためにメンバ変数のメッセージを格納する
        msgData.header.msgid = MSGID_MSG;
        msgData.msgSize = (unsigned short)datalen;
        memcpy(msgData.data, pData, datalen);

        //再送回数をリセットする
        nResendCounter = 0;

        //メッセージを送付する
        sendto(udpsocket,(char*) &msgData, sizeof(msgData),0,(struct sockaddr*)&sendAddr, sizeof(sendAddr));
        //データを送ってタイマーをスタート
        StartTimer(TIMERID_MSG, 3000);
        
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
    char buff[2048] = { 0 };
    struct sockaddr_in senderinfo;
    /* senderinfo.sin_addr.S_un.S_addr = INADDR_ANY;
    senderinfo.sin_family = AF_INET;
    senderinfo.sin_port = htons(12345);*/
    int addrlen = sizeof(senderinfo);

    int byte = recvfrom(sock, buff, sizeof(buff), 0, (struct sockaddr*)&senderinfo, &addrlen);

    char temp[2048] = { 0 };

    struct UdpCommHeader* msgKind;
    msgKind = (struct UdpCommHeader*) buff;

    if (SOCKET_ERROR != byte) {
        switch (msgKind->msgid) {
        
            //ACK
        case MSGID_ACK:
            if (nSendStatus == ACK_MSG) {
                StopTimer(TIMERID_MSG);
                OutputResult("< メッセージの送信が完了しました。");
            }
            else if (nSendStatus == ACK_FILE) {

            }
            break;

            //メッセージ
        case MSGID_MSG:
            struct AckData msgAck;
            msgAck.header.msgid = MSGID_ACK;
            SendData(sock, &msgAck, sizeof(msgAck), senderinfo);

            struct MsgData* msg;
            msg = (struct MsgData*) msgKind;

            snprintf(temp, sizeof(temp), "> メッセージを受信しました。IP[%s]:%s", inet_ntoa(senderinfo.sin_addr), msg->data);
            OutputResult(temp);
            break;
        
        case MSGID_FILE:
            //struct FileData* fileAck;
            
        default: break;
        }
        
    }
    
}

//引数で受け取ったデータを送信するメソッド
bool CSampleThread::SendData(SOCKET sock, const void * pData,int datalen, struct sockaddr_in add)
{
    int status = sendto(sock, (const char*)pData, datalen, 0, (struct sockaddr*)&add, sizeof(add));
    if (status !=SOCKET_ERROR) {
        return true;
    }
    return false;
}


