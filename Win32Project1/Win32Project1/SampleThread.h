#ifndef __SAMPLETHREAD_H__
#define __SAMPLETHREAD_H__

#include "ThreadBase.h"

const int ACK_MSG = 1;
const int ACK_FILE = 2;

const int TIMERID_MSG = 1;
const int TIMERID_FILE_SEND = 2;
const int TIMERID_FILE_RECIEVE = 3;

const int SEND_DATA_MAX = 512;
const int FILE_NAME_MAX = 75;

const int SENDSTATUS_MSG = 1;
const int SENDSTATUS_FILE = 2;

const unsigned short MSGID_MSG = 0x00F1;
const unsigned short MSGID_FILE = 0x00F2;
const unsigned short MSGID_ACK = 0x00F3;

struct UdpCommHeader {
    unsigned short msgid;
};

struct MsgData {
    struct UdpCommHeader header;
    unsigned short msgSize;
    char data[SEND_DATA_MAX];
};

struct FileData {
    struct UdpCommHeader header;
    unsigned short nameSize;
    char name[FILE_NAME_MAX + 1];
    unsigned short dataNo;
    unsigned short dataSumNo;
    unsigned short dataSize;
    char data[SEND_DATA_MAX];
};

struct AckData {
    struct UdpCommHeader header;
};

class CSampleThread : public CThreadBase
{
public:
    CSampleThread(PVOID param);
	virtual ~CSampleThread();


protected:
	// 初期化
	bool Initialize();
	// ソケット取得
	bool GetSocket(SOCKET & sock);

	// タイムアウト通知
	void OnTimeOut(unsigned int timerid);
	// 送信データ通知
	void OnSendDataNotify(unsigned int iplen, const char * pIpAddr, unsigned int datalen, const char * pData);
	// 終了通知
	void OnTerminateNotify();

	// データ受信通知
	void OnRecieveData(SOCKET sock);

    //自分で作ったもの
    //構造体を送信するための関数
    bool SendData(SOCKET sock, const void* pData, int datalen, struct sockaddr_in a);


private:
    SOCKET udpsocket;
    int nSendStatus;                //何を送信しているのか
    struct sockaddr_in sendAddr;    //送り先のIPアドレスとポート番号
    struct MsgData msgData;         //送るべきメッセージのフォーマット（再送処理のためクラスに保管）
    int nResendCounter;             //再送回数のカウンタ
};


#endif // __SAMPLETHREAD_H__

