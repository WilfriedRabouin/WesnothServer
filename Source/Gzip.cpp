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
#include <utility> // std::move

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/copy.hpp>

#include "Gzip.hpp"

namespace Gzip
{
	[[nodiscard]] std::string Compress(const char* data, std::size_t size)
	{
		boost::iostreams::filtering_istreambuf buffer{};
		buffer.push(boost::iostreams::gzip_compressor{});
		buffer.push(boost::iostreams::array_source{ data, size });

		std::stringstream stringStream{};
		boost::iostreams::copy(buffer, stringStream);
		return std::move(stringStream).str();
	}

	[[nodiscard]] std::string Uncompress(const char* data, std::size_t size)
	{
		boost::iostreams::filtering_istreambuf buffer{};
		buffer.push(boost::iostreams::gzip_decompressor{});
		buffer.push(boost::iostreams::array_source{ data, size });

		std::stringstream stringStream{};
		boost::iostreams::copy(buffer, stringStream);
		return std::move(stringStream).str();
	}
}
