/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK
© 2010 Merlin_de

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

int num_classes = 0;			// for core class api support

//===== Popup/AddPopup
INT_PTR Popup_AddPopup(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded)
		return -1;

	POPUPDATA *ppd = (POPUPDATA*)wParam;
	if (!ppd)
		return -1;

	ptrW wszText(mir_a2u(ppd->lpzText)), wszTitle(mir_a2u(ppd->lpzContactName));

	POPUPDATA2 ppd2 = { sizeof(ppd2) };
	ppd2.flags = PU2_UNICODE;
	ppd2.lchContact = ppd->lchContact;
	ppd2.lchIcon = ppd->lchIcon;
	ppd2.lpwzTitle = wszTitle;
	ppd2.lpwzText = wszText;
	ppd2.colorBack = ppd->colorBack;
	ppd2.colorText = ppd->colorText;
	ppd2.PluginWindowProc = ppd->PluginWindowProc;
	ppd2.PluginData = ppd->PluginData;
	ppd2.iSeconds = ppd->iSeconds;
	return Popup_AddPopup2((WPARAM)&ppd2, lParam);
}

//===== Popup/AddPopupW
INT_PTR Popup_AddPopupW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded)
		return -1;

	POPUPDATAW_V2 *ppd = (POPUPDATAW_V2*)wParam;
	if (!ppd)
		return -1;

	POPUPDATA2 ppd2 = { 0 };
	ppd2.cbSize = sizeof(ppd2);
	ppd2.flags = PU2_UNICODE;
	ppd2.lchContact = ppd->lchContact;
	ppd2.lchIcon = ppd->lchIcon;
	ppd2.lpwzTitle = ppd->lpwzContactName;
	ppd2.lpwzText = ppd->lpwzText;
	ppd2.colorBack = ppd->colorBack;
	ppd2.colorText = ppd->colorText;
	ppd2.PluginWindowProc = ppd->PluginWindowProc;
	ppd2.PluginData = ppd->PluginData;
	ppd2.iSeconds = ppd->iSeconds;

	if (lParam & APF_NEWDATA) {
		ppd2.lchNotification = ppd->hNotification;
		ppd2.actionCount = ppd->actionCount;
		ppd2.lpActions = ppd->lpActions;
	}

	return Popup_AddPopup2((WPARAM)&ppd2, lParam);
}

//===== Popup/AddPopup2
static __forceinline DWORD Proto_Status2Flag_My(DWORD status)
{
	if (DWORD res = Proto_Status2Flag(status))
		return res;
	return PF2_IDLE;
}

INT_PTR Popup_AddPopup2(WPARAM wParam, LPARAM lParam)
{
	/* NOTE: we will return 0 instead of -1 since tabSRMM stops using popup after first failure :/ */

	if (!gbPopupLoaded)
		return -1;

	POPUPDATA2 *ppdIn = (POPUPDATA2*)wParam;
	if (!ppdIn)
		return -1;

	if (NotifyEventHooks(hEventNotify, (WPARAM)ppdIn->lchContact, (LPARAM)ppdIn->PluginWindowProc))
		return 0;

	POPUPDATA2 ppdFixed = { 0 };
	POPUPDATA2 *ppd = &ppdFixed;
	memcpy(ppd, ppdIn, min(ppdIn->cbSize, sizeof(POPUPDATA2)));

	DWORD disableWhen;
	FillNotificationData(ppd, &disableWhen);

	if (!(lParam & APF_NO_HISTORY))
		PopupHistoryAdd(ppd);

	if (PopupThreadIsFull())
		return -1;

	if (IsWorkstationLocked())
		return -1;

	// Check if contact handle is valid.
	char *proto = nullptr;
	if (ppd->lchContact)
		proto = GetContactProto(ppd->lchContact);

	BYTE bShowMode = proto ? db_get_b(ppd->lchContact, MODULNAME, "ShowMode", PU_SHOWMODE_AUTO) : PU_SHOWMODE_AUTO;

	if (bShowMode == PU_SHOWMODE_BLOCK)
		return -1;

	if (bShowMode != PU_SHOWMODE_FAVORITE) {
		if (!PopupOptions.ModuleIsEnabled)
			return -1;

		if (PopupOptions.DisableWhenFullscreen && (bShowMode != PU_SHOWMODE_FULLSCREEN) && IsFullScreen())
			return -1;

		if (db_get_dw(NULL, MODULNAME, LPGEN("Global Status"), 0) & Proto_Status2Flag_My(CallService(MS_CLIST_GETSTATUSMODE, 0, 0)))
			return -1;

		if ((disableWhen & 0x0000FFFF) & Proto_Status2Flag_My(CallService(MS_CLIST_GETSTATUSMODE, 0, 0)))
			return -1;

		if (proto) {
			char prefix[128];
			mir_snprintf(prefix, LPGEN("Protocol Status") "/%s", GetContactProto(ppd->lchContact));
			if (db_get_dw(NULL, MODULNAME, prefix, 0) & Proto_Status2Flag_My(CallProtoService(proto, PS_GETSTATUS, 0, 0)))
				return -1;
			if (((disableWhen >> 16) & 0xFFFF0000) & Proto_Status2Flag_My(CallProtoService(proto, PS_GETSTATUS, 0, 0)))
				return -1;
		}
	}

	if (lParam & APF_CUSTOM_POPUP)
		ppd->flags |= PU2_CUSTOM_POPUP;

	PopupWnd2 *wnd = new PopupWnd2(ppd, nullptr, false);
	if (lParam & APF_RETURN_HWND) {
		while (!wnd->m_bWindowCreated) Sleep(1);
		return (INT_PTR)wnd->getHwnd();
	}

	return 0;
}

