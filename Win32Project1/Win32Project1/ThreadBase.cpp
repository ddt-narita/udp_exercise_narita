#include "stdafx.h"
#include <stdio.h>

#include "GuiIf.h"
#include "ThreadBase.h"


// --------------------------------------------------------------
/*!
 * @brief コンストラクタ
 *
 * 内部通信用のソケットを作成しローカルアドレスにバインドする。
 *
 */
CThreadBase::CThreadBase(PVOID param) : m_InternalSock(INVALID_SOCKET)
{
	struct ThreadParam & p = *((struct ThreadParam *)param);

	m_InternalSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET != m_InternalSock)
	{
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		addr.sin_port = htons(p.port);
		bind(m_InternalSock, (struct sockaddr *)&addr, sizeof(addr));
	}

	// スレッド種別保持
	m_ThreadKind = p.thKind;

	// 自IPアドレス保持
	sprintf_s(m_MyIpAddr, "%s", p.ipaddr);
}

// --------------------------------------------------------------
/*!
 * @brief デストラクタ
 *
 */
CThreadBase::~CThreadBase()
{
	closesocket(m_InternalSock);
}

// --------------------------------------------------------------
/*!
 * @brief 実行
 *
 * スレッドのメインループ
 *
 */
void CThreadBase::Run()
{
	if (! Initialize()) return;

	while (1)
	{
		fd_set readfds;
		SOCKET sock = INVALID_SOCKET;

		FD_ZERO(&readfds);
		FD_SET(m_InternalSock, &readfds);

		if (GetSocket(sock)) FD_SET(sock, &readfds);

		int n = select(0, &readfds, NULL, NULL, NULL);
		if (SOCKET_ERROR == n)
		{	// error ...
			continue;
		}
		else if(0 == n)
		{	// select time out ...
			continue;
		}
		else
		{	// 内部イベント
			if (FD_ISSET(m_InternalSock, &readfds))
			{
				char buff[2048] = {0};
				struct sockaddr_in addr;
				int addrlen = sizeof(addr);
				int bytes = recvfrom(m_InternalSock, buff, sizeof(buff), 0, (struct sockaddr *)&addr, &addrlen);
				if (SOCKET_ERROR != bytes)
				{
					struct CommHeader * pMsg;
					pMsg = (struct CommHeader *)buff;
					switch (pMsg->msgid)
					{
					case MSGID_TIMEOUT_NOTIFY:
						// タイムアウト通知
						OnTimeOut(((struct TimeoutNotify *)pMsg)->timerid);
						break;

					case MSGID_SENDDATA_NOTIFY:
						{// 送信データ通知
						struct SendDataNotify * pData = (struct SendDataNotify *)pMsg;
						OnSendDataNotify(pData->iplen, pData->ipaddr, pData->datalen, pData->data);
						}
						break;

					case MSGID_TERMINATE_NOTIFY:
						// 終了通知
						OnTerminateNotify();
						return;
					}
				}
			}

			// 外部イベント
			if (FD_ISSET(sock, &readfds))
			{
				// データ受信を通知
				OnRecieveData(sock);
			}
		}
	}
}

// --------------------------------------------------------------
/*!
 * @brief タイマ開始
 *
 * 指定されたタイマを開始する。
 *
 * @param[in] timerid タイマID
 * @param[in] msec    タイマ値
 */
void CThreadBase::StartTimer(unsigned int timerid, unsigned long msec)
{
	// タイマ開始
	::StartTimer(m_ThreadKind, timerid, msec);
}

// --------------------------------------------------------------
/*!
 * @brief タイマ停止
 *
 * タイマIDで指定されたタイマを停止する。
 *
 * @param[in] timerid タイマID
 */
void CThreadBase::StopTimer(unsigned int timerid)
{
	// タイマ停止
	::StopTimer(m_ThreadKind, timerid);
}

