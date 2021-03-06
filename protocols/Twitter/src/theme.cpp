/*
Copyright © 2012-18 Miranda NG team
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "theme.h"
#include "proto.h"

extern OBJLIST<TwitterProto> g_Instances;

static IconItem icons[] =
{
	{ LPGEN("Twitter Icon"), "twitter", IDI_TWITTER },
	{ LPGEN("Tweet"), "tweet", IDI_TWITTER },
	{ LPGEN("Reply to Tweet"), "reply", IDI_TWITTER },

	{ LPGEN("Visit Homepage"), "homepage", 0 },
};

void TwitterInitSounds(void)
{
	Skin_AddSound("TwitterNewContact", LPGENW("Twitter"), LPGENW("First tweet from new contact"));
	Skin_AddSound("TwitterNew",        LPGENW("Twitter"), LPGENW("New tweet"));
}

// TODO: uninit
void InitIcons(void)
{
	Icon_Register(g_hInstance, "Protocols/Twitter", icons, _countof(icons), "Twitter");
	icons[_countof(icons) - 1].hIcolib = Skin_GetIconHandle(SKINICON_EVENT_URL);
}

HANDLE GetIconHandle(const char *name)
{
	for (size_t i = 0; i < _countof(icons); i++)
		if (mir_strcmp(icons[i].szName, name) == 0)
			return icons[i].hIcolib;

	return nullptr;
}

// Contact List menu stuff
static HGENMENU g_hMenuItems[2];

// Helper functions
static TwitterProto* GetInstanceByHContact(MCONTACT hContact)
{
	char *proto = GetContactProto(hContact);
	if (!proto)
		return nullptr;

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!mir_strcmp(proto, g_Instances[i].m_szModuleName))
			return &g_Instances[i];

	return nullptr;
}

template<INT_PTR(__cdecl TwitterProto::*Fcn)(WPARAM, LPARAM)>
INT_PTR GlobalService(WPARAM wParam, LPARAM lParam)
{
	TwitterProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? (proto->*Fcn)(wParam, lParam) : 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	ShowContactMenus(false);

	TwitterProto *proto = GetInstanceByHContact(MCONTACT(wParam));
	return proto ? proto->OnPrebuildContactMenu(wParam, lParam) : 0;
}

void InitContactMenus()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CMenuItem mi;
	mi.flags = CMIF_NOTOFFLINE | CMIF_UNICODE;

	SET_UID(mi, 0xfc4e1245, 0xc8e0, 0x4de2, 0x92, 0x15, 0xfc, 0xcf, 0x48, 0xf9, 0x41, 0x56);
	mi.position = -2000006000;
	mi.hIcolibItem = GetIconHandle("reply");
	mi.name.w = LPGENW("Reply...");
	mi.pszService = "Twitter/ReplyToTweet";
	g_hMenuItems[0] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&TwitterProto::ReplyToTweet>);

	SET_UID(mi, 0x7f7e4c24, 0x821c, 0x450f, 0x93, 0x76, 0xbe, 0x65, 0xe9, 0x2f, 0xb6, 0xc2);
	mi.position = -2000006000;
	mi.hIcolibItem = GetIconHandle("homepage");
	mi.name.w = LPGENW("Visit Homepage");
	mi.pszService = "Twitter/VisitHomepage";
	g_hMenuItems[1] = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, GlobalService<&TwitterProto::VisitHomepage>);
}

void ShowContactMenus(bool show)
{
	for (size_t i = 0; i < _countof(g_hMenuItems); i++)
		Menu_ShowItem(g_hMenuItems[i], show);
}
