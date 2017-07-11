#ifndef __SAMPLETHREAD_H__
#define __SAMPLETHREAD_H__

#include "ThreadBase.h"


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
    bool SendData(SOCKET sock, const void* pData, struct sockaddr_in a);

private:
    SOCKET udpsocket;
};

const unsigned short MSGID_MSG = 0x00F1;
const unsigned short MSGID_FILE = 0x00F2;
const unsigned short MSGID_ACK = 0x00F3;

struct MsgNotify {
    struct CommHeader header;
    unsigned int msgSize;
    char data[2048];
};

struct FileNotify {
    struct CommHeader header;
    unsigned int nameSize;
    char name[256];
    unsigned int dataNo;
    unsigned int dataSumNo;
    unsigned int dataSize;
    char data[2048];
};
struct AckNotify {
    struct CommHeader header;
};

#endif // __SAMPLETHREAD_H__

