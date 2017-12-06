﻿#include "stdafx.h"

struct EditStreamData
{
	EditStreamData() { pbBuff = nullptr; cbBuff = iCurrent = 0; }
	~EditStreamData() { free(pbBuff); }

	PBYTE pbBuff;
	int cbBuff;
	int iCurrent;
};

static DWORD CALLBACK EditStreamOutRtf(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	EditStreamData *esd = (EditStreamData*)dwCookie;
	esd->cbBuff += cb;
	esd->pbBuff = (PBYTE)realloc(esd->pbBuff, esd->cbBuff + sizeof(wchar_t));
	memcpy(esd->pbBuff + esd->iCurrent, pbBuff, cb);
	esd->iCurrent += cb;
	esd->pbBuff[esd->iCurrent] = 0;
	esd->pbBuff[esd->iCurrent + 1] = 0;

	*pcb = cb;
	return 0;
}

LPTSTR GeTStringFromStreamData(EditStreamData *esd)
{
	DWORD i, k;

	LPTSTR ptszOutText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t));
	LPTSTR ptszTemp = (wchar_t*)esd->pbBuff;
	
	for (i = k = 0; i < mir_wstrlen(ptszTemp); i++) {
		if ((ptszTemp[i] == 0x0A) || (ptszTemp[i] == 0x2028))
			ptszOutText[k++] = 0x0D;
		else if (ptszTemp[i] == 0x0D) {
			ptszOutText[k++] = 0x0D;
			if (ptszTemp[i + 1] == 0x0A) i++;
		}
		else ptszOutText[k++] = ptszTemp[i];
	}

	ptszOutText[k] = 0;
	return ptszOutText;
}


BOOL CopyTextToClipboard(LPTSTR ptszText)
{
	if (!OpenClipboard(nullptr))
		return FALSE;

	EmptyClipboard(); 
	HGLOBAL hCopy = GlobalAlloc(GMEM_MOVEABLE, (mir_wstrlen(ptszText) + 1)*sizeof(wchar_t));
	mir_wstrcpy((wchar_t*)GlobalLock(hCopy), ptszText);
	GlobalUnlock(hCopy);
	SetClipboardData(CF_UNICODETEXT, hCopy);
	CloseClipboard();
	return TRUE;
}

LPSTR GetNameOfLayout(HKL hklLayout)
{
	LPSTR ptszLayName = (LPSTR)mir_alloc(KL_NAMELENGTH + 1);
	mir_snprintf(ptszLayName, KL_NAMELENGTH + 1, "%08x", hklLayout);
	return ptszLayName;
}

LPTSTR GetShortNameOfLayout(HKL hklLayout)
{
	wchar_t szLI[20];
	LPTSTR ptszLiShort = (LPTSTR)mir_alloc(3*sizeof(wchar_t));
	DWORD dwLcid = MAKELCID(LOWORD(hklLayout), 0);
	GetLocaleInfo(dwLcid, LOCALE_SISO639LANGNAME, szLI, 10);
	ptszLiShort[0] = toupper(szLI[0]);
	ptszLiShort[1] = toupper(szLI[1]);
	ptszLiShort[2] = 0;
	return ptszLiShort;
}

HKL GetNextLayout(HKL hklCurLay)
{
	for (DWORD i = 0; i < bLayNum; i++)
		if (hklLayouts[i] == hklCurLay)
			return hklLayouts[(i+1)%(bLayNum)];
	
	return nullptr;
}

LPTSTR GenerateLayoutString(HKL hklLayout)
{
	BYTE bState[256] = {0};

	LPTSTR ptszLayStr = (LPTSTR)mir_alloc(100 * sizeof(wchar_t));
	LPTSTR ptszTemp = (LPTSTR)mir_alloc(3 * sizeof(wchar_t));
	ptszTemp[0] = 0;

	DWORD i;
	for (i = 0; i < mir_wstrlen(ptszKeybEng); i++) {
		SHORT shVirtualKey = VkKeyScanEx(ptszKeybEng[i], hklEng);
		UINT iScanCode = MapVirtualKeyEx(shVirtualKey & 0x00FF, 0, hklEng);

		for (DWORD j = 0; j < 256; j++)
			bState[j] = 0;

		if (shVirtualKey & 0x0100) bState[VK_SHIFT] = 0x80;
		if (shVirtualKey & 0x0200) bState[VK_CONTROL] = 0x80;
		if (shVirtualKey & 0x0400) bState[VK_MENU] = 0x80;

		shVirtualKey = MapVirtualKeyEx(iScanCode, 1, hklLayout);
		bState[shVirtualKey & 0x00FF] = 0x80;

		int iRes = ToUnicodeEx(shVirtualKey, iScanCode, bState, ptszTemp, 3, 0, hklLayout);
		// Защита от дэд-кеев
		if (iRes < 0)
			ToUnicodeEx(shVirtualKey, iScanCode, bState, ptszTemp, 3, 0, hklLayout);

		// Если нам вернули нулевой символ, или не вернули ничего, то присвоим "звоночек"
		if (ptszTemp[0] == 0)
			ptszLayStr[i] = 3;
		else
			ptszLayStr[i] = ptszTemp[0];
	}
	ptszLayStr[i] = 0;

	mir_free(ptszTemp);
	return ptszLayStr;
}

