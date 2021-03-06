/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2012-18 Miranda NG team

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
#include "jabber_secur.h"

/////////////////////////////////////////////////////////////////////////////////////////
// ntlm auth - LanServer based authorization

TNtlmAuth::TNtlmAuth(ThreadData *info, const char *mechanism, const wchar_t *hostname) :
	TJabberAuth(info)
{
	szName = mechanism;
	szHostName = hostname;

	const wchar_t *szProvider;
	if (!mir_strcmp(mechanism, "GSS-SPNEGO"))
		szProvider = L"Negotiate";
	else if (!mir_strcmp(mechanism, "GSSAPI"))
		szProvider = L"Kerberos";
	else if (!mir_strcmp(mechanism, "NTLM"))
		szProvider = L"NTLM";
	else {
LBL_Invalid:
		bIsValid = false;
		hProvider = nullptr;
		return;
	}

	wchar_t szSpn[1024]; szSpn[0] = 0;
	if (!mir_strcmp(mechanism, "GSSAPI"))
		if (!getSpn(szSpn, _countof(szSpn)))
			goto LBL_Invalid;

	if ((hProvider = Netlib_InitSecurityProvider(szProvider, szSpn)) == nullptr)
		bIsValid = false;
}

TNtlmAuth::~TNtlmAuth()
{
	if (hProvider != nullptr)
		Netlib_DestroySecurityProvider(hProvider);
}

bool TNtlmAuth::getSpn(wchar_t* szSpn, size_t dwSpnLen)
{
	wchar_t szFullUserName[128] = L"";
	ULONG szFullUserNameLen = _countof(szFullUserName);
	if (!GetUserNameEx(NameDnsDomain, szFullUserName, &szFullUserNameLen)) {
		szFullUserName[0] = 0;
		szFullUserNameLen = _countof(szFullUserName);
		GetUserNameEx(NameSamCompatible, szFullUserName, &szFullUserNameLen);
	}

	wchar_t *name = wcsrchr(szFullUserName, '\\');
	if (name) *name = 0;
	else return false;

	if (szHostName && szHostName[0]) {
		wchar_t *szFullUserNameU = wcsupr(mir_wstrdup(szFullUserName));
		mir_snwprintf(szSpn, dwSpnLen, L"xmpp/%s/%s@%s", szHostName, szFullUserName, szFullUserNameU);
		mir_free(szFullUserNameU);
	}
	else {
		const char* connectHost = info->conn.manualHost[0] ? info->conn.manualHost : info->conn.server;

		unsigned long ip = inet_addr(connectHost);
		PHOSTENT host = (ip == INADDR_NONE) ? nullptr : gethostbyaddr((char*)&ip, 4, AF_INET);
		if (host && host->h_name)
			connectHost = host->h_name;

		wchar_t *connectHostT = mir_a2u(connectHost);
		mir_snwprintf(szSpn, dwSpnLen, L"xmpp/%s@%s", connectHostT, wcsupr(szFullUserName));
		mir_free(connectHostT);
	}

	Netlib_Logf(nullptr, "SPN: %S", szSpn);
	return true;
}

char* TNtlmAuth::getInitialRequest()
{
	if (!hProvider)
		return nullptr;

	// This generates login method advertisement packet
	if (info->conn.password[0] != 0)
		return Netlib_NtlmCreateResponse(hProvider, "", info->conn.username, info->conn.password, complete);

	return Netlib_NtlmCreateResponse(hProvider, "", nullptr, nullptr, complete);
}

char* TNtlmAuth::getChallenge(const wchar_t *challenge)
{
	if (!hProvider)
		return nullptr;

	ptrA text((!mir_wstrcmp(challenge, L"=")) ? mir_strdup("") : mir_u2a(challenge));
	if (info->conn.password[0] != 0)
		return Netlib_NtlmCreateResponse(hProvider, text, info->conn.username, info->conn.password, complete);
	
	return Netlib_NtlmCreateResponse(hProvider, text, nullptr, nullptr, complete);
}

/////////////////////////////////////////////////////////////////////////////////////////
// md5 auth - digest-based authorization

TMD5Auth::TMD5Auth(ThreadData *info) :
	TJabberAuth(info),
	iCallCount(0)
{
	szName = "DIGEST-MD5";
}

TMD5Auth::~TMD5Auth()
{
}

