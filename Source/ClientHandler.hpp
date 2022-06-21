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

#include <vector>
#include <memory>
#include <string>
#include <string_view>

#include <boost/asio.hpp>

class ClientHandler : public std::enable_shared_from_this<ClientHandler>
{
public:
	[[nodiscard]] static std::shared_ptr<ClientHandler> create(boost::asio::ip::tcp::socket socket);
	[[nodiscard]] static std::size_t GetInstanceCount();

	~ClientHandler();

	void StartHandshake();

private:
	explicit ClientHandler(boost::asio::ip::tcp::socket socket);

	[[nodiscard]] std::string GetAddressString() const;

	void Send(std::string_view message);
	void SendJoinLobbyMessage();
	void SendGamelistMessage();

	static std::size_t s_instanceCount;

	boost::asio::ip::tcp::socket m_socket;
	std::vector<char> m_inputData{};
	std::vector<char> m_outputData{};
};
