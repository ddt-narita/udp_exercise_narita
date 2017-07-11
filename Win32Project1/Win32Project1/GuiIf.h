#ifndef __GUIIF_H__
#define __GUIIF_H__


const UINT SEND_DATA_CHAR_MAX = 512;
const UINT IPv4_ADDR_CHAR_MAX = 15;

const unsigned int MSGID_TIMEOUT_NOTIFY = 0x1001;		// タイムアウト通知
const unsigned int MSGID_SENDDATA_NOTIFY = 0x1002;		// 送信データ通知
const unsigned int MSGID_TERMINATE_NOTIFY = 0x1003;		// 終了通知


// 共通ヘッダ
struct CommHeader
{
	unsigned int msgid;
};

// タイムアウト通知
struct TimeoutNotify
{
	struct CommHeader header;
	unsigned int timerid;
};

// 送信データ通知
struct SendDataNotify
{
	struct CommHeader header;
	unsigned int iplen;
	char ipaddr[IPv4_ADDR_CHAR_MAX + 1];
	unsigned int datalen;
	char data[SEND_DATA_CHAR_MAX + 1];
};

// 終了通知
struct TerminateNotify
{
	struct CommHeader header;
};


// 結果出力
extern void OutputResult(const char * pResultMessage);

// タイマ開始
extern void StartTimer(int thKind, unsigned int timerid, unsigned long msec);
// タイマ停止
extern void StopTimer(int thKind, unsigned int timerid);


#endif // __GUIIF_H__