LPTSTR GetLayoutString(HKL hklLayout)
{
	for (DWORD i = 0; i < bLayNum; i++)
		if (hklLayouts[i] == hklLayout)
			return ptszLayStrings[i];
	return nullptr;
}

LPTSTR ChangeTextCase(LPCTSTR ptszInText)
{
	LPTSTR ptszOutText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t));
	mir_wstrcpy(ptszOutText, ptszInText);

	for (DWORD i = 0; i < mir_wstrlen(ptszInText); i++) {
		CharUpperBuff(&ptszOutText[i], 1);
		if (ptszOutText[i] == ptszInText[i])
			CharLowerBuff(&ptszOutText[i], 1);
	}
	return ptszOutText;
}

LPTSTR ChangeTextLayout(LPCTSTR ptszInText, HKL hklCurLay, HKL hklToLay, BOOL TwoWay)
{
	LPTSTR ptszOutText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t));
	mir_wstrcpy(ptszOutText, ptszInText);

	if (hklCurLay == nullptr || hklToLay == nullptr)
		return ptszOutText;

	LPTSTR ptszKeybCur = GetLayoutString(hklCurLay);
	LPTSTR ptszKeybNext = GetLayoutString(hklToLay);
	if (ptszKeybCur == nullptr || ptszKeybNext == nullptr)
		return ptszOutText;

	for (DWORD i = 0; i < mir_wstrlen(ptszInText); i++) {
		BOOL Found = FALSE;
		for (DWORD j = 0; j < mir_wstrlen(ptszKeybCur) && !Found; j++)
		if (ptszKeybCur[j] == ptszInText[i]) {
			Found = TRUE;
			if (mir_wstrlen(ptszKeybNext) >= j)
				ptszOutText[i] = ptszKeybNext[j];
		}

		if (TwoWay && !Found)
		for (DWORD j = 0; j < mir_wstrlen(ptszKeybNext) && !Found; j++)
		if (ptszKeybNext[j] == ptszInText[i]) {
			Found = TRUE;
			if (mir_wstrlen(ptszKeybCur) >= j)
				ptszOutText[i] = ptszKeybCur[j];
		}
	}
	return ptszOutText;
}

HKL GetLayoutOfText(LPCTSTR ptszInText)
{
	HKL hklCurLay = hklLayouts[0];
	LPTSTR ptszKeybBuff = ptszLayStrings[0];
	DWORD dwMaxSymbols = 0, dwTemp = 0;

	for (DWORD j = 0; j < mir_wstrlen(ptszInText); j++)
		if (wcschr(ptszKeybBuff, ptszInText[j]) != nullptr)
			++dwMaxSymbols;

	for (DWORD i = 1; i < bLayNum; i++) {
		ptszKeybBuff = ptszLayStrings[i];
		DWORD dwCountSymbols = 0;
			
		for (DWORD j = 0; j<mir_wstrlen(ptszInText); j++)
			if (wcschr(ptszKeybBuff, ptszInText[j]) != nullptr)
				++dwCountSymbols;
		
		if (dwCountSymbols == dwMaxSymbols)
			dwTemp = dwCountSymbols;
		else if (dwCountSymbols>dwMaxSymbols) {
			dwMaxSymbols = dwCountSymbols;
			hklCurLay = hklLayouts[i];
		}
	}

	if (dwMaxSymbols == dwTemp)
		hklCurLay = GetKeyboardLayout(0);
	
	return hklCurLay;
}