char* TMD5Auth::getChallenge(const wchar_t *challenge)
{
	if (iCallCount > 0)
		return nullptr;

	iCallCount++;

	size_t resultLen;
	ptrA text((char*)mir_base64_decode( _T2A(challenge), &resultLen));

	TStringPairs pairs(text);
	const char *realm = pairs["realm"], *nonce = pairs["nonce"];

	char cnonce[40], tmpBuf[40];
	DWORD digest[4], hash1[4], hash2[4];
	mir_md5_state_t ctx;

	Utils_GetRandom(digest, sizeof(digest));
	mir_snprintf(cnonce, "%08x%08x%08x%08x", htonl(digest[0]), htonl(digest[1]), htonl(digest[2]), htonl(digest[3]));

	T2Utf uname(info->conn.username), passw(info->conn.password);
	ptrA  serv(mir_utf8encode(info->conn.server));

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (BYTE*)(char*)uname, (int)mir_strlen(uname));
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)realm, (int)mir_strlen(realm));
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)(char*)passw, (int)mir_strlen(passw));
	mir_md5_finish(&ctx, (BYTE*)hash1);

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (BYTE*)hash1, 16);
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)nonce, (int)mir_strlen(nonce));
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)cnonce, (int)mir_strlen(cnonce));
	mir_md5_finish(&ctx, (BYTE*)hash1);

	mir_md5_init(&ctx);
	mir_md5_append(&ctx, (BYTE*)"AUTHENTICATE:xmpp/", 18);
	mir_md5_append(&ctx, (BYTE*)(char*)serv, (int)mir_strlen(serv));
	mir_md5_finish(&ctx, (BYTE*)hash2);

	mir_md5_init(&ctx);
	mir_snprintf(tmpBuf, "%08x%08x%08x%08x", htonl(hash1[0]), htonl(hash1[1]), htonl(hash1[2]), htonl(hash1[3]));
	mir_md5_append(&ctx, (BYTE*)tmpBuf, (int)mir_strlen(tmpBuf));
	mir_md5_append(&ctx, (BYTE*)":", 1);
	mir_md5_append(&ctx, (BYTE*)nonce, (int)mir_strlen(nonce));
	mir_snprintf(tmpBuf, ":%08d:", iCallCount);
	mir_md5_append(&ctx, (BYTE*)tmpBuf, (int)mir_strlen(tmpBuf));
	mir_md5_append(&ctx, (BYTE*)cnonce, (int)mir_strlen(cnonce));
	mir_md5_append(&ctx, (BYTE*)":auth:", 6);
	mir_snprintf(tmpBuf, "%08x%08x%08x%08x", htonl(hash2[0]), htonl(hash2[1]), htonl(hash2[2]), htonl(hash2[3]));
	mir_md5_append(&ctx, (BYTE*)tmpBuf, (int)mir_strlen(tmpBuf));
	mir_md5_finish(&ctx, (BYTE*)digest);

	char *buf = (char*)alloca(8000);
	int cbLen = mir_snprintf(buf, 8000,
		"username=\"%s\",realm=\"%s\",nonce=\"%s\",cnonce=\"%s\",nc=%08d,"
		"qop=auth,digest-uri=\"xmpp/%s\",charset=utf-8,response=%08x%08x%08x%08x",
		uname, realm, nonce, cnonce, iCallCount, serv,
		htonl(digest[0]), htonl(digest[1]), htonl(digest[2]), htonl(digest[3]));

	return mir_base64_encode(buf, cbLen);
}

/////////////////////////////////////////////////////////////////////////////////////////
// SCRAM-SHA-1 authorization

TScramAuth::TScramAuth(ThreadData *info) :
	TJabberAuth(info)
{
	szName = "SCRAM-SHA-1";
	cnonce = msg1 = serverSignature = nullptr;
}

TScramAuth::~TScramAuth()
{
	mir_free(cnonce);
	mir_free(msg1);
	mir_free(serverSignature);
}

void TScramAuth::Hi(BYTE* res, char* passw, size_t passwLen, char* salt, size_t saltLen, int ind)
{
	size_t bufLen = saltLen + sizeof(UINT32);
	BYTE *u = (BYTE*)_alloca(max(bufLen, MIR_SHA1_HASH_SIZE));
	memcpy(u, salt, saltLen); *(UINT32*)(u + saltLen) = htonl(1);
	
	memset(res, 0, MIR_SHA1_HASH_SIZE);

	for (int i = 0; i < ind; i++) {
		mir_hmac_sha1(u, (BYTE*)passw, passwLen, u, bufLen);
		bufLen = MIR_SHA1_HASH_SIZE;

		for (unsigned j = 0; j < MIR_SHA1_HASH_SIZE; j++)
			res[j] ^= u[j];
	}
}

