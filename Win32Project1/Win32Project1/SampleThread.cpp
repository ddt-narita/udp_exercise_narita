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
 * @brief �R���X�g���N�^
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
 * �X�^�[�g�������^�C�}�[�ɂ��ă^�C���A�E�g���N����������windows����GUI�ցAGUI����threadBase�ւƑ�����
 * ���̊֐����Ăяo�����B�V���ȃ^�C�}�[�������s���K�v�͂Ȃ��B
 * 
 * 
 * @param[in] msg �^�C���A�E�g�����^�C�}ID
 *
 */
void CSampleThread::OnTimeOut(unsigned int timerid)
{
    SOCKET sock = socket(AF_INET,SOCK_DGRAM, 0);

    //���b�Z�[�W�Ŏg�����^�C�}�[�ɂ��Ẵ^�C���A�E�g�̎�
    if (timerid == TIMERID_MSG) {
        //�đ���3��ȓ�
        if (nResendCounter < 3) {
            OutputResult("< ���b�Z�[�W�^�C���A�E�g�B�đ����܂��B");
            SendData(sock, &msgData.data, sizeof(msgData), sendAddr);
            StartTimer(TIMERID_MSG, 3000);
            nResendCounter++;
        }
        else {
            char temp[1024];
            snprintf(temp, sizeof(temp), "<! ���b�Z�[�W�̑��M�Ɏ��s���܂����BIP[%s]:%s", inet_ntoa(sendAddr.sin_addr), msgData.data);
            OutputResult(temp);
        }

    }//�t�@�C���𑗐M��ACK���ԋp���ꂸ�^�C���A�E�g�����Ƃ�
    else if (timerid == TIMERID_FILE_SEND) {
        OutputResult("< �����҂��^�C���A�E�g�B�I�����܂��B");

    }//�t�@�C������M�㎟�̃f�[�^�����M���ꂸ�^�C���A�E�g�����Ƃ�
    else if (timerid == TIMERID_FILE_RECIEVE) {
        OutputResult("> ��M�҂��^�C���A�E�g�B�I�����܂��B");
    }

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
	
    
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = htons(12345);
    sendAddr.sin_addr.s_addr = inet_addr(pIpAddr);
    
    //�t�@�C���̏ꍇ
    if (strstr(pData, "C:") != NULL) {
        
        struct FileData fileSendData = { 0 };
        fileSendData.header.msgid = MSGID_FILE;
        
        //�t�@�C���̑��M���s���Ă��邱�Ƃ�����
        nSendStatus = SENDSTATUS_FILE;
        
        char* pathname = "C:\test\test.txt";
        
        if (PathFileExists(pathname)) {
            OutputResult("�t�@�C������܂��B");
        }

        FILE* fp;
        if ((fp = fopen("C:\test\test.txt", "rb")) == 0) {
            exit(0);
        }
        int nFileSize = GetFileSize(fp, NULL);


        char* temp = new char[nFileSize + 1];
        fread(temp, sizeof(char), 10000, fp);
        


        StartTimer(TIMERID_FILE_SEND, 5000);
         
    
    }//���b�Z�[�W�̏ꍇ
    else {
        char temp[1024];
        snprintf(temp, sizeof(temp), "< ���b�Z�[�W�𑗐M���܂����BIP[%s]:%s", pIpAddr, pData);
        OutputResult(temp);

        nSendStatus = SENDSTATUS_MSG;
        
        //���郁�b�Z�[�W�t�H�[�}�b�g�����Z�b�g
        memset(&msgData, 0, sizeof(msgData));

        //�đ����邽�߂Ƀ����o�ϐ��̃��b�Z�[�W���i�[����
        msgData.header.msgid = MSGID_MSG;
        msgData.msgSize = (unsigned short)datalen;
        memcpy(msgData.data, pData, datalen);

        //�đ��񐔂����Z�b�g����
        nResendCounter = 0;

        //���b�Z�[�W�𑗕t����
        sendto(udpsocket,(char*) &msgData, sizeof(msgData),0,(struct sockaddr*)&sendAddr, sizeof(sendAddr));
        //�f�[�^�𑗂��ă^�C�}�[���X�^�[�g
        StartTimer(TIMERID_MSG, 3000);
        
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
                OutputResult("< ���b�Z�[�W�̑��M���������܂����B");
            }
            else if (nSendStatus == ACK_FILE) {

            }
            break;

            //���b�Z�[�W
        case MSGID_MSG:
            struct AckData msgAck;
            msgAck.header.msgid = MSGID_ACK;
            SendData(sock, &msgAck, sizeof(msgAck), senderinfo);

            struct MsgData* msg;
            msg = (struct MsgData*) msgKind;

            snprintf(temp, sizeof(temp), "> ���b�Z�[�W����M���܂����BIP[%s]:%s", inet_ntoa(senderinfo.sin_addr), msg->data);
            OutputResult(temp);
            break;
        
        case MSGID_FILE:
            //struct FileData* fileAck;
            
        default: break;
        }
        
    }
    
}

//�����Ŏ󂯎�����f�[�^�𑗐M���郁�\�b�h
bool CSampleThread::SendData(SOCKET sock, const void * pData,int datalen, struct sockaddr_in add)
{
    int status = sendto(sock, (const char*)pData, datalen, 0, (struct sockaddr*)&add, sizeof(add));
    if (status !=SOCKET_ERROR) {
        return true;
    }
    return false;
}


