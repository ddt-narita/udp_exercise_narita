#include "stdafx.h"
#include <stdio.h>

#include "GuiIf.h"
#include "ThreadBase.h"


// --------------------------------------------------------------
/*!
 * @brief �R���X�g���N�^
 *
 * �����ʐM�p�̃\�P�b�g���쐬�����[�J���A�h���X�Ƀo�C���h����B
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

	// �X���b�h��ʕێ�
	m_ThreadKind = p.thKind;

	// ��IP�A�h���X�ێ�
	sprintf_s(m_MyIpAddr, "%s", p.ipaddr);
}

// --------------------------------------------------------------
/*!
 * @brief �f�X�g���N�^
 *
 */
CThreadBase::~CThreadBase()
{
	closesocket(m_InternalSock);
}

// --------------------------------------------------------------
/*!
 * @brief ���s
 *
 * �X���b�h�̃��C�����[�v
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
		{	// �����C�x���g
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
						// �^�C���A�E�g�ʒm
						OnTimeOut(((struct TimeoutNotify *)pMsg)->timerid);
						break;

					case MSGID_SENDDATA_NOTIFY:
						{// ���M�f�[�^�ʒm
						struct SendDataNotify * pData = (struct SendDataNotify *)pMsg;
						OnSendDataNotify(pData->iplen, pData->ipaddr, pData->datalen, pData->data);
						}
						break;

					case MSGID_TERMINATE_NOTIFY:
						// �I���ʒm
						OnTerminateNotify();
						return;
					}
				}
			}

			// �O���C�x���g
			if (FD_ISSET(sock, &readfds))
			{
				// �f�[�^��M��ʒm
				OnRecieveData(sock);
			}
		}
	}
}

// --------------------------------------------------------------
/*!
 * @brief �^�C�}�J�n
 *
 * �w�肳�ꂽ�^�C�}���J�n����B
 *
 * @param[in] timerid �^�C�}ID
 * @param[in] msec    �^�C�}�l
 */
void CThreadBase::StartTimer(unsigned int timerid, unsigned long msec)
{
	// �^�C�}�J�n
	::StartTimer(m_ThreadKind, timerid, msec);
}

// --------------------------------------------------------------
/*!
 * @brief �^�C�}��~
 *
 * �^�C�}ID�Ŏw�肳�ꂽ�^�C�}���~����B
 *
 * @param[in] timerid �^�C�}ID
 */
void CThreadBase::StopTimer(unsigned int timerid)
{
	// �^�C�}��~
	::StopTimer(m_ThreadKind, timerid);
}

