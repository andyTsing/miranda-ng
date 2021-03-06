#include "stdafx.h"

CSteamPasswordEditor::CSteamPasswordEditor(CSteamProto *proto)
	: CSteamDlgBase(proto, IDD_PASSWORD_EDITOR, false), m_ok(this, IDOK),
	m_password(this, IDC_PASSWORD), m_savePermanently(this, IDC_SAVEPERMANENTLY)
{
	m_ok.OnClick = Callback(this, &CSteamPasswordEditor::OnOk);
}

void CSteamPasswordEditor::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "main");
	Window_SetIcon_IcoLib(m_hwnd, IcoLib_GetIconHandle(iconName));

	SendMessage(m_password.GetHwnd(), EM_LIMITTEXT, 64, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "PasswordWindow");
}

void CSteamPasswordEditor::OnOk(CCtrlButton*)
{
	if (m_proto->password != nullptr)
		mir_free(m_proto->password);
	m_proto->password = m_password.GetText();
	if (m_savePermanently.Enabled())
		m_proto->setWString("Password", m_proto->password);

	EndDialog(m_hwnd, DIALOG_RESULT_OK);
}

void CSteamPasswordEditor::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "PasswordWindow");
}

/////////////////////////////////////////////////////////////////////////////////

CSteamGuardDialog::CSteamGuardDialog(CSteamProto *proto, const char *domain)
	: CSteamDlgBase(proto, IDD_GUARD, false),
	m_ok(this, IDOK),
	m_text(this, IDC_TEXT),
	m_link(this, IDC_GETDOMAIN, domain)
{
	memset(m_guardCode, 0, sizeof(m_guardCode));
	mir_strcpy(m_domain, domain);
	m_ok.OnClick = Callback(this, &CSteamGuardDialog::OnOk);
}

void CSteamGuardDialog::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "main");
	Window_SetIcon_IcoLib(m_hwnd, IcoLib_GetIconHandle(iconName));

	SendMessage(m_text.GetHwnd(), EM_LIMITTEXT, 5, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "GuardWindow");
}

void CSteamGuardDialog::OnOk(CCtrlButton*)
{
	mir_strncpy(m_guardCode, ptrA(m_text.GetTextA()), _countof(m_guardCode));
	EndDialog(m_hwnd, DIALOG_RESULT_OK);
}

void CSteamGuardDialog::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "GuardWindow");
}

const char* CSteamGuardDialog::GetGuardCode()
{
	return m_guardCode;
}

/////////////////////////////////////////////////////////////////////////////////

CSteamTwoFactorDialog::CSteamTwoFactorDialog(CSteamProto *proto)
: CSteamDlgBase(proto, IDD_TWOFACTOR, false),
m_ok(this, IDOK),
m_text(this, IDC_TEXT)
{
	memset(m_twoFactorCode, 0, sizeof(m_twoFactorCode));
	m_ok.OnClick = Callback(this, &CSteamTwoFactorDialog::OnOk);
}

void CSteamTwoFactorDialog::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "main");
	Window_SetIcon_IcoLib(m_hwnd, IcoLib_GetIconHandle(iconName));

	SendMessage(m_text.GetHwnd(), EM_LIMITTEXT, 5, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "TwoFactorWindow");
}

void CSteamTwoFactorDialog::OnOk(CCtrlButton*)
{
	mir_strncpy(m_twoFactorCode, ptrA(m_text.GetTextA()), _countof(m_twoFactorCode));
	EndDialog(m_hwnd, DIALOG_RESULT_OK);
}

void CSteamTwoFactorDialog::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "TwoFactorWindow");
}

const char* CSteamTwoFactorDialog::GetTwoFactorCode()
{
	return m_twoFactorCode;
}

/////////////////////////////////////////////////////////////////////////////////

CSteamCaptchaDialog::CSteamCaptchaDialog(CSteamProto *proto, BYTE *captchaImage, int captchaImageSize)
	: CSteamDlgBase(proto, IDD_CAPTCHA, false),
	m_ok(this, IDOK), m_text(this, IDC_TEXT),
	m_captchaImage(nullptr)
{
	memset(m_captchaText, 0, sizeof(m_captchaText));
	m_captchaImageSize = captchaImageSize;
	m_captchaImage = (BYTE*)mir_alloc(captchaImageSize);
	memcpy(m_captchaImage, captchaImage, captchaImageSize);
	m_ok.OnClick = Callback(this, &CSteamCaptchaDialog::OnOk);
}

CSteamCaptchaDialog::~CSteamCaptchaDialog()
{
	if(m_captchaImage)
		mir_free(m_captchaImage);
}

void CSteamCaptchaDialog::OnInitDialog()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "main");
	Window_SetIcon_IcoLib(m_hwnd, IcoLib_GetIconHandle(iconName));

	SendMessage(m_text.GetHwnd(), EM_LIMITTEXT, 6, 0);

	Utils_RestoreWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "CaptchaWindow");
}

void CSteamCaptchaDialog::OnOk(CCtrlButton*)
{
	mir_strncpy(m_captchaText, ptrA(m_text.GetTextA()), _countof(m_captchaText));
	EndDialog(m_hwnd, DIALOG_RESULT_OK);
}

void CSteamCaptchaDialog::OnClose()
{
	Utils_SaveWindowPosition(m_hwnd, NULL, m_proto->m_szModuleName, "CaptchaWindow");
}

INT_PTR CSteamCaptchaDialog::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_PAINT)
	{
		FI_INTERFACE *fei = nullptr;

		INT_PTR result = CALLSERVICE_NOTFOUND;
		if (ServiceExists(MS_IMG_GETINTERFACE))
			result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&fei);

		if (fei == nullptr || result != S_OK) {
			MessageBox(nullptr, TranslateT("Fatal error, image services not found. Avatar services will be disabled."), TranslateT("Avatar Service"), MB_OK);
			return 0;
		}

		FIMEMORY *stream = fei->FI_OpenMemory(m_captchaImage, m_captchaImageSize);
		FREE_IMAGE_FORMAT fif = fei->FI_GetFileTypeFromMemory(stream, 0);
		FIBITMAP *bitmap = fei->FI_LoadFromMemory(fif, stream, 0);
		fei->FI_CloseMemory(stream);

		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(m_hwnd, &ps);

		//SetStretchBltMode(hDC, COLORONCOLOR);
		StretchDIBits(
			hDC,
			11, 11,
			fei->FI_GetWidth(bitmap) - 13,
			fei->FI_GetHeight(bitmap),
			0, 0,
			fei->FI_GetWidth(bitmap),
			fei->FI_GetHeight(bitmap),
			fei->FI_GetBits(bitmap),
			fei->FI_GetInfo(bitmap),
			DIB_RGB_COLORS, SRCCOPY);

		fei->FI_Unload(bitmap);
		//fei->FI_DeInitialise();

		EndPaint(m_hwnd, &ps);

		return FALSE;
	}
	CSteamDlgBase::DlgProc(msg, wParam, lParam);
	return FALSE;
}

const char* CSteamCaptchaDialog::GetCaptchaText()
{
	return m_captchaText;
}