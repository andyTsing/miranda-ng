/*
    Helper functions for Miranda-IM (www.miranda-im.org)
    Copyright 2006 P. Boon

    This program is mir_free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "commonheaders.h"
#include "gen_helpers.h"

int ProtoServiceExists(const char *szModule, const char *szService) {

	char str[MAXMODULELABELLENGTH];
	mir_snprintf(str, sizeof(str), "%s%s", szModule, szService);

	return ServiceExists(str);
}

char *Hlp_GetProtocolNameA(const char *proto) {

	char protoname[256];
	if ((!ProtoServiceExists(proto, PS_GETNAME)) || (CallProtoService(proto, PS_GETNAME, (WPARAM)sizeof(protoname), (LPARAM)protoname)))
		return NULL;

	return mir_strdup(protoname);
}

TCHAR *Hlp_GetProtocolName(const char *proto) {

	char protoname[256];
	if ((!ProtoServiceExists(proto, PS_GETNAME)) || (CallProtoService(proto, PS_GETNAME, (WPARAM)sizeof(protoname), (LPARAM)protoname)))
		return NULL;

	return mir_a2t(protoname);

}

char *Hlp_GetDlgItemTextA(HWND hwndDlg, int nIDDlgItem) {

	int len = SendDlgItemMessageA(hwndDlg, nIDDlgItem, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return NULL;

	char *res = ( char* )mir_alloc((len+1));
	ZeroMemory(res, (len+1));
	GetDlgItemTextA(hwndDlg, nIDDlgItem, res, len+1);

	return res;
}

TCHAR *Hlp_GetDlgItemText(HWND hwndDlg, int nIDDlgItem) {

	int len = SendDlgItemMessage(hwndDlg, nIDDlgItem, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return NULL;

	TCHAR *res = (TCHAR*)mir_alloc((len+1)*sizeof(TCHAR));
	ZeroMemory(res, (len+1)*sizeof(TCHAR));
	GetDlgItemText(hwndDlg, nIDDlgItem, res, len+1);

	return res;
}

char *Hlp_GetWindowTextA(HWND hwndDlg)
{
	int len = SendMessageA(hwndDlg, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return NULL;

	char *res = ( char* )mir_alloc((len+1));
	ZeroMemory(res, (len+1));
	GetWindowTextA(hwndDlg, res, len+1);

	return res;
}

TCHAR *Hlp_GetWindowText(HWND hwndDlg)
{
	int len = SendMessage(hwndDlg, WM_GETTEXTLENGTH, 0, 0);
	if (len < 0)
		return NULL;

	TCHAR *res = (TCHAR*)mir_alloc((len+1)*sizeof(TCHAR));
	ZeroMemory(res, (len+1)*sizeof(TCHAR));
	GetWindowText(hwndDlg, res, len+1);

	return res;
}

/**
 * Modified from Miranda CList, clistsettings.c
 **/

// Logging
static int WriteToDebugLogA(const char *szMsg) {

	int res = 0;
	if (ServiceExists(MS_NETLIB_LOG))
		res = CallService(MS_NETLIB_LOG, (WPARAM)NULL, (LPARAM)szMsg);
	else {
		OutputDebugStringA(szMsg);
		OutputDebugStringA("\r\n");
	}

	return res;
}

int AddDebugLogMessageA(const char* fmt, ...)
{
	int res;
	char szText[MAX_DEBUG], szFinal[MAX_DEBUG];
	va_list va;

	va_start(va,fmt);
	_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);
#ifdef MODULENAME
	mir_snprintf(szFinal, sizeof(szFinal), "%s: %s", MODULENAME, szText);
#else
	strncpy(szFinal, szText, sizeof(szFinal));
#endif
	res = WriteToDebugLogA(szFinal);

	return res;
}

int AddDebugLogMessage(const TCHAR* fmt, ...) {

	int res;
	TCHAR tszText[MAX_DEBUG], tszFinal[MAX_DEBUG];
	char *szFinal;
	va_list va;

	va_start(va,fmt);
	_vsntprintf(tszText, sizeof(tszText), fmt, va);
	va_end(va);
#ifdef MODULENAME
	mir_sntprintf(tszFinal, SIZEOF(tszFinal), _T("%s: %s"), MODULENAME, tszText);
#else
	_tcsncpy(tszFinal, tszText, sizeof(tszFinal));
#endif


	szFinal = mir_t2a(tszFinal);

	res = WriteToDebugLogA(szFinal);
	mir_free(szFinal);

	return res;
}

int AddErrorLogMessageA(const char* fmt, ...) {

	int res;
	char szText[MAX_DEBUG], szFinal[MAX_DEBUG];
	va_list va;

	va_start(va,fmt);
	_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);
#ifdef MODULENAME
	mir_snprintf(szFinal, sizeof(szFinal), "%s: %s", MODULENAME, szText);
#else
	strncpy(szFinal, szText, sizeof(szFinal));
#endif
	res = WriteToDebugLogA(szFinal);
	MessageBoxA(NULL, szFinal, "Error", MB_OK|MB_ICONERROR);

	return res;
}

int AddErrorLogMessage(const TCHAR* fmt, ...) {

	int res;
	TCHAR tszText[MAX_DEBUG], tszFinal[MAX_DEBUG];
	char *szFinal;
	va_list va;

	va_start(va,fmt);
	_vsntprintf(tszText, sizeof(tszText), fmt, va);
	va_end(va);
#ifdef MODULENAME
	mir_sntprintf(tszFinal, SIZEOF(tszFinal), _T("%s: %s"), MODULENAME, tszText);
#else
	_tcsncpy(tszFinal, tszText, sizeof(tszFinal));
#endif


	szFinal = mir_t2a(tszFinal);

	res = WriteToDebugLogA(szFinal);
	MessageBoxA(NULL, szFinal, "Error", MB_OK|MB_ICONERROR);
	mir_free(szFinal);

	return res;
}


//
int ttoi(TCHAR *string)
{
	return ( string == NULL) ? 0 : _ttoi( string );
}

TCHAR *itot(int num) {

	char tRes[32];

	// check this
	_itoa(num, tRes, 10);


	return mir_a2t(tRes);

}

// Helper functions that need MODULENAME
#define SETTING_NOENCODINGCHECK		"NoEncodingCheck"

int Hlp_UnicodeCheck(char *szPluginName, BOOL bForce, const char *szModule)
{
	return 0;
}
