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

class Config
{
public:
	enum class CompressionLevel
	{
		None,
		Speed,
		Default,
		Size
	};

	[[nodiscard]] static bool Init(int argc, char* argv[]);
	[[nodiscard]] static const Config& GetInstance();

	Config(const Config&) = delete;
	Config(Config&&) = delete;
	Config& operator=(const Config&) = delete;
	Config& operator=(Config&&) = delete;

	std::size_t clientCountLimit{ 3 };
	std::size_t bufferCapacity{ 128 };
	bool isThreadCountAuto{ true };
	std::size_t threadCount{ 1 };
	CompressionLevel compressionLevel{ CompressionLevel::Default };

private:
	Config() = default;

	static Config s_instance;
};