//===== Popup/GetContact
INT_PTR Popup_GetContact(WPARAM wParam, LPARAM)
{
	if (!gbPopupLoaded) return -1;

	HWND hwnd = (HWND)wParam;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (wnd && IsValidPopupObject(wnd)) return (INT_PTR)wnd->getContactPassed();
	return (INT_PTR)(-1);
}

//===== Popup/GetPluginData
INT_PTR Popup_GetPluginData(WPARAM wParam, LPARAM)
{
	if (!gbPopupLoaded || !wParam)
		return -1;

	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (wnd && IsValidPopupObject(wnd))
		return (INT_PTR)wnd->getData();
	return (INT_PTR)(-1);
}

//===== Popup/ChangeTextW
INT_PTR Popup_ChangeTextW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded || !wParam)
		return -1;

	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || !IsValidPopupObject(wnd))
		return -1;

	wnd->callMethodSync(&PopupWnd2::m_updateText, lParam);
	return 0;
}

//===== Popup/ChangeW
INT_PTR Popup_ChangeW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded || !wParam)
		return -1;

	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || !IsValidPopupObject(wnd))
		return -1;

	wnd->callMethodSync(&PopupWnd2::m_updateData_POPUPDATAW_V2, lParam);
	return 0;
}

//===== Popup/Change2
INT_PTR Popup_Change2(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded) return -1;

	if (!wParam) return -1;
	PopupWnd2 *wnd = (PopupWnd2 *)GetWindowLongPtr((HWND)wParam, GWLP_USERDATA);
	if (!wnd || IsValidPopupObject(wnd)) return -1;
	wnd->callMethodSync(&PopupWnd2::m_updateData_POPUPDATA2, lParam);
	return 0;
}

INT_PTR Popup_ShowMessageW(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded || !wParam || !lParam) return -1;
	if (closing) return 0;

	POPUPDATA2 ppd2 = { 0 };
	ppd2.cbSize = sizeof(ppd2);
	ppd2.flags = PU2_UNICODE;
	ppd2.lptzText = (wchar_t*)wParam;
	switch (lParam & 0x7fffffff) {
	case SM_ERROR:
		ppd2.lchIcon = LoadIconEx(IDI_MB_STOP, 0);
		ppd2.colorBack = RGB(191, 0, 0);
		ppd2.colorText = RGB(255, 245, 225);
		ppd2.lchNotification = g_hntfError;
		ppd2.lptzTitle = TranslateT("Error");
		break;
	case SM_WARNING:
		ppd2.lchIcon = LoadIconEx(IDI_MB_WARN, 0);
		ppd2.colorBack = RGB(210, 210, 150);
		ppd2.colorText = RGB(0, 0, 0);
		ppd2.lchNotification = g_hntfWarning;
		ppd2.lptzTitle = TranslateT("Warning");
		break;
	case SM_NOTIFY:
		ppd2.lchIcon = LoadIconEx(IDI_MB_INFO, 0);
		ppd2.colorBack = RGB(230, 230, 230);
		ppd2.colorText = RGB(0, 0, 0);
		ppd2.lchNotification = g_hntfNotification;
		ppd2.lptzTitle = TranslateT("Notify");
		break;
	default: // No no no... you must give me a good value.
		return -1;
	}
	return Popup_AddPopup2((WPARAM)&ppd2, (lParam & 0x80000000) ? APF_NO_HISTORY : 0);
}

INT_PTR Popup_ShowMessage(WPARAM wParam, LPARAM lParam)
{
	if (!gbPopupLoaded || !wParam || !lParam) return -1;
	if (closing) return 0;

	ptrW wszMsg(mir_a2u((char*)wParam));
	return Popup_ShowMessageW(wszMsg, lParam);
}

