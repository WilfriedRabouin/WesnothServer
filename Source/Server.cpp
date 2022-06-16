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

// TODO: make it a recursive lambda once the deducing 'this' feature is available (C++23)
void AsyncAccept(boost::asio::ip::tcp::acceptor& acceptor, bool isClientCountLimited, std::size_t clientCountLimit)
{
	acceptor.async_accept(
		[&acceptor, isClientCountLimited, clientCountLimit](const boost::system::error_code& error, boost::asio::ip::tcp::socket socket)
		{
			if (error)
			{
				spdlog::error("Failed to accept new connection: {}", error.message());
			}
			else if (isClientCountLimited && ClientHandler::GetInstanceCount() == clientCountLimit)
			{
				spdlog::warn("{}: connection refused (client count limit reached)", socket.remote_endpoint().address().to_string());
			}
			else
			{
				ClientHandler clientHandler{ std::move(socket) };
				// TODO
			}
			AsyncAccept(acceptor, isClientCountLimited, clientCountLimit);
		});
}

void RunServer(bool isClientCountLimited, std::size_t clientCountLimit)
{
	const boost::asio::ip::tcp::endpoint endpoint{ boost::asio::ip::tcp::v4(), 15000 };
	boost::asio::io_context ioContext{};
	boost::asio::ip::tcp::acceptor acceptor{ ioContext, endpoint };
	AsyncAccept(acceptor, isClientCountLimited, clientCountLimit);
	ioContext.run();
}