char* TScramAuth::getChallenge(const wchar_t *challenge)
{
	size_t chlLen, saltLen = 0;
	ptrA snonce, salt;
	int ind = -1;

	ptrA chl((char*)mir_base64_decode(_T2A(challenge), &chlLen));

	for (char *p = strtok(NEWSTR_ALLOCA(chl), ","); p != nullptr; p = strtok(nullptr, ",")) {
		if (*p == 'r' && p[1] == '=') { // snonce
			if (strncmp(cnonce, p + 2, mir_strlen(cnonce)))
				return nullptr;
			snonce = mir_strdup(p + 2);
		}
		else if (*p == 's' && p[1] == '=') // salt
			salt = (char*)mir_base64_decode(p + 2, &saltLen);
		else if (*p == 'i' && p[1] == '=')
			ind = atoi(p + 2);
	}

	if (snonce == nullptr || salt == nullptr || ind == -1)
		return nullptr;

	ptrA passw(mir_utf8encodeW(info->conn.password));
	size_t passwLen = mir_strlen(passw);

	BYTE saltedPassw[MIR_SHA1_HASH_SIZE];
	Hi(saltedPassw, passw, passwLen, salt, saltLen, ind);

	BYTE clientKey[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(clientKey, saltedPassw, sizeof(saltedPassw), (BYTE*)"Client Key", 10);

	BYTE storedKey[MIR_SHA1_HASH_SIZE];

	mir_sha1_ctx ctx;
	mir_sha1_init(&ctx);
	mir_sha1_append(&ctx, clientKey, MIR_SHA1_HASH_SIZE);
	mir_sha1_finish(&ctx, storedKey);

	char authmsg[4096];
	int authmsgLen = mir_snprintf(authmsg, "%s,%s,c=biws,r=%s", msg1, chl, snonce);

	BYTE clientSig[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(clientSig, storedKey, sizeof(storedKey), (BYTE*)authmsg, authmsgLen);

	BYTE clientProof[MIR_SHA1_HASH_SIZE];
	for (unsigned j = 0; j < sizeof(clientKey); j++)
		clientProof[j] = clientKey[j] ^ clientSig[j];

	/* Calculate the server signature */
	BYTE serverKey[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(serverKey, saltedPassw, sizeof(saltedPassw), (BYTE*)"Server Key", 10);

	BYTE srvSig[MIR_SHA1_HASH_SIZE];
	mir_hmac_sha1(srvSig, serverKey, sizeof(serverKey), (BYTE*)authmsg, authmsgLen);
	serverSignature = mir_base64_encode(srvSig, sizeof(srvSig));

	char buf[4096];
	ptrA encproof(mir_base64_encode(clientProof, sizeof(clientProof)));
	int cbLen = mir_snprintf(buf, "c=biws,r=%s,p=%s", snonce, encproof);
	return mir_base64_encode(buf, cbLen);
}

char* TScramAuth::getInitialRequest()
{
	T2Utf uname(info->conn.username);

	unsigned char nonce[24];
	Utils_GetRandom(nonce, sizeof(nonce));
	cnonce = mir_base64_encode(nonce, sizeof(nonce));

	char buf[4096];
	int cbLen = mir_snprintf(buf, "n,,n=%s,r=%s", uname, cnonce);
	msg1 = mir_strdup(buf + 3);
	return mir_base64_encode(buf, cbLen);
}

bool TScramAuth::validateLogin(const wchar_t *challenge)
{
	size_t chlLen;
	ptrA chl((char*)mir_base64_decode(_T2A(challenge), &chlLen));
	return chl && strncmp((char*)chl + 2, serverSignature, chlLen - 2) == 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plain auth - the most simple one

TPlainAuth::TPlainAuth(ThreadData *info, bool old) :
	TJabberAuth(info)
{
	szName = "PLAIN";
	bOld = old;
}

TPlainAuth::~TPlainAuth()
{
}

char* TPlainAuth::getInitialRequest()
{
	T2Utf uname(info->conn.username), passw(info->conn.password);

	size_t size = 2 * mir_strlen(uname) + mir_strlen(passw) + mir_strlen(info->conn.server) + 4;
	char *toEncode = (char*)alloca(size);
	if (bOld)
		size = mir_snprintf(toEncode, size, "%s@%s%c%s%c%s", uname, info->conn.server, 0, uname, 0, passw);
	else
		size = mir_snprintf(toEncode, size, "%c%s%c%s", 0, uname, 0, passw);

	return mir_base64_encode(toEncode, size);
}

/////////////////////////////////////////////////////////////////////////////////////////
// basic type

TJabberAuth::TJabberAuth(ThreadData* pInfo) :
	bIsValid(true),
	complete(0),
	szName(nullptr),
	info(pInfo)
{
}

TJabberAuth::~TJabberAuth()
{
}

char* TJabberAuth::getInitialRequest()
{
	return nullptr;
}

char* TJabberAuth::getChallenge(const wchar_t*)
{
	return nullptr;
}

bool TJabberAuth::validateLogin(const wchar_t*)
{
	return true;
}