//===== Popup/Query
INT_PTR Popup_Query(WPARAM wParam, LPARAM)
{
	if (!gbPopupLoaded) return -1;

	if (closing)
		return 0;

	switch (wParam) {
	case PUQS_ENABLEPOPUPS:
		if (PopupOptions.ModuleIsEnabled)
			return 1; // They're already ON!!!
		
		// Module was disabled.
		svcEnableDisableMenuCommand(0, 0);
		return 0;

	case PUQS_DISABLEPOPUPS:
		if (!(PopupOptions.ModuleIsEnabled))
			return 1; // They're already OFF!!!

		svcEnableDisableMenuCommand(0, 0);
		return 0;

	case PUQS_GETSTATUS:
		return (PopupOptions.ModuleIsEnabled);

	default:
		return -1;
	}
}

//===== Popup/RegisterActions
INT_PTR Popup_RegisterActions(WPARAM wParam, LPARAM lParam)
{
	LPPOPUPACTION actions = (LPPOPUPACTION)wParam;
	for (int i = 0; i < lParam; ++i)
		RegisterAction(&actions[i]);
	return 0;
}


INT_PTR Popup_RegisterNotification(WPARAM wParam, LPARAM)
{
	return (INT_PTR)RegisterNotification((LPPOPUPNOTIFICATION)wParam);
}


//===== Popup/UnhookEventAsync
struct SafeUnhookEventParam
{
	HWND hwndPopup;
	HANDLE hEvent;
};

static void CALLBACK SafeUnhookEventFunc(ULONG_PTR dwParam)
{
	UnhookEvent(((SafeUnhookEventParam *)dwParam)->hEvent);
	PostMessage(((SafeUnhookEventParam *)dwParam)->hwndPopup, UM_POPUPUNHOOKCOMPLETE, 0,
		(LPARAM)((SafeUnhookEventParam *)dwParam)->hEvent);
	delete (SafeUnhookEventParam *)dwParam;
}

INT_PTR Popup_UnhookEventAsync(WPARAM wParam, LPARAM lParam)
{
	SafeUnhookEventParam *param = new SafeUnhookEventParam;
	param->hwndPopup = (HWND)wParam;
	param->hEvent = (HANDLE)lParam;
	QueueUserAPC(SafeUnhookEventFunc, hMainThread, (ULONG_PTR)param);
	return 0;
}

//===== Popup/RegisterVfx (effekt name for drop down box)
INT_PTR Popup_RegisterVfx(WPARAM, LPARAM lParam)
{
	OptAdv_RegisterVfx((char *)lParam);
	return 0;
}

//===== Popup/RegisterClass		(for core class api support)
INT_PTR Popup_RegisterPopupClass(WPARAM, LPARAM lParam)
{
	char setting[256];
	POPUPCLASS *pc = (POPUPCLASS *)lParam;
	POPUPTREEDATA *ptd = (POPUPTREEDATA *)mir_calloc(sizeof(POPUPTREEDATA));
	ptd->cbSize = sizeof(POPUPTREEDATA);
	ptd->signature = 0/*PopupNotificationData_SIGNATURE*/;
	ptd->typ = 2;
	memcpy(&ptd->pupClass, pc, sizeof(POPUPCLASS));
	ptd->pszTreeRoot = mir_a2u(pc->pszName);
	ptd->pupClass.pszName = mir_strdup(pc->pszName);
	if (pc->flags & PCF_UNICODE) {
		ptd->pupClass.pwszDescription = mir_wstrdup(pc->pwszDescription);
		ptd->pszDescription = mir_wstrdup(pc->pwszDescription);
	}
	else {
		ptd->pupClass.pszDescription = mir_strdup(pc->pszDescription);
		ptd->pszDescription = mir_a2u(pc->pszDescription);
	}
	LoadClassSettings(ptd, PU_MODULCLASS);

	// we ignore pc->colorText and use fonts.text as default (if no setting found in DB)
	mir_snprintf(setting, "%s/TextCol", ptd->pupClass.pszName);
	ptd->pupClass.colorText = (COLORREF)db_get_dw(NULL, PU_MODULCLASS, setting, fonts.clText/*pc->colorText*/);
	FontIDW fid = { 0 };
	fid.cbSize = sizeof(FontIDW);
	mir_snwprintf(fid.group, _A2W(PU_FNT_AND_COLOR) L"/%S", ptd->pupClass.pszName);
	mir_strncpy(fid.dbSettingsGroup, PU_MODULCLASS, _countof(fid.dbSettingsGroup) - 1);
	fid.flags = FIDF_DEFAULTVALID;
	fid.deffontsettings.charset = DEFAULT_CHARSET;
	fid.deffontsettings.size = -11;
	mir_wstrncpy(fid.deffontsettings.szFace, L"Verdana", _countof(fid.deffontsettings.szFace) - 1);
	mir_wstrncpy(fid.name, _A2W(PU_FNT_NAME_TEXT), _countof(fid.name) - 1);
	mir_strncpy(fid.prefix, setting, _countof(fid.prefix));
	mir_snprintf(fid.prefix, "%s/Text", ptd->pupClass.pszName);  // result is "%s/TextCol"
	fid.deffontsettings.style = 0;
	fid.deffontsettings.colour = fonts.clText;
	Font_RegisterW(&fid);

	// we ignore pc->colorBack and use fonts.clBack as default (if no setting found in DB)
	mir_snprintf(setting, "%s/BgCol", ptd->pupClass.pszName);
	ptd->pupClass.colorBack = (COLORREF)db_get_dw(NULL, PU_MODULCLASS, setting, (DWORD)fonts.clBack/*pc->colorBack*/);
	ColourIDW cid = { 0 };
	cid.cbSize = sizeof(ColourIDW);
	mir_snwprintf(cid.group, _A2W(PU_FNT_AND_COLOR) L"/%S", ptd->pupClass.pszName);
	mir_strncpy(cid.dbSettingsGroup, PU_MODULCLASS, _countof(fid.dbSettingsGroup));
	mir_wstrncpy(cid.name, PU_COL_BACK_NAME, _countof(cid.name));
	mir_snprintf(cid.setting, "%s/BgCol", ptd->pupClass.pszName);
	cid.defcolour = fonts.clBack;
	Colour_RegisterW(&cid);

	gTreeData.insert(ptd);
	num_classes++;
	return (INT_PTR)ptd;
}

