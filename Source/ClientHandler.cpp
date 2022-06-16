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

#include <iostream>
#include <utility>

#include "ClientHandler.hpp"

std::size_t ClientHandler::s_instanceCount{};

[[nodiscard]] std::size_t ClientHandler::GetInstanceCount()
{
	return s_instanceCount;
}

ClientHandler::ClientHandler(boost::asio::ip::tcp::socket socket)
	: m_socket{ std::move(socket) }
{
	std::cout << "[INFO] " << m_socket.remote_endpoint().address().to_string() << ": connected\n";
	++s_instanceCount;
}

ClientHandler::~ClientHandler()
{
	std::cout << "[INFO] " << m_socket.remote_endpoint().address().to_string() << ": disconnected\n";
	--s_instanceCount;
}
