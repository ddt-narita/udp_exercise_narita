#include "stdafx.h"

#include "GuiMain.h"
#include "GuiIf.h"


// --------------------------------------------------------------
/*!
 * @brief 出力エリアへの結果表示
 *
 * 指定された結果を出力エリアへ表示する。
 *
 * @param[in] pResultMessage 結果のメッセージ（NULL終端されていること）
 */
void OutputResult(const char * pResultMessage)
{
	CGuiMain * pGUI = CGuiMain::GetGuiMain();
	if (NULL == pGUI) return;
	pGUI->OutputResult(pResultMessage);
}

// --------------------------------------------------------------
/*!
 * @brief タイマ開始
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
 * @brief タイマ停止
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

