#ifndef _STEAM_REQUEST_PENDING_H_
#define _STEAM_REQUEST_PENDING_H_

class ApprovePendingRequest : public HttpRequest
{
public:
	ApprovePendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(HttpPost, FORMAT, STEAM_WEB_URL "/profiles/%s/home_process", steamId)
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		Headers << CHAR_PARAM("Cookie", cookie);

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("sessionID", sessionId)
			<< CHAR_PARAM("id", who)
			<< CHAR_PARAM("perform", "accept")
			<< CHAR_PARAM("action", "approvePending")
			<< CHAR_PARAM("itype", "friend")
			<< INT_PARAM("json", 1)
			<< INT_PARAM("xml", 0);
	}
};

class IgnorePendingRequest : public HttpRequest
{
public:
	IgnorePendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(HttpPost, FORMAT, STEAM_WEB_URL "/profiles/%s/home_process", steamId)
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		Headers << CHAR_PARAM("Cookie", cookie);

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("sessionID", sessionId)
			<< CHAR_PARAM("id", who)
			<< CHAR_PARAM("perform", "ignore")
			<< CHAR_PARAM("action", "approvePending")
			<< CHAR_PARAM("itype", "friend")
			<< INT_PARAM("json", 1)
			<< INT_PARAM("xml", 0);
	}
};

class BlockPendingRequest : public HttpRequest
{
public:
	BlockPendingRequest(const char *token, const char *sessionId, const char *steamId, const char *who) :
		HttpRequest(HttpPost, FORMAT, STEAM_WEB_URL "/profiles/%s/home_process", steamId)
	{
		char login[MAX_PATH];
		mir_snprintf(login, "%s||oauth:%s", steamId, token);

		char cookie[MAX_PATH];
		mir_snprintf(cookie, "steamLogin=%s;sessionid=%s;mobileClientVersion=1291812;forceMobile=1;mobileClient=ios", login, sessionId);

		Headers << CHAR_PARAM("Cookie", cookie);

		Content = new FormUrlEncodedContent(this)
			<< CHAR_PARAM("sessionID", sessionId)
			<< CHAR_PARAM("id", who)
			<< CHAR_PARAM("perform", "block")
			<< CHAR_PARAM("action", "approvePending")
			<< CHAR_PARAM("itype", "friend")
			<< INT_PARAM("json", 1)
			<< INT_PARAM("xml", 0);
	}
};

#endif //_STEAM_REQUEST_PENDING_H_
