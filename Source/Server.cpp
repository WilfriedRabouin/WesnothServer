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

#include <utility>
#include <map>
#include <stdexcept>
#include <format>
#include <string>

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include "Server.hpp"
#include "ClientHandler.hpp"
#include "Config.hpp"
#include "Gzip.hpp"

static void Accept(boost::asio::ip::tcp::acceptor& acceptor)
{
	acceptor.async_accept(
		[&acceptor](const boost::system::error_code& error, boost::asio::ip::tcp::socket&& socket)
		{
			if (error)
			{
				spdlog::error("Failed to accept new connection: {}", error.message());
			}
			else
			{
				const std::string ipAddress{ socket.remote_endpoint().address().to_string() };

				if (ClientHandler::GetInstanceCountTotal() == Config::GetInstance().clientCountLimitTotal)
				{
					spdlog::warn("{}: connection refused (total limit reached)", ipAddress);
				}
				else if (ClientHandler::GetInstanceCountIpAddress(ipAddress) == Config::GetInstance().clientCountLimitIpAddress)
				{
					spdlog::warn("{}: connection refused (IP address limit reached)", ipAddress);
				}
				else
				{
					ClientHandler::Create(std::move(socket))->StartHandshake();
				}
			}

			Accept(acceptor);
		});
}

namespace Server
{
	void Run()
	{
		static const std::map<Config::CompressionLevel, Gzip::CompressionLevel> compressionLevelMapping
		{
			{ Config::CompressionLevel::None, Gzip::CompressionLevel::None },
			{ Config::CompressionLevel::Speed, Gzip::CompressionLevel::Speed },
			{ Config::CompressionLevel::Default, Gzip::CompressionLevel::Default },
			{ Config::CompressionLevel::Size, Gzip::CompressionLevel::Size }
		};

		const Config::CompressionLevel compressionLevel{ Config::GetInstance().compressionLevel };

		if (compressionLevelMapping.contains(compressionLevel))
		{
			Gzip::SetCompressionLevel(compressionLevelMapping.at(compressionLevel));
		}
		else
		{
			throw std::runtime_error{ std::format("Config compression level {} not mapped", std::to_underlying(compressionLevel)) };
		}

		constexpr boost::asio::ip::port_type port{ 15000 };
		const boost::asio::ip::tcp::endpoint endpoint{ boost::asio::ip::tcp::v4(), port };
		boost::asio::io_context ioContext{};
		boost::asio::ip::tcp::acceptor acceptor{ ioContext, endpoint };
		Accept(acceptor);
		ioContext.run();
	}
}
