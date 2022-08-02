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

#include <string>
#include <iostream>
#include <filesystem>
#include <thread>
#include <unordered_map>
#include <algorithm>

#include <boost/program_options.hpp>
#include <fmt/core.h>

#include "ProgramOptions.hpp"
#include "Versions.hpp"

ProgramOptions::Config ProgramOptions::s_config{};

[[nodiscard]] bool ProgramOptions::Init(int argc, char* argv[])
{
	boost::program_options::options_description genericOptions{};
	genericOptions.add_options()
		("help,h", "Print help")
		("version,v", "Print version")
		("config,c", boost::program_options::value<std::string>(), "Set configuration file");

	boost::program_options::options_description configurationOptions{ "Configuration" };
	configurationOptions.add_options()
		("server_port", boost::program_options::value<std::uint16_t>())
		("client_count_limit_total", boost::program_options::value<std::size_t>())
		("client_count_limit_ip_address", boost::program_options::value<std::size_t>())
		("client_buffer_capacity", boost::program_options::value<std::size_t>())
		("client_thread_count", boost::program_options::value<std::string>())
		("compression_level", boost::program_options::value<std::string>());

	boost::program_options::options_description commandLineOptions{ "Options" };
	commandLineOptions.add(genericOptions).add(configurationOptions);

	boost::program_options::variables_map variablesMap{};
	boost::program_options::store(boost::program_options::parse_command_line(argc, argv, commandLineOptions), variablesMap);
	boost::program_options::notify(variablesMap);

	if (variablesMap.count("help"))
	{
		std::cout << commandLineOptions;
		return false;
	}

	if (variablesMap.count("version"))
	{
		fmt::print
		(
			"Wesnoth Server - version {}\n"
			"Compatible with client version {}",
			Versions::g_server, Versions::g_compatibleClient
		);
		return false;
	}

	if (variablesMap.count("config"))
	{
		/*try
		{*/
		const std::string configurationFileName{ variablesMap["config"].as<std::string>() };

		if (std::filesystem::exists(configurationFileName))
		{
			fmt::print("Load configuration file\n");
			boost::program_options::store(boost::program_options::parse_config_file(configurationFileName.c_str(), configurationOptions), variablesMap);
			boost::program_options::notify(variablesMap);
		}
		else
		{
			fmt::print("Configuration file not found");
			return false;
		}
		/*}
		catch (const std::exception& exception)
		{
			fmt::print("Configuration file not found ({})", exception.what());
			return false;
		}*/
	}
	else
	{
		//	constexpr std::string configurationFile{ "config.ini" };
	}

	if (variablesMap.count("server_port"))
	{
		s_config.serverPort = variablesMap["server_port"].as<std::uint16_t>();
	}

	fmt::print("server_port={}\n", s_config.serverPort);

	// TODO: check if >=1
	if (variablesMap.count("client_count_limit_total"))
	{
		s_config.clientCountLimitTotal = variablesMap["client_count_limit_total"].as<std::size_t>();
	}

	fmt::print("client_count_limit_total={}\n", s_config.clientCountLimitTotal);

	if (variablesMap.count("client_count_limit_ip_address"))
	{
		s_config.clientCountLimitIpAddress = variablesMap["client_count_limit_ip_address"].as<std::size_t>();
	}

	fmt::print("client_count_limit_ip_address={}\n", s_config.clientCountLimitIpAddress);

	if (variablesMap.count("client_buffer_capacity"))
	{
		s_config.clientBufferCapacity = variablesMap["client_buffer_capacity"].as<std::size_t>();
	}

	fmt::print("client_buffer_capacity={}\n", s_config.clientBufferCapacity);

	if (variablesMap.count("client_thread_count"))
	{
		const std::string value{ variablesMap["client_thread_count"].as<std::string>() };

		if (value == "auto")
		{
			s_config.clientThreadCount = std::max(std::thread::hardware_concurrency(), 2u) - 1;
		}
		else
		{
			s_config.clientThreadCount = std::stoull(value);
		}
	}

	fmt::print("client_thread_count={}\n", s_config.clientThreadCount);

	// resume here
	if (variablesMap.count("compression_level"))
	{
		const std::string value{ variablesMap["compression_level"].as<std::string>() };

		static const std::unordered_map<std::string, Config::CompressionLevel> mapping
		{
			{ "none", Config::CompressionLevel::None },
			{ "speed", Config::CompressionLevel::Speed },
			{ "default", Config::CompressionLevel::Default },
			{ "size", Config::CompressionLevel::Size },
		};

		if (mapping.contains(value))
		{
			s_config.compressionLevel = mapping.at(value);
		}
		else
		{
			fmt::print("Unknown compression level");
		}
	}

	return true;
}

[[nodiscard]] const ProgramOptions::Config& ProgramOptions::GetConfig()
{
	return s_config;
}
