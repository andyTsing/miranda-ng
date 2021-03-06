/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#ifndef M_DB_INT_H__
#define M_DB_INT_H__ 1

#ifndef M_CORE_H__
	#include <m_core.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// basic database interface

struct DBCachedGlobalValue
{
	char *name;
	DBVARIANT value;
};

struct DBCachedContactValue
{
	char *name;
	DBVARIANT value;
	DBCachedContactValue *next;
};

struct DBCachedContactBase
{
	MCONTACT contactID;
	char *szProto;
	DBCachedContactValue *first, *last;

	// metacontacts
	int       nSubs;    // == -1 -> not a metacontact
	MCONTACT *pSubs;
	MCONTACT  parentID; // == 0 -> not a subcontact
	int       nDefault; // default sub number

	__forceinline bool IsMeta() const { return nSubs != -1; }
	__forceinline bool IsSub() const { return parentID != 0; }
};

#ifndef OWN_CACHED_CONTACT
struct DBCachedContact : public DBCachedContactBase {};
#else
struct DBCachedContact;
#endif

interface MIDatabaseCache : public MZeroedObject
{
	STDMETHOD_(DBCachedContact*, AddContactToCache)(MCONTACT contactID) PURE;
	STDMETHOD_(DBCachedContact*, GetCachedContact)(MCONTACT contactID) PURE;
	STDMETHOD_(DBCachedContact*, GetFirstContact)(void) PURE;
	STDMETHOD_(DBCachedContact*, GetNextContact)(MCONTACT contactID) PURE;
	STDMETHOD_(void, FreeCachedContact)(MCONTACT contactID) PURE;

	STDMETHOD_(char*, InsertCachedSetting)(const char *szName, int) PURE;
	STDMETHOD_(char*, GetCachedSetting)(const char *szModuleName, const char *szSettingName, int, int) PURE;
	STDMETHOD_(void, SetCachedVariant)(DBVARIANT *s, DBVARIANT *d) PURE;
	STDMETHOD_(DBVARIANT*, GetCachedValuePtr)(MCONTACT contactID, char *szSetting, int bAllocate) PURE;
};

interface MIR_APP_EXPORT MIDatabase
{
	MIDatabaseCache* m_cache;

	STDMETHOD_(BOOL, IsRelational)(void) PURE;
	STDMETHOD_(void, SetCacheSafetyMode)(BOOL) PURE;

	STDMETHOD_(LONG, GetContactCount)(void) PURE;
	STDMETHOD_(MCONTACT, FindFirstContact)(const char *szProto = NULL) PURE;
	STDMETHOD_(MCONTACT, FindNextContact)(MCONTACT contactID, const char *szProto = NULL) PURE;

	STDMETHOD_(LONG, DeleteContact)(MCONTACT contactID) PURE;
	STDMETHOD_(MCONTACT, AddContact)(void) PURE;
	STDMETHOD_(BOOL, IsDbContact)(MCONTACT contactID) PURE;
	STDMETHOD_(LONG, GetContactSize)(void) PURE;

	STDMETHOD_(LONG, GetEventCount)(MCONTACT contactID) PURE;
	STDMETHOD_(MEVENT, AddEvent)(MCONTACT contactID, DBEVENTINFO *dbe) PURE;
	STDMETHOD_(BOOL, DeleteEvent)(MCONTACT contactID, MEVENT hDbEvent) PURE;
	STDMETHOD_(LONG, GetBlobSize)(MEVENT hDbEvent) PURE;
	STDMETHOD_(BOOL, GetEvent)(MEVENT hDbEvent, DBEVENTINFO *dbe) PURE;
	STDMETHOD_(BOOL, MarkEventRead)(MCONTACT contactID, MEVENT hDbEvent) PURE;
	STDMETHOD_(MCONTACT, GetEventContact)(MEVENT hDbEvent) PURE;
	STDMETHOD_(MEVENT, FindFirstEvent)(MCONTACT contactID) PURE;
	STDMETHOD_(MEVENT, FindFirstUnreadEvent)(MCONTACT contactID) PURE;
	STDMETHOD_(MEVENT, FindLastEvent)(MCONTACT contactID) PURE;
	STDMETHOD_(MEVENT, FindNextEvent)(MCONTACT contactID, MEVENT hDbEvent) PURE;
	STDMETHOD_(MEVENT, FindPrevEvent)(MCONTACT contactID, MEVENT hDbEvent) PURE;

