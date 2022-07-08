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

#include <sstream>
#include <utility>
#include <ios>
#include <map>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>

#include <spdlog/spdlog.h>

#include "Gzip.hpp"

static int s_compressionLevel{ boost::iostreams::gzip::default_compression };

namespace Gzip
{
	void SetCompressionLevel(CompressionLevel level)
	{
		static const std::map<CompressionLevel, int> mapping{
			{ CompressionLevel::None, boost::iostreams::gzip::no_compression },
			{ CompressionLevel::Speed, boost::iostreams::gzip::best_speed },
			{ CompressionLevel::Default, boost::iostreams::gzip::default_compression },
			{ CompressionLevel::Size, boost::iostreams::gzip::best_compression }
		};

		if (mapping.contains(level))
		{
			s_compressionLevel = mapping.at(level);
		}
		else
		{
			spdlog::error("GZIP compression level {} not mapped", std::to_underlying(level));
		}
	}

	[[nodiscard]] Result Compress(std::string_view data)
	{
		boost::iostreams::filtering_istreambuf buffer{};
		buffer.push(boost::iostreams::gzip_compressor{ s_compressionLevel });
		buffer.push(boost::iostreams::array_source{ data.data(), data.size() });

		std::stringstream stringStream{};

		try
		{
			boost::iostreams::copy(buffer, stringStream);
		}
		catch (const std::ios_base::failure& failure)
		{
			spdlog::error("Compression failed ({})", failure.what());
			return { {}, true };
		}

		return { std::move(stringStream).str(), false };
	}

	[[nodiscard]] Result Uncompress(std::string_view data)
	{
		boost::iostreams::filtering_istreambuf buffer{};
		buffer.push(boost::iostreams::gzip_decompressor{});
		buffer.push(boost::iostreams::array_source{ data.data(), data.size() });

		std::stringstream stringStream{};

		try
		{
			boost::iostreams::copy(buffer, stringStream);
		}
		catch (const std::ios_base::failure& failure)
		{
			spdlog::error("Decompression failed ({})", failure.what());
			return { {}, true };
		}

		return { std::move(stringStream).str(), false };
	}
}
