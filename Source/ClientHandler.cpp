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
#include <algorithm>

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
	boost::asio::async_read(m_socket, boost::asio::dynamic_buffer(m_inputData, 4),
		[this, self = shared_from_this()](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
		{
			if (error)
			{
				spdlog::error("{}: handshake failed ({})", m_socket.remote_endpoint().address().to_string(), error.message());
			}
			else
			{
				// TODO: handle TLS
				constexpr std::array<char, 4> expectedData{};
				if (std::ranges::equal(m_inputData, expectedData))
				{
					m_outputData = { '\xde', '\xad', '\xbe', '\xef' };
					boost::asio::async_write(m_socket, boost::asio::buffer(m_outputData),
						[this, self](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
						{
							if (error)
							{
								spdlog::error("{}: handshake failed ({})", m_socket.remote_endpoint().address().to_string(), error.message());
							}
							else
							{
								spdlog::info("{}: handshake done", m_socket.remote_endpoint().address().to_string());
								// TODO
							}
						});
				}
				else
				{
					spdlog::error("{}: handshake failed (wrong data received from client)", m_socket.remote_endpoint().address().to_string());
				}
			}
		});
}

ClientHandler::ClientHandler(boost::asio::ip::tcp::socket socket)
	: m_socket{ std::move(socket) }
{
	spdlog::info("{}: connected", m_socket.remote_endpoint().address().to_string());
	++s_instanceCount;
}