int ChangeLayout(HWND hTextWnd, BYTE TextOperation, BOOL CurrentWord)
{
	HKL hklCurLay = nullptr, hklToLay = nullptr;

	ptrW ptszInText;
	CHARRANGE crSelection = { 0 }, crTemp = { 0 };
	DWORD dwStartWord, dwEndWord;
	int i, iRes;

	BYTE WindowType = WTYPE_Unknown;
	BOOL WindowIsReadOnly, TwoWay;

	if (hTextWnd == nullptr)
		hTextWnd = GetFocus();

	if (hTextWnd == nullptr)
		return 0;

	//--------------Определяем тип окна-----------------
	IEVIEWEVENT ieEvent = { 0 };
	ieEvent.cbSize = sizeof(IEVIEWEVENT);
	ieEvent.iType = IEE_GET_SELECTION;

	if (ServiceExists(MS_HPP_EG_EVENT)) {
		// То же самое для History++
		ieEvent.hwnd = hTextWnd;
		ptszInText = (wchar_t*)CallService(MS_HPP_EG_EVENT, 0, (LPARAM)&ieEvent);

		if (!IsBadStringPtr(ptszInText, MaxTextSize))
			WindowType = WTYPE_HistoryPP;
	}

	if ((WindowType == WTYPE_Unknown) && (ServiceExists(MS_IEVIEW_EVENT))) {
		// Извращенное определение хэндла IEView
		ieEvent.hwnd = GetParent(GetParent(hTextWnd));

		ptszInText = (wchar_t*)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&ieEvent);
		if (!IsBadStringPtr(ptszInText, MaxTextSize))
			WindowType = WTYPE_IEView;
	}

	if (WindowType == WTYPE_Unknown) {
		ptrW ptszTemp((LPTSTR)mir_alloc(255 * sizeof(wchar_t)));
		i = GetClassName(hTextWnd, ptszTemp, 255);
		ptszTemp[i] = 0;

		if (wcsstr(CharUpper(ptszTemp), L"RICHEDIT") != nullptr) {
			WindowType = WTYPE_RichEdit;
			SendMessage(hTextWnd, EM_EXGETSEL, 0, (LPARAM)&crSelection);
		}
	}

	if (WindowType == WTYPE_Unknown) {
		SendMessage(hTextWnd, EM_GETSEL, (WPARAM)&crSelection.cpMin, (LPARAM)&crSelection.cpMax);
		if ((SendMessage(hTextWnd, WM_GETDLGCODE, 0, 0)&(DLGC_HASSETSEL)) && (crSelection.cpMin >= 0))
			WindowType = WTYPE_Edit;
	}

	// Получим текст из Рича или обычного Едита
	if (WindowType == WTYPE_RichEdit || WindowType == WTYPE_Edit) {
		dwStartWord = dwEndWord = -1;
		SendMessage(hTextWnd, WM_SETREDRAW, FALSE, 0);

		// Бэкап выделения
		crTemp = crSelection;

		// Если имеется выделенный текст, то получим его
		if (crSelection.cpMin != crSelection.cpMax) {
			if (WindowType == WTYPE_RichEdit) {
				EditStreamData esdData;
				EDITSTREAM esStream = { 0 };
				esStream.dwCookie = (DWORD_PTR)&esdData;
				esStream.pfnCallback = EditStreamOutRtf;
				if (SendMessage(hTextWnd, EM_STREAMOUT, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM)&esStream) > 0)
					ptszInText = GeTStringFromStreamData(&esdData);
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, nullptr, FALSE);
					return 1;
				}
			}
			if (WindowType == WTYPE_Edit) {
				ptrW ptszTemp((LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t)));
				ptszInText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t));
				iRes = GetWindowText(hTextWnd, ptszTemp, MaxTextSize);
				if (!IsBadStringPtr(ptszInText, MaxTextSize) && (iRes > 0)) {
					wcsncpy(ptszInText, &ptszTemp[crSelection.cpMin], crSelection.cpMax - crSelection.cpMin);
					ptszInText[crSelection.cpMax - crSelection.cpMin] = 0;
				}
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, nullptr, FALSE);
					return 1;
				}
			}
		}
		// Если выделения нет, то получим нужный текст
		else {
			// Получаем весь текст в поле
			if (WindowType == WTYPE_RichEdit) {
				crTemp.cpMin = 0;
				crTemp.cpMax = -1;
				SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crTemp);

				EditStreamData esdData;
				EDITSTREAM esStream = { 0 };
				esStream.dwCookie = (DWORD_PTR)&esdData;
				esStream.pfnCallback = EditStreamOutRtf;
				if (SendMessage(hTextWnd, EM_STREAMOUT, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM)&esStream) != 0)
					ptszInText = GeTStringFromStreamData(&esdData);
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, nullptr, FALSE);
					return 1;
				}
			}
			if (WindowType == WTYPE_Edit) {
				ptszInText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t));
				iRes = GetWindowText(hTextWnd, ptszInText, MaxTextSize);

				if (!IsBadStringPtr(ptszInText, MaxTextSize) && (iRes > 0)) {
					crTemp.cpMin = 0;
					crTemp.cpMax = (int)mir_wstrlen(ptszInText);
				}
				else {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
					SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
					InvalidateRect(hTextWnd, nullptr, FALSE);
					return 1;
				}
			}
			// Получаем текущее слово
			if (CurrentWord) {
				for (dwStartWord = crSelection.cpMin; (dwStartWord > 0) && (wcschr(ptszSeparators, ptszInText[dwStartWord - 1]) == nullptr); dwStartWord--);
				for (dwEndWord = crSelection.cpMin; (dwEndWord < (mir_wstrlen(ptszInText))) && (wcschr(ptszSeparators, ptszInText[dwEndWord]) == nullptr); dwEndWord++);

				crTemp.cpMin = dwStartWord;
				crTemp.cpMax = dwEndWord;

				if (WindowType == WTYPE_RichEdit) {
					SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crTemp);

					EditStreamData esdData;
					EDITSTREAM esStream = { 0 };
					esStream.dwCookie = (DWORD_PTR)&esdData;
					esStream.pfnCallback = EditStreamOutRtf;
					if (SendMessage(hTextWnd, EM_STREAMOUT, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM)&esStream) != 0)
						ptszInText = GeTStringFromStreamData(&esdData);
					else {
						SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
						SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
						InvalidateRect(hTextWnd, nullptr, FALSE);
						return 1;
					}
				}

				if (WindowType == WTYPE_Edit) {
					ptrW ptszTemp((LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t)));
					wcsncpy(ptszTemp, &ptszInText[crTemp.cpMin], crTemp.cpMax - crTemp.cpMin);
					ptszTemp[crTemp.cpMax - crTemp.cpMin] = 0;
					mir_wstrcpy(ptszInText, ptszTemp);

					if (mir_wstrlen(ptszInText) == 0) {
						SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
						SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
						InvalidateRect(hTextWnd, nullptr, FALSE);
						return 1;
					}
				}
			}
		}
	}

	//---------------Выдаем результаты--------------------
	WindowIsReadOnly = FALSE;
	if (WindowType == WTYPE_IEView || WindowType == WTYPE_HistoryPP)
		WindowIsReadOnly = TRUE;

	// if ((SendMessage(hTextWnd, EM_GETOPTIONS, 0, 0)&ECO_READONLY))
	if (WindowType == WTYPE_RichEdit || WindowType == WTYPE_Edit)
	if (GetWindowLongPtr(hTextWnd, GWL_STYLE) & ES_READONLY)
		WindowIsReadOnly = TRUE;

	// Лог Иевью и ХисториПП в режиме эмуляции Иевью  и поля только для чтения.
	if (WindowType != WTYPE_Unknown && !IsBadStringPtr(ptszInText, MaxTextSize))
	if (WindowIsReadOnly) {
		ptrW ptszMBox((LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t)));
		ptszMBox[0] = 0;

		if (TextOperation == TOT_Layout) {
			hklCurLay = GetLayoutOfText(ptszInText);
			hklToLay = GetNextLayout(hklCurLay);
			TwoWay = (moOptions.TwoWay) && (bLayNum == 2);

			if (bLayNum == 2)
				ptszMBox = ChangeTextLayout(ptszInText, hklCurLay, hklToLay, TwoWay);
			else {
				for (i = 0; i < bLayNum; i++)
					if (hklLayouts[i] != hklCurLay) {
						if (mir_wstrlen(ptszMBox) != 0)
							mir_wstrcat(ptszMBox, L"\n\n");
						ptrW ptszTemp(GetShortNameOfLayout(hklLayouts[i]));
						mir_wstrcat(ptszMBox, ptszTemp);
						mir_wstrcat(ptszMBox, L":\n");
						ptrW ptszOutText(ChangeTextLayout(ptszInText, hklCurLay, hklLayouts[i], FALSE));
						mir_wstrcat(ptszMBox, ptszOutText);
					}
			}
		}
		else if (TextOperation == TOT_Case)
			ptszMBox = ChangeTextCase(ptszInText);

		if ((WindowType == WTYPE_Edit) || (WindowType == WTYPE_RichEdit)) {
			SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
			SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(hTextWnd, nullptr, FALSE);
		}

		if (TextOperation == TOT_Layout)
			Skin_PlaySound(SND_ChangeLayout);
		else if (TextOperation == TOT_Case)
			Skin_PlaySound(SND_ChangeCase);

		if (moOptions.CopyToClipboard)
			CopyTextToClipboard(ptszMBox);

		//-------------------------------Покажем попапы------------------------------------------ 			
		if (moOptions.ShowPopup) {
			LPTSTR ptszPopupText = (LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t));
			mir_wstrcpy(ptszPopupText, ptszMBox);

			POPUPDATAT_V2 pdtData = { 0 };
			pdtData.cbSize = sizeof(pdtData);
			wcsncpy(pdtData.lptzContactName, TranslateT(ModuleName), MAX_CONTACTNAME);
			wcsncpy(pdtData.lptzText, ptszPopupText, MAX_SECONDLINE);

			switch (poOptions.bColourType) {
			case PPC_POPUP:
				pdtData.colorBack = pdtData.colorText = 0;
				break;
			case PPC_WINDOWS:
				pdtData.colorBack = GetSysColor(COLOR_BTNFACE);
				pdtData.colorText = GetSysColor(COLOR_WINDOWTEXT);
				break;
			case PPC_CUSTOM:
				pdtData.colorBack = poOptions.crBackColour;
				pdtData.colorText = poOptions.crTextColour;
				break;
			}

			switch (poOptions.bTimeoutType) {
			case PPT_POPUP:
				pdtData.iSeconds = 0;
				break;
			case PPT_PERMANENT:
				pdtData.iSeconds = -1;
				break;
			case PPC_CUSTOM:
				pdtData.iSeconds = poOptions.bTimeout;
				break;
			}
			pdtData.PluginData = ptszPopupText;
			pdtData.PluginWindowProc = (WNDPROC)CKLPopupDlgProc;

			pdtData.lchIcon = hPopupIcon;
			poOptions.paActions[0].lchIcon = hCopyIcon;
			pdtData.lpActions = poOptions.paActions;
			pdtData.actionCount = 1;

			if (CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&pdtData, APF_NEWDATA) < 0) {
				mir_free(ptszPopupText);
				MessageBox(nullptr, ptszMBox, TranslateT(ModuleName), MB_ICONINFORMATION);
			}
		}
	}
	//------------------Редактируемые поля ----------------------------
	else {
		ptrW ptszOutText;
		if (TextOperation == TOT_Layout) {
			hklCurLay = GetLayoutOfText(ptszInText);
			hklToLay = GetNextLayout(hklCurLay);
			TwoWay = (moOptions.TwoWay) && (bLayNum == 2);
			ptszOutText = ChangeTextLayout(ptszInText, hklCurLay, hklToLay, TwoWay);
		}
		else if (TextOperation == TOT_Case)
			ptszOutText = ChangeTextCase(ptszInText);

		if (WindowType == WTYPE_RichEdit) {
			SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crTemp);
			SendMessage(hTextWnd, EM_REPLACESEL, FALSE, (LPARAM)ptszOutText);
			SendMessage(hTextWnd, EM_EXSETSEL, 0, (LPARAM)&crSelection);
		}
		else {
			ptrW ptszTemp((LPTSTR)mir_alloc(MaxTextSize*sizeof(wchar_t)));
			GetWindowText(hTextWnd, ptszTemp, MaxTextSize);
			for (i = crTemp.cpMin; i < crTemp.cpMax; i++)
				ptszTemp[i] = ptszOutText[i - crTemp.cpMin];
			SetWindowText(hTextWnd, ptszTemp);
			SendMessage(hTextWnd, EM_SETSEL, crSelection.cpMin, crSelection.cpMax);
		}

		// Переключим раскладку или изменим состояние Caps Lock
		if (TextOperation == TOT_Layout && hklToLay != nullptr && moOptions.ChangeSystemLayout)
			ActivateKeyboardLayout(hklToLay, KLF_SETFORPROCESS);
		else if (TextOperation == TOT_Case) {
			// Если нужно инвертнуть
			if (moOptions.bCaseOperations == 0) {
				keybd_event(VK_CAPITAL, 0x45, 0, 0);
				keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_KEYUP, 0);
			}
			// Если нужно отключить
			else if (moOptions.bCaseOperations == 1) {
				if (GetKeyState(VK_CAPITAL) & 0x0001) {
					keybd_event(VK_CAPITAL, 0x45, 0, 0);
					keybd_event(VK_CAPITAL, 0x45, KEYEVENTF_KEYUP, 0);
				}
			}
		}

		SendMessage(hTextWnd, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(hTextWnd, nullptr, FALSE);

		if (TextOperation == TOT_Layout)
			Skin_PlaySound(SND_ChangeLayout);
		else if (TextOperation == TOT_Case)
			Skin_PlaySound(SND_ChangeCase);
	}

	return 0;
}
