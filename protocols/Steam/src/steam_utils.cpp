﻿#include "stdafx.h"

#pragma comment(lib, "crypt32.lib")

WORD CSteamProto::SteamToMirandaStatus(int state)
{
	switch (state)
	{
	case 0: //Offline
		return ID_STATUS_OFFLINE;
	case 1: //Online
		return ID_STATUS_ONLINE;
	case 2: //Busy
		return ID_STATUS_DND;
	case 3: //Away
		return ID_STATUS_AWAY;
	case 4: //Snoozing
		return ID_STATUS_NA;
	case 5: //Looking to trade
		return ID_STATUS_OUTTOLUNCH;
	case 6: //Looking to play
		return ID_STATUS_FREECHAT;
	default:
		return ID_STATUS_ONLINE;
	}
}

int CSteamProto::MirandaToSteamState(int status)
{
	switch (status)
	{
	case ID_STATUS_OFFLINE:
		return 0; //Offline
	case ID_STATUS_ONLINE:
		return 1; //Online
	case ID_STATUS_DND:
		return 2; //Busy
	case ID_STATUS_AWAY:
		return 3; //Away
	case ID_STATUS_NA:
		return 4; //Snoozing
	case ID_STATUS_OUTTOLUNCH:
		return 5; //Looking to trade
	case ID_STATUS_FREECHAT:
		return 6; //Looking to play
	default:
		return 1;
	}
}

int CSteamProto::RsaEncrypt(const char *pszModulus, DWORD &exponent, const char *data, BYTE *encryptedData, DWORD &encryptedSize)
{
	DWORD cchModulus = (DWORD)mir_strlen(pszModulus);
	int result = 0;
	BYTE *pbBuffer = nullptr;
	BYTE *pKeyBlob = nullptr;
	HCRYPTKEY phKey = 0;
	HCRYPTPROV hCSP = 0;

	// convert hex string to byte array
	DWORD cbLen = 0, dwSkip = 0, dwFlags = 0;
	if (!CryptStringToBinaryA(pszModulus, cchModulus, CRYPT_STRING_HEX, nullptr, &cbLen, &dwSkip, &dwFlags))
	{
		result = GetLastError();
		goto exit;
	}

	// allocate a new buffer.
	pbBuffer = (BYTE*)malloc(cbLen);
	if (!CryptStringToBinaryA(pszModulus, cchModulus, CRYPT_STRING_HEX, pbBuffer, &cbLen, &dwSkip, &dwFlags))
	{
		result = GetLastError();
		goto exit;
	}

	// reverse byte array, because of microsoft
	for (int i = 0; i < (int)(cbLen / 2); ++i)
	{
		BYTE temp = pbBuffer[cbLen - i - 1];
		pbBuffer[cbLen - i - 1] = pbBuffer[i];
		pbBuffer[i] = temp;
	}
	
	if (!CryptAcquireContext(&hCSP, nullptr, nullptr, PROV_RSA_AES, CRYPT_SILENT) &&
		!CryptAcquireContext(&hCSP, nullptr, nullptr, PROV_RSA_AES, CRYPT_SILENT | CRYPT_NEWKEYSET))
	{
		result = GetLastError();
		goto exit;
	}

	// Move the key into the key container.
	DWORD cbKeyBlob = sizeof(PUBLICKEYSTRUC) + sizeof(RSAPUBKEY) + cbLen;
	pKeyBlob = (BYTE*)malloc(cbKeyBlob);

	// Fill in the data.
	PUBLICKEYSTRUC *pPublicKey = (PUBLICKEYSTRUC*)pKeyBlob;
	pPublicKey->bType = PUBLICKEYBLOB; 
	pPublicKey->bVersion = CUR_BLOB_VERSION;  // Always use this value.
	pPublicKey->reserved = 0;                 // Must be zero.
	pPublicKey->aiKeyAlg = CALG_RSA_KEYX;     // RSA public-key key exchange. 

	// The next block of data is the RSAPUBKEY structure.
	RSAPUBKEY *pRsaPubKey = (RSAPUBKEY*)(pKeyBlob + sizeof(PUBLICKEYSTRUC));
	pRsaPubKey->magic = 0x31415352; // RSA1 // Use public key
	pRsaPubKey->bitlen = cbLen * 8;  // Number of bits in the modulus.
	pRsaPubKey->pubexp = exponent; // Exponent.

	// Copy the modulus into the blob. Put the modulus directly after the
	// RSAPUBKEY structure in the blob.
	BYTE *pKey = (BYTE*)(((BYTE *)pRsaPubKey) + sizeof(RSAPUBKEY));
	//pKeyBlob + sizeof(BLOBHEADER)+ sizeof(RSAPUBKEY); 
	memcpy(pKey, pbBuffer, cbLen);

	// Now import public key	
	if (!CryptImportKey(hCSP, pKeyBlob, cbKeyBlob, 0, 0, &phKey))
	{
		result = GetLastError();
		goto exit;
	}

	DWORD dataSize = (DWORD)mir_strlen(data);

	// if data is not allocated just renurn size
	if (encryptedData == nullptr)
	{
		// get length of encrypted data
		if (!CryptEncrypt(phKey, 0, TRUE, 0, nullptr, &encryptedSize, dataSize))
			result = GetLastError();
		goto exit;
	}

	// encrypt password
	memcpy(encryptedData, data, dataSize);
	if (!CryptEncrypt(phKey, 0, TRUE, 0, encryptedData, &dataSize, encryptedSize))
	{
		result = GetLastError();
		goto exit;
	}

	// reverse byte array again
	for (int i = 0; i < (int)(encryptedSize / 2); ++i)
	{
		BYTE temp = encryptedData[encryptedSize - i - 1];
		encryptedData[encryptedSize - i - 1] = encryptedData[i];
		encryptedData[i] = temp;
	}

exit:
	if (pKeyBlob)
		free(pKeyBlob);
	if (phKey)
		CryptDestroyKey(phKey);
	
	if (pbBuffer)
		free(pbBuffer);
	if (hCSP)
		CryptReleaseContext(hCSP, 0);

	return 0;
}

void CSteamProto::ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact)
{
	if (Miranda_IsTerminated())
		return;

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = IcoLib_GetIcon(MODULE"_main");

		if (!PUAddPopupT(&ppd))
			return;
	}

	MessageBox(nullptr, message, caption, MB_OK | flags);
}

void CSteamProto::ShowNotification(const wchar_t *message, int flags, MCONTACT hContact)
{
	ShowNotification(_A2W(MODULE), message, flags, hContact);
}

INT_PTR __cdecl CSteamProto::OnGetEventTextChatStates(WPARAM pEvent, LPARAM datatype)
{
	// Retrieves a chat state description from an event

	DBEVENTINFO *dbei = (DBEVENTINFO *)pEvent;
	if (dbei->cbBlob > 0) {
		if (dbei->pBlob[0] == STEAM_DB_EVENT_CHATSTATES_GONE) {
			if (datatype == DBVT_WCHAR)
				return (INT_PTR)mir_wstrdup(TranslateT("closed chat session"));
			return (INT_PTR)mir_strdup(Translate("closed chat session"));
		}
	}

	return NULL;
}
