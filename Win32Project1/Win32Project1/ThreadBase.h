#ifndef __THREADBASE_H__
#define __THREADBASE_H__

#include "stdafx.h"

#include <winsock2.h>

const unsigned short INTERNAL_PORT_ORIG = 40001;	// 内部通信用開始ポート番号

struct ThreadParam
{
	int thKind;
	char ipaddr[IPv4_ADDR_CHAR_MAX + 1];
	unsigned short port;
};

class CThreadBase
{
public:
	CThreadBase(PVOID param);
	virtual ~CThreadBase();

	void Run();

protected:
	virtual bool Initialize() = 0;
	virtual bool GetSocket(SOCKET & sock) = 0;

	virtual void OnTimeOut(unsigned int timerid) = 0;
	virtual void OnSendDataNotify(unsigned int iplen, const char * pIpAddr, unsigned int datalen, const char * pData) = 0;
	virtual void OnTerminateNotify() = 0;

	virtual void OnRecieveData(SOCKET sock) = 0;

protected:
	// 自IPアドレス取得
	const char * GetMyIpAddress() { return m_MyIpAddr; }

	// タイマ開始
	void StartTimer(unsigned int timerid, unsigned long msec);
	// タイマ停止
	void StopTimer(unsigned int timerid);

private:
	SOCKET m_InternalSock;
	int m_ThreadKind;
	char m_MyIpAddr[IPv4_ADDR_CHAR_MAX + 1];
};


#endif // __THREADBASE_H__

