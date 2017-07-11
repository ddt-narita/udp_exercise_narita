#include "stdafx.h"

#include "GuiMain.h"
#include "GuiIf.h"


// --------------------------------------------------------------
/*!
 * @brief �o�̓G���A�ւ̌��ʕ\��
 *
 * �w�肳�ꂽ���ʂ��o�̓G���A�֕\������B
 *
 * @param[in] pResultMessage ���ʂ̃��b�Z�[�W�iNULL�I�[����Ă��邱�Ɓj
 */
void OutputResult(const char * pResultMessage)
{
	CGuiMain * pGUI = CGuiMain::GetGuiMain();
	if (NULL == pGUI) return;
	pGUI->OutputResult(pResultMessage);
}

// --------------------------------------------------------------
/*!
 * @brief �^�C�}�J�n
 *
 * 
 *
 * @param[in] 
 */
void StartTimer(int thKind, unsigned int timerid, unsigned long msec)
{
	CGuiMain * pGUI = CGuiMain::GetGuiMain();
	if (NULL == pGUI) return;
	pGUI->StartTimer(thKind, timerid, msec);
}

// --------------------------------------------------------------
/*!
 * @brief �^�C�}��~
 *
 * 
 *
 * @param[in] 
 */
void StopTimer(int thKind, unsigned int timerid)
{
	CGuiMain * pGUI = CGuiMain::GetGuiMain();
	if (NULL == pGUI) return;
	pGUI->StopTimer(thKind, timerid);
}

