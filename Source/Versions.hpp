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

#pragma once

#include <string_view>

namespace Versions
{
	constexpr std::string_view g_server{ "1.0.0" };
	constexpr std::string_view g_client{ "1.16.2" };

	constexpr std::string_view g_formatString
	{
		"Wesnoth Server - version {}\n"
		"Compatible with client version {}"
	};
}
