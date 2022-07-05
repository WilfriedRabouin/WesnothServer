/*
Copyright (C) 2022 Wilfried Rabouin

This file is part of WesnothServer.

WesnothServer is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

WesnothServer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with WesnothServer.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <string_view>
#include <string>

#include "Config.hpp"

Config Config::s_instance{};

[[nodiscard]] bool Config::Init(int argc, char* argv[])
{
	// TODO: load config file

	if (argc == 3 && argv[1] == std::string_view{ "--client-limit" })
	{
		if (argv[2] == std::string_view{ "none" })
		{
			s_instance.isClientCountLimited = false;
		}
		else
		{
			s_instance.clientCountLimit = std::stoull(argv[2]);
		}
	}

	return false;
}

[[nodiscard]] const Config& Config::GetInstance()
{
	return s_instance;
}
