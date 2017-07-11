#ifndef __GUIIF_H__
#define __GUIIF_H__


const UINT SEND_DATA_CHAR_MAX = 512;
const UINT IPv4_ADDR_CHAR_MAX = 15;

const unsigned int MSGID_TIMEOUT_NOTIFY = 0x1001;		// �^�C���A�E�g�ʒm
const unsigned int MSGID_SENDDATA_NOTIFY = 0x1002;		// ���M�f�[�^�ʒm
const unsigned int MSGID_TERMINATE_NOTIFY = 0x1003;		// �I���ʒm


// ���ʃw�b�_
struct CommHeader
{
	unsigned int msgid;
};

// �^�C���A�E�g�ʒm
struct TimeoutNotify
{
	struct CommHeader header;
	unsigned int timerid;
};

// ���M�f�[�^�ʒm
struct SendDataNotify
{
	struct CommHeader header;
	unsigned int iplen;
	char ipaddr[IPv4_ADDR_CHAR_MAX + 1];
	unsigned int datalen;
	char data[SEND_DATA_CHAR_MAX + 1];
};

// �I���ʒm
struct TerminateNotify
{
	struct CommHeader header;
};


// ���ʏo��
extern void OutputResult(const char * pResultMessage);

// �^�C�}�J�n
extern void StartTimer(int thKind, unsigned int timerid, unsigned long msec);
// �^�C�}��~
extern void StopTimer(int thKind, unsigned int timerid);


#endif // __GUIIF_H__

