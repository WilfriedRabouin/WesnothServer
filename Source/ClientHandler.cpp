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

#include <stdlib.h>
#include <array>
#include <utility>
#include <algorithm>
#include <cstring>
#include <ios>

#include <spdlog/spdlog.h>

#include "ClientHandler.hpp"
#include "Gzip.hpp"

using SizeField = std::uint32_t;

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
	spdlog::info("{}: disconnected", m_address);
	--s_instanceCount;
}

void ClientHandler::StartHandshake()
{
	constexpr std::size_t requestSize{ 4 };
	m_readData.resize(requestSize);

	boost::asio::async_read(m_socket, boost::asio::buffer(m_readData),
		[this, self = shared_from_this()](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
	{
		constexpr std::array<std::uint8_t, requestSize> normalConnectionRequest{ { 0, 0, 0, 0 } };
		constexpr std::array<std::uint8_t, requestSize> tlsConnectionRequest{ { 0, 0, 0, 1 } };

		if (error)
		{
			spdlog::error("{}: handshake request failed ({})", m_address, error.message());
		}
		else if (std::ranges::equal(m_readData, normalConnectionRequest))
		{
			m_writeData = { 0xDE, 0xAD, 0xBE, 0xEF };

			boost::asio::async_write(m_socket, boost::asio::buffer(m_writeData),
				[this, self](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
				{
					if (error)
					{
						spdlog::error("{}: handshake response failed ({})", m_address, error.message());
					}
					else
					{
						spdlog::info("{}: handshake successful", m_address);
						StartLogin();
					}
				});
		}
		else if (std::ranges::equal(m_readData, tlsConnectionRequest))
		{
			spdlog::warn("{}: handshake request failed (TLS not implemented yet)", m_address);
			// TODO: implement TLS
		}
		else
		{
			spdlog::error("{}: handshake request failed (wrong data)", m_address);
		}
	});
}

ClientHandler::ClientHandler(boost::asio::ip::tcp::socket socket)
	: m_address{ socket.remote_endpoint().address().to_string() }, m_socket{ std::move(socket) }
{
	spdlog::info("{}: connected", m_address);
	++s_instanceCount;
}

// WIP
void ClientHandler::StartLogin()
{
	Send(versionMessage,
		[this]
		{
			Receive([](std::string) {});
		});

	//spdlog::info("{}: login successful", m_address);
}

template <typename CompletionHandler>
void ClientHandler::Receive(CompletionHandler completionHandler)
{
	m_readData.resize(sizeof(SizeField));

	boost::asio::async_read(m_socket, boost::asio::buffer(m_readData),
		[this, self = shared_from_this(), completionHandler = std::move(completionHandler)](const boost::system::error_code& error, std::size_t /*bytesTransferred*/) mutable
	{
		if (error)
		{
			spdlog::error("{}: receiving failed ({})", m_address, error.message());
		}
		else
		{
			const SizeField sizeField{ *reinterpret_cast<SizeField*>(m_readData.data()) };
			const std::size_t dataSize{ _byteswap_ulong(sizeField) }; // TODO C++23: replace _byteswap_ulong with std::byteswap
			m_readData.resize(dataSize);

			boost::asio::async_read(m_socket, boost::asio::buffer(m_readData),
				[this, self, completionHandler = std::move(completionHandler)](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
			{
				if (error)
				{
					spdlog::error("{}: receiving failed ({})", m_address, error.message());
				}
				else
				{
					const std::string_view data{ /*reinterpret_cast<char*>(m_readData.data()), m_readData.size()*/ };

					try
					{
						std::string message{ Gzip::Uncompress(data) };
						spdlog::debug("{}: receiving {} bytes\n{}", m_address, m_readData.size() + sizeof(SizeField), message);
						completionHandler(std::move(message));
					}
					catch (const std::ios_base::failure& failure)
					{
						spdlog::error("{}: sending failed (decompression error: \"{}\")", m_address, failure.what());
					}
				}
			});
		}
	});
}

template <typename CompletionHandler>
void ClientHandler::Send(std::string_view message, CompletionHandler completionHandler)
{
	try
	{
		const std::string data{ Gzip::Compress(message) };
		const SizeField sizeField{ _byteswap_ulong(static_cast<SizeField>(data.size())) }; // TODO C++23: replace _byteswap_ulong with std::byteswap

		m_writeData.resize(sizeof(sizeField) + data.size());
		std::memcpy(m_writeData.data(), &sizeField, sizeof(sizeField));
		std::memcpy(m_writeData.data() + sizeof(sizeField), data.data(), data.size());
	}
	catch (const std::ios_base::failure& failure)
	{
		spdlog::error("{}: sending failed (compression error: \"{}\")", m_address, failure.what());
		return;
	}

	spdlog::debug("{}: sending {} bytes\n{}", m_address, m_writeData.size(), message);

	boost::asio::async_write(m_socket, boost::asio::buffer(m_writeData),
		[this, self = shared_from_this(), completionHandler = std::move(completionHandler)](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
	{
		if (error)
		{
			spdlog::error("{}: sending failed ({})", m_address, error.message());
		}
		else
		{
			completionHandler();
		}
	});
}
