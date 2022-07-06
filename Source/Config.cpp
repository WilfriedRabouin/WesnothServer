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
#include <iostream>

#include <boost/program_options.hpp>

#include "Config.hpp"

Config Config::s_instance{};

[[nodiscard]] bool Config::Init(int argc, char* argv[])
{
	boost::program_options::options_description optionsDescription{ "Allowed options" };
	optionsDescription.add_options()
		("version,v", "display version")
		("help,h", "display help")
		("client-limit,cl", boost::program_options::value<std::size_t>())
	;

	boost::program_options::variables_map variablesMap{};
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, optionsDescription), variablesMap);
	//boost::program_options::store(boost::program_options::parse_config_file("config.txt", optionsDescription), variablesMap);
	boost::program_options::notify(variablesMap);

	if (variablesMap.count("help"))
	{
		std::cout << optionsDescription << "\n";
		return true;
	}

	/*if (vm.count("compression")) {
		cout << "Compression level was set to "
			<< vm["compression"].as<int>() << ".\n";
	}
	else {
		cout << "Compression level was not set.\n";
	}*/

	/*if (argc == 3 && argv[1] == std::string_view{ "--client-limit" })
	{
		if (argv[2] == std::string_view{ "none" })
		{
			s_instance.isClientCountLimited = false;
		}
		else
		{
			s_instance.clientCountLimit = std::stoull(argv[2]);
		}
	}*/

	return false;
}

[[nodiscard]] const Config& Config::GetInstance()
{
	return s_instance;
}
