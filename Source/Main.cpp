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
#include <exception>
#include <fstream>
#include <string>

#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include "Server.hpp"
#include "ProgramOptions.hpp"

int main(int argc, char* argv[])
{
	if (std::ifstream bannerFile{ "Banner.txt" }; bannerFile)
	{
		fmt::print("\n");

		while (bannerFile)
		{
			std::string line{};
			std::getline(bannerFile, line);
			fmt::print("{}\n", line);
		}

		fmt::print("\n");
	}

	if (!ProgramOptions::Init(argc, argv))
	{
		return EXIT_SUCCESS;
	}

	try
	{
		Server::Setup();
	}
	catch (const std::exception& exception)
	{
		fmt::print("{}", exception.what());
		return EXIT_FAILURE;
	}

#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif
	spdlog::set_pattern("[%Y-%m-%d %T.%e] [%t] [%^%l%$] %v");

	try
	{
		Server::Run();
	}
	catch (const std::exception& exception)
	{
		spdlog::critical("{}", exception.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