	STDMETHOD_(BOOL, DeleteModule)(MCONTACT contactID, LPCSTR szModule) PURE;
	STDMETHOD_(BOOL, EnumModuleNames)(DBMODULEENUMPROC pFunc, void *pParam) PURE;

	STDMETHOD_(BOOL, GetContactSetting)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv) PURE;
	STDMETHOD_(BOOL, GetContactSettingStr)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv) PURE;
	STDMETHOD_(BOOL, GetContactSettingStatic)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv) PURE;
	STDMETHOD_(BOOL, FreeVariant)(DBVARIANT *dbv) PURE;
	STDMETHOD_(BOOL, WriteContactSetting)(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws) PURE;
	STDMETHOD_(BOOL, DeleteContactSetting)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting) PURE;
	STDMETHOD_(BOOL, EnumContactSettings)(MCONTACT contactID, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param) PURE;
	STDMETHOD_(BOOL, SetSettingResident)(BOOL bIsResident, const char *pszSettingName) PURE;
	STDMETHOD_(BOOL, EnumResidentSettings)(DBMODULEENUMPROC pFunc, void *pParam) PURE;
	STDMETHOD_(BOOL, IsSettingEncrypted)(LPCSTR szModule, LPCSTR szSetting) PURE;

	STDMETHOD_(BOOL, MetaDetouchSub)(DBCachedContact*, int nSub) PURE;
	STDMETHOD_(BOOL, MetaSetDefault)(DBCachedContact*) PURE;
	STDMETHOD_(BOOL, MetaMergeHistory)(DBCachedContact *ccMeta, DBCachedContact *ccSub) PURE;
	STDMETHOD_(BOOL, MetaSplitHistory)(DBCachedContact *ccMeta, DBCachedContact *ccSub) PURE;
};

class MIR_APP_EXPORT MDatabaseCommon : public MIDatabase
{

protected:
	int m_codePage;
	
	mir_cs m_csDbAccess;
	LIST<char> m_lResidentSettings;

protected:
	MDatabaseCommon();

	int CheckProto(DBCachedContact *cc, const char *proto);

	STDMETHOD_(BOOL, GetContactSettingWorker)(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic) PURE;

public:
	STDMETHODIMP_(BOOL) DeleteModule(MCONTACT contactID, LPCSTR szModule);

	STDMETHODIMP_(MCONTACT) FindFirstContact(const char *szProto = nullptr);
	STDMETHODIMP_(MCONTACT) FindNextContact(MCONTACT contactID, const char *szProto = nullptr);

	STDMETHODIMP_(BOOL) MetaDetouchSub(DBCachedContact *cc, int nSub);
	STDMETHODIMP_(BOOL) MetaSetDefault(DBCachedContact *cc);

	STDMETHODIMP_(BOOL) IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting);
	STDMETHODIMP_(BOOL) GetContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
	STDMETHODIMP_(BOOL) GetContactSettingStr(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
	STDMETHODIMP_(BOOL) GetContactSettingStatic(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv);
	STDMETHODIMP_(BOOL) FreeVariant(DBVARIANT *dbv);
	
	STDMETHODIMP_(BOOL) EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam);
	STDMETHODIMP_(BOOL) SetSettingResident(BOOL bIsResident, const char *pszSettingName);
};

///////////////////////////////////////////////////////////////////////////////
// basic database checker interface

#define STATUS_MESSAGE    0
#define STATUS_WARNING    1
#define STATUS_ERROR      2
#define STATUS_FATAL      3
#define STATUS_SUCCESS    4

struct DBCHeckCallback
{
	int    cbSize;
	DWORD  spaceProcessed, spaceUsed;
	HANDLE hOutFile;
	int    bCheckOnly, bBackup, bAggressive, bEraseHistory, bMarkRead, bConvertUtf;

