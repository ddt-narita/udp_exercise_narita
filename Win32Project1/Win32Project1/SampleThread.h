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
	// ������
	bool Initialize();
	// �\�P�b�g�擾
	bool GetSocket(SOCKET & sock);

	// �^�C���A�E�g�ʒm
	void OnTimeOut(unsigned int timerid);
	// ���M�f�[�^�ʒm
	void OnSendDataNotify(unsigned int iplen, const char * pIpAddr, unsigned int datalen, const char * pData);
	// �I���ʒm
	void OnTerminateNotify();

	// �f�[�^��M�ʒm
	void OnRecieveData(SOCKET sock);

    //�����ō��������
    //�\���̂𑗐M���邽�߂̊֐�
    bool SendData(SOCKET sock, const void* pData, int datalen, struct sockaddr_in a);


private:
    SOCKET udpsocket;
    int nSendStatus;                //���𑗐M���Ă���̂�
    struct sockaddr_in sendAddr;    //������IP�A�h���X�ƃ|�[�g�ԍ�
    struct MsgData msgData;         //����ׂ����b�Z�[�W�̃t�H�[�}�b�g�i�đ������̂��߃N���X�ɕۊǁj
    int nResendCounter;             //�đ��񐔂̃J�E���^
};


#endif // __SAMPLETHREAD_H__

