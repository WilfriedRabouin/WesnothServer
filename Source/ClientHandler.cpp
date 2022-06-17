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

#include <spdlog/spdlog.h>

#include "ClientHandler.hpp"

std::size_t ClientHandler::s_instanceCount{};

[[nodiscard]] std::shared_ptr<ClientHandler> ClientHandler::create(boost::asio::ip::tcp::socket socket)
{
	return std::shared_ptr<ClientHandler>{ new ClientHandler{ std::move(socket) } };
}

[[nodiscard]] std::size_t ClientHandler::GetInstanceCount()
{
	return s_instanceCount;
}

ClientHandler::~ClientHandler()
{
	spdlog::info("{}: disconnected", m_socket.remote_endpoint().address().to_string());
	--s_instanceCount;
}

void ClientHandler::AsyncHandshake()
{
	m_receivedData.resize(4);

	boost::asio::async_read(m_socket, boost::asio::buffer(m_receivedData),
		[self = shared_from_this()](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
		{
			if (!error)
			{
				//m_sentData = { static_cast<char>(0xde), static_cast<char>(0xad), static_cast<char>(0xbe), static_cast<char>(0xef) };
				//boost::asio::async_write(m_socket, boost::asio::buffer(m_sentData),
				//	[this, clientHandler](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
				//	{
				//		if (!error)
				//		{
				//			//std::cout << "Handshake done\n";
				//			//AsyncRead();
				//		}
				//	});
			}
		});
}

ClientHandler::ClientHandler(boost::asio::ip::tcp::socket socket)
	: m_socket{ std::move(socket) }
{
	spdlog::info("{}: connected", m_socket.remote_endpoint().address().to_string());
	++s_instanceCount;
}
