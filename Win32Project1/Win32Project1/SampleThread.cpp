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
 * @brief �R���X�g���N�^
 *
 */
CSampleThread::CSampleThread(PVOID param) : CThreadBase(param)
{
}

// --------------------------------------------------------------
/*!
 * @brief �f�X�g���N�^
 *
 */
CSampleThread::~CSampleThread()
{
    CThreadBase::~CThreadBase();
}

// --------------------------------------------------------------
/*!
 * @brief ������
 *
 * �X���b�h�ŗL�̏��������K�v�ȏꍇ�͂����ōs���Ă��������B
 * 
 * �A�v���P�[�V�����N�����ɌĂяo�����
 * 
 * @retval true  ����
 * @retval false ���s
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
 * @brief �\�P�b�g�擾
 *
 * �X���b�h�ō쐬�����\�P�b�g������΂����ŕԂ��Ă��������B
 * 
 * threadbase�Ƀ\�P�b�g��n����select�ł��̃\�P�b�g���g����悤�ɂ���
 * �����̃\�P�b�g��initialize�ō�����\�P�b�g��IP�A�h���X�Ȃǂ�bind���ĕԋp����
 * 
 *
 * @param[out] sock �f�[�^��M�p�̃\�P�b�g
 *
 * @retval true  �\�P�b�g��ݒ肵���ꍇ
 * @retval false �\�P�b�g���g�p���Ă��Ȃ��ꍇ
 */
bool CSampleThread::GetSocket(SOCKET & sock)
{
    
    sock = udpsocket;
   
    if (INVALID_SOCKET != sock) {
        return true;
    }

	return false;	// sock���Z�b�g�����ꍇ��true��Ԃ�
}

// --------------------------------------------------------------
/*!
 * @brief �^�C���A�E�g�ʒm
 *
 * �^�C���A�E�g�����������ꍇ�ɖ{���\�b�h���Ăяo����܂��B
 *
 * @param[in] msg �^�C���A�E�g�����^�C�}ID
 *
 */
void CSampleThread::OnTimeOut(unsigned int timerid)
{
	OutputResult("�^�C���A�E�g����");
}

// --------------------------------------------------------------
/*!
 * @brief ���M�f�[�^�ʒm
 *
 * ���M�{�^�����������ꂽ�ꍇ�ɖ{���\�b�h���Ăяo����܂��B
 *
 * @param[in] iplen   ���M��IP�A�h���X��
 * @param[in] pIpAddr ���M��IP�A�h���X
 * @param[in] datalen ���M�f�[�^��
 * @param[in] pData   ���M�f�[�^
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
	snprintf(temp, sizeof(temp), "���M�{�^������ IP=%s, �f�[�^=%s", pIpAddr, pData);
	OutputResult(temp);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    
    addr.sin_addr.S_un.S_addr = inet_addr(pIpAddr);
        
    int result=  sendto(udpsocket, pData, datalen, 0, (struct sockaddr*)&addr, sizeof(addr));

	if (IDYES == ::MessageBox(NULL, "�^�C�}���J�n���܂����H", "", MB_ICONINFORMATION | MB_YESNO))
	{
		StartTimer(1,3000);
		OutputResult("�^�C�}�i3�b�j�J�n");
	}
}

// --------------------------------------------------------------
/*!
 * @brief �I���ʒm
 *
 * �_�C�A���O������ꍇ�ɖ{���\�b�h���Ăяo����܂��B
 * �X���b�h�ŗL�̏I������������ꍇ�ɂ͂����ōs���Ă��������B
 *
 */
void CSampleThread::OnTerminateNotify()
{
	OutputResult("�X���b�h�I��");

    WSACleanup();

}

// --------------------------------------------------------------
/*!
 * @brief �f�[�^��M�ʒm
 *
 * GetSocket()�Őݒ肵���\�P�b�g�Ƀf�[�^����M�����ꍇ�ɖ{���\�b�h���Ăяo����܂��B
 *
 * @param[in] sock �\�P�b�g
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
    snprintf(temp, sizeof(temp), "��M���܂����BIP[%s],���b�Z�[�W�F%s", inet_ntoa(senderinfo.sin_addr), buff);
    OutputResult(temp);
    StopTimer(1);

    struct CommHeader* msgKind;
    msgKind = (struct CommHeader*) buff;
   
    if (SOCKET_ERROR != byte) {
        switch (msgKind->msgid) {
        case MSGID_ACK:
            OutputResult("��������");
            break;

        //���b�Z�[�W
        case MSGID_MSG:
            struct MsgNotify ack;
            ack.header.msgid = MSGID_MSG;
            SendData(udpsocket, &ack, senderinfo);
            snprintf(temp, sizeof(temp), "��M���܂����BIP[%s],���b�Z�[�W�F%s", inet_ntoa(senderinfo.sin_addr), buff);
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
