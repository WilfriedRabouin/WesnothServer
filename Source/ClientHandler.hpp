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
#include <vector>
#include <memory>
#include <string>
#include <string_view>

#include <boost/asio.hpp>

class ClientHandler : public std::enable_shared_from_this<ClientHandler>
{
public:
	[[nodiscard]] static std::shared_ptr<ClientHandler> create(boost::asio::ip::tcp::socket&& socket);
	[[nodiscard]] static std::size_t GetInstanceCount();

	ClientHandler(const ClientHandler&) = delete;
	ClientHandler(ClientHandler&&) = delete;
	ClientHandler& operator=(const ClientHandler&) = delete;
	ClientHandler& operator=(ClientHandler&&) = delete;

	~ClientHandler();

	void StartHandshake();

private:
	explicit ClientHandler(boost::asio::ip::tcp::socket&& socket);

	void StartLogin();

	template <typename CompletionHandler>
	void Receive(CompletionHandler&& completionHandler);

	template <typename CompletionHandler>
	void Send(std::string_view message, CompletionHandler&& completionHandler);

	static std::size_t s_instanceCount;

	const std::string m_address{};
	boost::asio::ip::tcp::socket m_socket;
	std::vector<std::uint8_t> m_readBuffer{};
	std::vector<std::uint8_t> m_writeBuffer{};
};
