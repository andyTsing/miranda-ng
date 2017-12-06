﻿/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel

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

#pragma once

// Parser front-end

#define lltoa _i64toa

class facebook_json_parser
{
public:
	FacebookProto* proto;
	int parse_friends(std::string*, std::map< std::string, facebook_user* >*, bool);
	int parse_notifications(std::string*, std::map< std::string, facebook_notification* >*);
	int parse_messages(std::string*, std::vector< facebook_message >*, std::map< std::string, facebook_notification* >*);
	int parse_unread_threads(std::string*, std::vector< std::string >*);
	int parse_thread_messages(std::string*, std::vector< facebook_message >*, bool unreadOnly);
	int parse_history(std::string*, std::vector< facebook_message >*, std::string *);
	int parse_thread_info(std::string* data, std::string* user_id);
	int parse_user_info(std::string* data, facebook_user* fbu);
	int parse_chat_info(std::string* data, facebook_chatroom* fbc);
	int parse_chat_participant_names(std::string *data, std::map<std::string, chatroom_participant>* participants);
	int parse_messages_count(std::string *data, int *messagesCount, int *unreadCount);

	facebook_json_parser(FacebookProto* proto)
	{
		this->proto = proto;
	}
};
