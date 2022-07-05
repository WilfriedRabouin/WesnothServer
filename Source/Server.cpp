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

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include "Server.hpp"
#include "ClientHandler.hpp"
#include "Config.hpp"

void Accept(boost::asio::ip::tcp::acceptor& acceptor)
{
	acceptor.async_accept(
		[&acceptor](const boost::system::error_code& error, boost::asio::ip::tcp::socket socket)
		{
			const Config& config{ Config::GetInstance() };

			if (error)
			{
				spdlog::error("Failed to accept new connection: {}", error.message());
			}
			else if (config.isClientCountLimited && ClientHandler::GetInstanceCount() == config.clientCountLimit)
			{
				spdlog::warn("{}: connection refused (client count limit reached)", socket.remote_endpoint().address().to_string());
			}
			else
			{
				ClientHandler::create(std::move(socket))->StartHandshake();
			}

			Accept(acceptor);
		});
}

void RunServer()
{
	constexpr boost::asio::ip::port_type port{ 15000 };
	const boost::asio::ip::tcp::endpoint endpoint{ boost::asio::ip::tcp::v4(), port };
	boost::asio::io_context ioContext{};
	boost::asio::ip::tcp::acceptor acceptor{ ioContext, endpoint };
	Accept(acceptor);
	ioContext.run();
}
