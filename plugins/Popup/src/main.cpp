/*
Popup Plus plugin for Miranda IM

Copyright	© 2002 Luca Santarelli,
© 2004-2007 Victor Pavlychko
© 2010 MPK

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

WORD SETTING_MAXIMUMWIDTH_MAX = GetSystemMetrics(SM_CXSCREEN);

#define MENUCOMMAND_HISTORY "Popup/ShowHistory"
#define MENUCOMMAND_SVC "Popup/EnableDisableMenuCommand"

HANDLE hEventNotify;

//===== Options =========================================================================
static int OptionsInitialize(WPARAM, LPARAM);
void UpgradeDb();

//===== Initializations =================================================================
static int OkToExit(WPARAM, LPARAM);
bool OptionLoaded = false;
int hLangpack = 0;
CLIST_INTERFACE *pcli;

//===== Global variables ================================================================
HMODULE  hUserDll = nullptr;
HMODULE  hMsimgDll = nullptr;
HMODULE  hKernelDll = nullptr;
HMODULE  hGdiDll = nullptr;
HMODULE  hDwmapiDll = nullptr;

GLOBAL_WND_CLASSES g_wndClass = { 0 };

HANDLE   htuText;
HANDLE   htuTitle;

HGENMENU hMenuRoot;
HGENMENU hMenuItem;
HGENMENU hMenuItemHistory;

HANDLE   hTTButton;

//===== Options pages ===================================================================

static int OptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = MODULNAME_PLU;

	odp.szTab.a = LPGEN("General");
	odp.pfnDlgProc = DlgProcPopupGeneral;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP_GENERAL);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Classes");
	odp.pfnDlgProc = DlgProcOptsClasses;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_NOTIFICATIONS);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Actions");
	odp.pfnDlgProc = DlgProcPopupActions;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_ACTIONS);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Contacts");
	odp.pfnDlgProc = DlgProcContactOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONTACTS);
	Options_AddPage(wParam, &odp);

	odp.szTab.a = LPGEN("Advanced");
	odp.pfnDlgProc = DlgProcPopupAdvOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP_ADVANCED);
	Options_AddPage(wParam, &odp);

	odp.szGroup.a = LPGEN("Skins");
	odp.szTab.a = LPGEN(MODULNAME_PLU);
	odp.pfnDlgProc = DlgProcPopSkinsOpts;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_SKIN2);
	Options_AddPage(wParam, &odp);
	return 0;
}

static int FontsChanged(WPARAM, LPARAM)
{
	ReloadFonts();
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
	LoadActions();

	HANDLE hIcon = PopupOptions.ModuleIsEnabled == TRUE
		? GetIconHandle(IDI_POPUP)
		: GetIconHandle(IDI_NOPOPUP);

	Menu_ModifyItem(hMenuItem, nullptr, hIcon);
	Menu_ModifyItem(hMenuRoot, nullptr, hIcon);

	Menu_ModifyItem(hMenuItemHistory, nullptr, GetIconHandle(IDI_HISTORY));
	return 0;
}

static int TTBLoaded(WPARAM, LPARAM)
{
	TTBButton ttb = {};
	ttb.pszService = MENUCOMMAND_SVC;
	ttb.lParamUp = 1;
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON;
	if (PopupOptions.ModuleIsEnabled)
		ttb.dwFlags |= TTBBF_PUSHED;
	ttb.name = LPGEN("Toggle Popups");
	ttb.hIconHandleUp = GetIconHandle(IDI_NOPOPUP);
	ttb.hIconHandleDn = GetIconHandle(IDI_POPUP);
	ttb.pszTooltipUp = LPGEN("Enable Popups");
	ttb.pszTooltipDn = LPGEN("Disable Popups");
	hTTButton = TopToolbar_AddButton(&ttb);
	return 0;
}

//===== EnableDisableMenuCommand ========================================================
INT_PTR svcEnableDisableMenuCommand(WPARAM, LPARAM)
{
	HANDLE hIcon;
	if (PopupOptions.ModuleIsEnabled) {
		// The module is enabled.
		// The action to do is "disable popups" (show disabled) and we must write "enable popup" in the new item.
		PopupOptions.ModuleIsEnabled = FALSE;
		db_set_b(NULL, "Popup", "ModuleIsEnabled", FALSE);
		Menu_ModifyItem(hMenuItem, LPGENW("Enable Popups"), hIcon = GetIconHandle(IDI_NOPOPUP));
	}
	else {
		// The module is disabled.
		// The action to do is enable popups (show enabled), then write "disable popup" in the new item.
		PopupOptions.ModuleIsEnabled = TRUE;
		db_set_b(NULL, "Popup", "ModuleIsEnabled", TRUE);
		Menu_ModifyItem(hMenuItem, LPGENW("Disable Popups"), hIcon = GetIconHandle(IDI_POPUP));
	}

	Menu_ModifyItem(hMenuRoot, nullptr, hIcon);

	if (hTTButton)
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)hTTButton, (PopupOptions.ModuleIsEnabled) ? TTBST_PUSHED : 0);

	return 0;
}

INT_PTR svcShowHistory(WPARAM, LPARAM)
{
	PopupHistoryShow();
	return 0;
}

void InitMenuItems(void)
{
	CMenuItem mi;
	mi.flags = CMIF_UNICODE;

	HANDLE hIcon = GetIconHandle(PopupOptions.ModuleIsEnabled ? IDI_POPUP : IDI_NOPOPUP);

	// Build main menu
	hMenuRoot = mi.root = Menu_CreateRoot(MO_MAIN, MODULNAME_PLUW, -1000000000, hIcon);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "3F5B5AB5-46D8-469E-8951-50B287C59037");

	// Add item to main menu
	SET_UID(mi, 0x4353d44e, 0x177, 0x4843, 0x88, 0x30, 0x25, 0x5d, 0x91, 0xad, 0xdf, 0x3f);
	mi.pszService = MENUCOMMAND_SVC;
	CreateServiceFunction(mi.pszService, svcEnableDisableMenuCommand);
	mi.name.w = PopupOptions.ModuleIsEnabled ? LPGENW("Disable Popups") : LPGENW("Enable Popups");
	mi.hIcolibItem = hIcon;
	hMenuItem = Menu_AddMainMenuItem(&mi);

	// Popup History
	SET_UID(mi, 0x92c386ae, 0x6e81, 0x452d, 0xb5, 0x71, 0x87, 0x46, 0xe9, 0x2, 0x66, 0xe9);
	mi.pszService = MENUCOMMAND_HISTORY;
	CreateServiceFunction(mi.pszService, svcShowHistory);
	mi.position = 1000000000;
	mi.name.w = LPGENW("Popup History");
	mi.hIcolibItem = GetIconHandle(IDI_HISTORY);
	hMenuItemHistory = Menu_AddMainMenuItem(&mi);
}

//===== GetStatus =======================================================================
INT_PTR GetStatus(WPARAM, LPARAM)
{
	return PopupOptions.ModuleIsEnabled;
}

// register Hotkey
void LoadHotkey()
{
	HOTKEYDESC hk = {};
	hk.dwFlags = HKD_UNICODE;
	hk.pszName = "Toggle Popups";
	hk.szDescription.w = LPGENW("Toggle Popups");
	hk.szSection.w = MODULNAME_PLUW;
	hk.pszService = MENUCOMMAND_SVC;
	Hotkey_Register(&hk);

	// 'Popup History' Hotkey
	hk.pszName = "Popup History";
	hk.szDescription.w = LPGENW("Popup History");
	hk.pszService = MENUCOMMAND_HISTORY;
	Hotkey_Register(&hk);
}

// menu
// Function which makes the initializations
static int ModulesLoaded(WPARAM, LPARAM)
{
	// check if History++ is installed
	gbHppInstalled = ServiceExists(MS_HPP_GETVERSION) && ServiceExists(MS_HPP_EG_WINDOW) &&
		(CallService(MS_HPP_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(1, 5, 0, 112));

	// check if MText plugin is installed
	mir_getMTI(&MText);
	if (MText.Register) {
		htuText = MText.Register("Popup Plus/Text", MTEXT_FANCY_DEFAULT);
		htuTitle = MText.Register("Popup Plus/Title", MTEXT_FANCY_DEFAULT);
	}
	else htuTitle = htuText = nullptr;

	// check if OptionLoaded
	if (!OptionLoaded)
		LoadOptions();

	// Uninstalling purposes
	if (ServiceExists("PluginSweeper/Add"))
		CallService("PluginSweeper/Add", (WPARAM)Translate(MODULNAME), (LPARAM)MODULNAME);

	// load fonts / create hook
	InitFonts();
	HookEvent(ME_FONT_RELOAD, FontsChanged);

	// load actions and notifications
	LoadActions();
	LoadNotifications();
	// hook TopToolBar
	HookEvent(ME_TTB_MODULELOADED, TTBLoaded);
	// Folder plugin support
	folderId = FoldersRegisterCustomPathT(LPGEN("Skins"), LPGEN("Popup Plus"), MIRANDA_PATHT L"\\Skins\\Popup");
	// load skin
	skins.load();
	const PopupSkin *skin;
	if (skin = skins.getSkin(PopupOptions.SkinPack)) {
		mir_free(PopupOptions.SkinPack);
		PopupOptions.SkinPack = mir_wstrdup(skin->getName());
		skin->loadOpts();
	}
	// init PopupEfects
	PopupEfectsInitialize();
	// MessageAPI support
	SrmmMenu_Load();
	// Hotkey
	LoadHotkey();

	gbPopupLoaded = TRUE;
	return 0;
}

//=== DllMain ===========================================================================
// DLL entry point, Required to store the instance handle
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

//===== MirandaPluginInfo ===============================================================
// Called by Miranda to get the information associated to this plugin.
// It only returns the PLUGININFOEX structure, without any test on the version
// @param mirandaVersion - The version of the application calling this function
MIRAPI PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

// called before the app goes into shutdown routine to make sure everyone is happy to exit
static int OkToExit(WPARAM, LPARAM)
{
	closing = TRUE;
	StopPopupThread();
	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	UnloadPopupThread();
	UnloadPopupWnd2();
	return 0;
}

//===== Load ============================================================================
// Initializes the services provided and the link to those needed
// Called when the plugin is loaded into Miranda
MIRAPI int Load(void)
{
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0);

	mir_getLP(&pluginInfoEx);
	pcli = Clist_GetInterface();

	CreateServiceFunction(MS_POPUP_GETSTATUS, GetStatus);

#if defined(_DEBUG)
	PopupOptions.debug = db_get_b(NULL, MODULNAME, "debug", FALSE);
#else
	PopupOptions.debug = false;
#endif
	LoadGDIPlus();

	// Transparent and animation routines
	hDwmapiDll = LoadLibrary(L"dwmapi.dll");
	MyDwmEnableBlurBehindWindow = nullptr;
	if (hDwmapiDll)
		MyDwmEnableBlurBehindWindow = (HRESULT(WINAPI *)(HWND, DWM_BLURBEHIND *))GetProcAddress(hDwmapiDll, "DwmEnableBlurBehindWindow");

	PopupHistoryLoad();
	LoadPopupThread();
	if (!LoadPopupWnd2()) {
		MessageBox(nullptr, TranslateT("Error: I could not register the Popup Window class.\r\nThe plugin will not operate."), MODULNAME_LONG, MB_ICONSTOP | MB_OK);
		return 0; // We couldn't register our Window Class, don't hook any event: the plugin will act as if it was disabled.
	}
	RegisterOptPrevBox();

	// Register in DBEditor++
	UpgradeDb();

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OkToExit);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);

	hbmNoAvatar = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_NOAVATAR));

	if (!OptionLoaded)
		LoadOptions();

	// Service Functions
	hEventNotify = CreateHookableEvent(ME_POPUP_FILTER);

	CreateServiceFunction(MS_POPUP_ADDPOPUP, Popup_AddPopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUPW, Popup_AddPopupW);
	CreateServiceFunction(MS_POPUP_ADDPOPUP2, Popup_AddPopup2);

	CreateServiceFunction(MS_POPUP_CHANGETEXTW, Popup_ChangeTextW);

	CreateServiceFunction(MS_POPUP_CHANGEW, Popup_ChangeW);
	CreateServiceFunction(MS_POPUP_CHANGEPOPUP2, Popup_Change2);

	CreateServiceFunction(MS_POPUP_GETCONTACT, Popup_GetContact);
	CreateServiceFunction(MS_POPUP_GETPLUGINDATA, Popup_GetPluginData);

	CreateServiceFunction(MS_POPUP_SHOWMESSAGE, Popup_ShowMessage);
	CreateServiceFunction(MS_POPUP_SHOWMESSAGEW, Popup_ShowMessageW);
	CreateServiceFunction(MS_POPUP_QUERY, Popup_Query);

	CreateServiceFunction(MS_POPUP_REGISTERACTIONS, Popup_RegisterActions);
	CreateServiceFunction(MS_POPUP_REGISTERNOTIFICATION, Popup_RegisterNotification);

	CreateServiceFunction(MS_POPUP_UNHOOKEVENTASYNC, Popup_UnhookEventAsync);

	CreateServiceFunction(MS_POPUP_REGISTERVFX, Popup_RegisterVfx);

	CreateServiceFunction(MS_POPUP_REGISTERCLASS, Popup_RegisterPopupClass);
	CreateServiceFunction(MS_POPUP_UNREGISTERCLASS, Popup_UnregisterPopupClass);
	CreateServiceFunction(MS_POPUP_ADDPOPUPCLASS, Popup_CreateClassPopup);

	CreateServiceFunction(MS_POPUP_DESTROYPOPUP, Popup_DeletePopup);

	CreateServiceFunction("Popup/LoadSkin", Popup_LoadSkin);

	// load icons / create hook
	InitIcons();
	HookEvent(ME_SKIN2_ICONSCHANGED, IconsChanged);
	// add menu items
	InitMenuItems();

	return 0;
}

//===== Unload ==========================================================================
// Prepare the plugin to stop
// Called by Miranda when it will exit or when the plugin gets deselected

MIRAPI int Unload(void)
{
	DeleteObject(fonts.title);
	DeleteObject(fonts.clock);
	DeleteObject(fonts.text);
	DeleteObject(fonts.action);
	DeleteObject(fonts.actionHover);

	DeleteObject(hbmNoAvatar);

	FreeLibrary(hDwmapiDll);
	FreeLibrary(hUserDll);
	FreeLibrary(hMsimgDll);
	FreeLibrary(hGdiDll);

	DestroyHookableEvent(hEventNotify);

	mir_free(PopupOptions.SkinPack);
	mir_free(PopupOptions.Effect);

	OptAdv_UnregisterVfx();
	PopupHistoryUnload();
	SrmmMenu_Unload();

	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupWnd2), hInst);
	UnregisterClass(L"PopupEditBox", hInst);
	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupMenuHostWnd), hInst);
	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupThreadManagerWnd), hInst);
	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupPreviewBoxWndclass), hInst);
	UnregisterClass(MAKEINTATOM(g_wndClass.cPopupPlusDlgBox), hInst);

	UnloadGDIPlus();

	UnloadActions();
	UnloadTreeData();

	CloseHandle(hMainThread);
	return 0;
}
