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

#include <cstdint>

class ProgramOptions
{
public:
	class Config
	{
		friend class ProgramOptions;

	public:
		enum class CompressionLevel
		{
			None,
			Speed,
			Default,
			Size
		};

		Config(const Config&) = delete;
		Config(Config&&) = delete;
		Config& operator=(const Config&) = delete;
		Config& operator=(Config&&) = delete;

		std::uint16_t serverPort{ 15000 };
		std::size_t clientCountLimitTotal{ 8 };
		std::size_t clientCountLimitIpAddress{ 4 };
		std::size_t clientBufferCapacity{ 128 };
		std::size_t clientThreadCount{ 1 };
		CompressionLevel compressionLevel{ CompressionLevel::Default };

	private:
		Config() = default;
	};

	[[nodiscard]] static bool Init(int argc, char* argv[]);
	[[nodiscard]] static const Config& GetConfig();

	ProgramOptions() = delete;
	ProgramOptions(const ProgramOptions&) = delete;
	ProgramOptions(ProgramOptions&&) = delete;
	ProgramOptions& operator=(const ProgramOptions&) = delete;
	ProgramOptions& operator=(ProgramOptions&&) = delete;

private:
	static Config s_config;
};