INT_PTR Popup_UnregisterPopupClass(WPARAM, LPARAM lParam)
{
	POPUPTREEDATA *ptd = (POPUPTREEDATA*)lParam;
	if (ptd == nullptr)
		return 1;

	for (int i = 0; i < gTreeData.getCount(); i++)
		if (gTreeData[i] == ptd) {
			gTreeData.remove(i);
			FreePopupClass(ptd);
			return 0;
		}

	return 1;
}

//===== Popup/AddPopupClass		(for core class api support)
INT_PTR Popup_CreateClassPopup(WPARAM wParam, LPARAM lParam)
{
	POPUPDATACLASS *pdc = (POPUPDATACLASS *)lParam;
	if (!pdc || (pdc->cbSize != sizeof(POPUPDATACLASS)))
		return 1;

	POPUPCLASS *pc;
	if (wParam)
		pc = (POPUPCLASS*)wParam;
	else {
		LPTSTR group = mir_a2u(pdc->pszClassName);
		POPUPTREEDATA *ptd = (POPUPTREEDATA *)FindTreeData(group, nullptr, 2);
		if (ptd)
			pc = &ptd->pupClass;
		else
			pc = nullptr;
		mir_free(group);
	}
	if (pc == nullptr)
		return 1;

	POPUPDATA2 ppd2 = { sizeof(ppd2) };
	ppd2.colorBack = pc->colorBack;
	ppd2.colorText = pc->colorText;
	ppd2.lchIcon = pc->hIcon;
	ppd2.iSeconds = pc->iSeconds;
	ppd2.PluginWindowProc = pc->PluginWindowProc;
	if (pc->flags & PCF_UNICODE) {
		ppd2.flags = PU2_UNICODE;
		ppd2.lptzTitle = (wchar_t*)pdc->pwszTitle;
		ppd2.lptzText = (wchar_t*)pdc->pwszText;
	}
	else {
		ppd2.flags = PU2_ANSI;
		ppd2.lpzTitle = (char *)pdc->pszTitle;
		ppd2.lpzText = (char *)pdc->pszText;
	}
	ppd2.lchContact = pdc->hContact;
	ppd2.PluginData = pdc->PluginData;

	return Popup_AddPopup2((WPARAM)&ppd2, pc->lParam);
}

INT_PTR Popup_DeletePopup(WPARAM, LPARAM lParam)
{
	return (INT_PTR)SendMessage((HWND)lParam, UM_DESTROYPOPUP, 0, 0);
}

INT_PTR Popup_LoadSkin(WPARAM, LPARAM lParam)
{
	if (lParam)
	{
		mir_free(PopupOptions.SkinPack);
		PopupOptions.SkinPack = mir_a2u((char*)lParam);
	}

	const PopupSkin *skin = nullptr;
	if (skin = skins.getSkin(PopupOptions.SkinPack)) {
		mir_free(PopupOptions.SkinPack);
		PopupOptions.SkinPack = mir_wstrdup(skin->getName());
	}

	return 1;
}