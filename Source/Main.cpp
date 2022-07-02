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

#include <cstdlib>
#include <string>
#include <exception>
#include <iostream>

#include <spdlog/spdlog.h>

#include "Server.hpp"
#include "Versions.hpp"

int main(int argc, char* argv[])
{
#ifdef _DEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	bool isClientCountLimited{ true };
	std::size_t clientCountLimit{ 1 };

	if (argc == 3 && argv[1] == std::string{ "--client-limit" })
	{
		if (argv[2] == std::string{ "none" })
		{
			isClientCountLimited = false;
		}
		else
		{
			clientCountLimit = std::stoull(argv[2]);
		}
	}

	std::cout
		<< "Wesnoth Server - version " << g_serverVersion << "\n"
		<< "Compatible with client version " << g_clientVersion << "\n\n";

	try
	{
		RunServer(isClientCountLimited, clientCountLimit);
		return EXIT_SUCCESS;
	}
	catch (const std::exception& e)
	{
		spdlog::critical("{}", e.what());
		return EXIT_FAILURE;
	}
}
