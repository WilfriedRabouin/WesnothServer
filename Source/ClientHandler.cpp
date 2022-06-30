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

#include <array>
#include <utility>
#include <algorithm>

#include <spdlog/spdlog.h>

#include "ClientHandler.hpp"
#include "Gzip.hpp"

constexpr std::string_view versionMessage{
	"[version]\n"
	"[/version]"
};

constexpr std::string_view mustloginMessage{
	"[mustlogin]\n"
	"[/mustlogin]"
};

constexpr std::string_view joinLobbyMessage{
	"[join_lobby]\n"
	"is_moderator=\"no\"\n"
	"profile_url_prefix=\"\"\n"
	"[/join_lobby]"
};

constexpr std::string_view gamelistMessage{
	"[gamelist]\n"
	"[/gamelist]"
};

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
	spdlog::info("{}: disconnected", GetAddress());
	--s_instanceCount;
}

void ClientHandler::StartHandshake()
{
	constexpr std::size_t requestSize{ 4 };

	boost::asio::async_read(m_socket, boost::asio::dynamic_buffer(m_inputData, requestSize),
		[this, self = shared_from_this()](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
	{
		constexpr std::array<std::uint8_t, requestSize> normalConnection{ { 0, 0, 0, 0 } };
		constexpr std::array<std::uint8_t, requestSize> tlsConnection{ { 0, 0, 0, 1 } };

		if (error)
		{
			spdlog::error("{}: handshake request failed ({})", GetAddress(), error.message());
		}
		else if (std::ranges::equal(m_inputData, normalConnection))
		{
			m_outputData = { 0xDE, 0xAD, 0xBE, 0xEF };

			boost::asio::async_write(m_socket, boost::asio::buffer(m_outputData),
				[this, self](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
				{
					if (error)
					{
						spdlog::error("{}: handshake response failed ({})", GetAddress(), error.message());
					}
					else
					{
						spdlog::debug("{}: handshake successful", GetAddress());
						// TODO
					}
				});
		}
		else if (std::ranges::equal(m_inputData, tlsConnection))
		{
			spdlog::warn("{}: handshake request failed (TLS not implemented yet)", GetAddress());
			// TODO: implement TLS
		}
		else
		{
			spdlog::error("{}: handshake request failed (wrong data)", GetAddress());
		}
	});
}

ClientHandler::ClientHandler(boost::asio::ip::tcp::socket socket)
	: m_socket{ std::move(socket) }
{
	spdlog::info("{}: connected", GetAddress());
	++s_instanceCount;
}

[[nodiscard]] std::string ClientHandler::GetAddress() const
{
	return m_socket.remote_endpoint().address().to_string();
}

void ClientHandler::Receive()
{
	boost::asio::async_read(m_socket, boost::asio::dynamic_buffer(m_inputData, 4),
		[this, self = shared_from_this()](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
	{
		if (error)
		{
			spdlog::error("{}: receiving failed ({})", GetAddress(), error.message());
		}
		else
		{
			const std::size_t size{ static_cast<std::size_t>((m_inputData[0] << 24) | (m_inputData[1] << 16) | (m_inputData[2] << 8) | (m_inputData[3] << 0)) };

			spdlog::debug("Received {} bytes", size);

			boost::asio::async_read(m_socket, boost::asio::dynamic_buffer(m_inputData, size),
				[this, self](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
				{
					if (error)
					{
						spdlog::error("{}: receiving failed ({})", GetAddress(), error.message());
					}
					else
					{
						// TODO
						Receive();
					}
				});

		}
	});
}

void ClientHandler::Send(std::string_view message)
{
	const std::string data = Gzip::Compress(message);

	m_outputData.resize(data.size() + 4);
	m_outputData[0] = (data.size() >> 24) & 0xFF;
	m_outputData[1] = (data.size() >> 16) & 0xFF;
	m_outputData[2] = (data.size() >> 8) & 0xFF;
	m_outputData[3] = (data.size() >> 0) & 0xFF;
	std::ranges::copy_n(data.data(), data.size(), m_outputData.data() + 4);

	boost::asio::async_write(m_socket, boost::asio::buffer(m_outputData),
		[this, self = shared_from_this()](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
	{
		if (error)
		{
			spdlog::error("{}: sending failed ({})", GetAddress(), error.message());
		}
		else
		{
			spdlog::debug("{}: sending successful", GetAddress());
		}
	});
}