	void (*pfnAddLogMessage)(int type, const wchar_t* ptszFormat, ...);
};

interface MIDatabaseChecker
{
	STDMETHOD_(BOOL,Start)(DBCHeckCallback *callback) PURE;
	STDMETHOD_(BOOL,CheckDb)(int phase, int firstTime) PURE;
	STDMETHOD_(VOID,Destroy)() PURE;
};

///////////////////////////////////////////////////////////////////////////////
// Each database plugin should register itself using this structure

/*
 Codes for DATABASELINK functions
*/

// grokHeader() error codes
#define EGROKPRF_NOERROR   0
#define EGROKPRF_CANTREAD  1  // can't open the profile for reading
#define EGROKPRF_UNKHEADER 2  // header not supported, not a supported profile
#define EGROKPRF_VERNEWER  3  // header correct, version in profile newer than reader/writer
#define EGROKPRF_DAMAGED   4  // header/version fine, other internal data missing, damaged.
#define EGROKPRF_OBSOLETE  5  // obsolete database version detected, requiring conversion

// makeDatabase() error codes
#define EMKPRF_CREATEFAILED 1   // for some reason CreateFile() didnt like something

struct DATABASELINK
{
	int cbSize;
	char* szShortName;  // uniqie short database name
	wchar_t* szFullName;  // in English, auto-translated by the core

	/*
		profile: pointer to a string which contains full path + name
		Affect: The database plugin should create the profile, the filepath will not exist at
			the time of this call, profile will be C:\..\<name>.dat
		Returns: 0 on success, non zero on failure - error contains extended error information, see EMKPRF_*
	*/
	int (*makeDatabase)(const wchar_t *profile);

	/*
		profile: [in] a null terminated string to file path of selected profile
		error: [in/out] pointer to an int to set with error if any
		Affect: Ask the database plugin if it supports the given profile, if it does it will
			return 0, if it doesnt return 1, with the error set in error -- EGROKPRF_* can be valid error
			condition, most common error would be [EGROKPRF_UNKHEADER]
		Note: Just because 1 is returned, doesnt mean the profile is not supported, the profile might be damaged
			etc.
		Returns: 0 on success, non zero on failure
	*/
	int (*grokHeader)(const wchar_t *profile);

	/*
	Affect: Tell the database to create all services/hooks that a 3.xx legacy database might support into link,
		which is a PLUGINLINK structure
	Returns: 0 on success, nonzero on failure
	*/
	MIDatabase* (*Load)(const wchar_t *profile, BOOL bReadOnly);

	/*
	Affect: The database plugin should shutdown, unloading things from the core and freeing internal structures
	Returns: 0 on success, nonzero on failure
	Note: Unload() might be called even if Load(void) was never called, wasLoaded is set to 1 if Load(void) was ever called.
	*/
	int (*Unload)(MIDatabase*);

	/*
	Returns a pointer to the database checker or NULL if a database doesn't support checking
	When you don't need this object aanymore,  call its Destroy() method
	*/
	MIDatabaseChecker* (*CheckDB)(const wchar_t *profile, int *error);
};

///////////////////////////////////////////////////////////////////////////////
// cache access function

EXTERN_C MIR_CORE_DLL(DBCachedContact*) db_get_contact(MCONTACT);

///////////////////////////////////////////////////////////////////////////////
// Database list's functions

EXTERN_C MIR_CORE_DLL(MIDatabase*) db_get_current(void);

// registers a database plugin

EXTERN_C MIR_APP_DLL(void) RegisterDatabasePlugin(DATABASELINK *pDescr);

// looks for a database plugin suitable to open this file
// returns DATABASELINK* of the required plugin or NULL on error

EXTERN_C MIR_APP_DLL(DATABASELINK*) FindDatabasePlugin(const wchar_t *ptszFileName);

// initializes a database instance

EXTERN_C MIR_APP_DLL(void) InitDbInstance(MIDatabase *pDatabase);

// destroys a database instance

EXTERN_C MIR_APP_DLL(void) DestroyDbInstance(MIDatabase *pDatabase);

#endif // M_DB_INT_H__
