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

#include <bit>
#include <array>
#include <utility>
#include <algorithm>
#include <cstring>

#include <spdlog/spdlog.h>

#include "ClientHandler.hpp"
#include "Gzip.hpp"
#include "Config.hpp"

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

[[nodiscard]] std::shared_ptr<ClientHandler> ClientHandler::Create(boost::asio::ip::tcp::socket&& socket)
{
	return std::shared_ptr<ClientHandler>{ new ClientHandler{ std::move(socket) } };
}

[[nodiscard]] std::size_t ClientHandler::GetInstanceCount()
{
	return s_instanceCount;
}

ClientHandler::~ClientHandler()
{
	spdlog::info("{}: disconnected", m_ipAddress);
	--s_instanceCount;
}

void ClientHandler::StartHandshake()
{
	constexpr std::size_t requestSize{ 4 };
	m_readBuffer.resize(requestSize);

	boost::asio::async_read(m_socket, boost::asio::buffer(m_readBuffer),
		[this, self = shared_from_this()](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
	{
		if (error)
		{
			if (error != boost::asio::error::connection_reset)
			{
				spdlog::error("{}: handshake request failed ({})", m_ipAddress, error.message());
			}
			return;
		}

		constexpr std::array<std::uint8_t, requestSize> normalConnectionRequest{ { 0, 0, 0, 0 } };
		constexpr std::array<std::uint8_t, requestSize> tlsConnectionRequest{ { 0, 0, 0, 1 } };

		if (std::ranges::equal(m_readBuffer, normalConnectionRequest))
		{
			m_writeBuffer = { 0xDE, 0xAD, 0xBE, 0xEF };

			boost::asio::async_write(m_socket, boost::asio::buffer(m_writeBuffer),
				[this, self](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
				{
					if (error)
					{
						if (error != boost::asio::error::connection_reset)
						{
							spdlog::error("{}: handshake response failed ({})", m_ipAddress, error.message());
						}
						return;
					}

					spdlog::info("{}: handshake successful", m_ipAddress);
					StartLogin();
				});
		}
		else if (std::ranges::equal(m_readBuffer, tlsConnectionRequest))
		{
			spdlog::warn("{}: handshake request failed (TLS not implemented yet)", m_ipAddress);
			// TODO: implement TLS
		}
		else
		{
			spdlog::error("{}: handshake request failed (wrong data)", m_ipAddress);
		}
	});
}

ClientHandler::ClientHandler(boost::asio::ip::tcp::socket&& socket)
	: m_ipAddress{ socket.remote_endpoint().address().to_string() }, m_socket{ std::move(socket) }
{
	spdlog::info("{}: connected", m_ipAddress);
	++s_instanceCount;

	const Config& config{ Config::GetInstance() };
	m_readBuffer.reserve(config.clientBufferCapacity);
	m_writeBuffer.reserve(config.clientBufferCapacity);
}

// WIP
void ClientHandler::StartLogin()
{
	Send(versionMessage,
		[this]
		{
			Receive(
				[this](std::string&& /*message*/)
				{
					// TODO: check client version

					Send(mustloginMessage,
						[this]
						{
							Receive(
								[this](std::string&& /*message*/)
								{
									// TODO: display username

									Send(joinLobbyMessage,
										[this]
										{
											Send(gamelistMessage,
												[this]
												{
													spdlog::info("{}: login successful", m_ipAddress);

													Receive(
														[this](std::string&& /*message*/)
														{
															// nothing
														});
												});
										});
								});
						});
				});
		});
}

template <typename CompletionHandler>
void ClientHandler::Receive(CompletionHandler&& completionHandler)
{
	m_readBuffer.resize(sizeof(SizeField));

	boost::asio::async_read(m_socket, boost::asio::buffer(m_readBuffer),
		[this, self = shared_from_this(), completionHandler = std::forward<CompletionHandler>(completionHandler)](const boost::system::error_code& error, std::size_t /*bytesTransferred*/) mutable
	{
		if (error)
		{
			if (error != boost::asio::error::connection_reset)
			{
				spdlog::error("{}: receiving failed ({})", m_ipAddress, error.message());
			}
			return;
		}

		const SizeField sizeField = [this]
		{
			SizeField sizeField{};
			std::memcpy(&sizeField, m_readBuffer.data(), sizeof(SizeField));
			return sizeField;
		}();

		const std::size_t size{ std::byteswap(sizeField) };

		if (size > m_readBuffer.capacity())
		{
			spdlog::error("{}: receiving {} bytes failed (buffer overflow)", m_ipAddress, size);
			return;
		}

		m_readBuffer.resize(size);

		boost::asio::async_read(m_socket, boost::asio::buffer(m_readBuffer),
			[this, self, completionHandler = std::move(completionHandler)](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
		{
			if (error)
			{
				if (error != boost::asio::error::connection_reset)
				{
					spdlog::error("{}: receiving failed ({})", m_ipAddress, error.message());
				}
				return;
			}

			const std::string_view data{ reinterpret_cast<char*>(m_readBuffer.data()), m_readBuffer.size() };
			Gzip::Result result{ Gzip::Uncompress(data) };

			if (result.error)
			{
				spdlog::error("{}: receiving failed", m_ipAddress);
				return;
			}

			spdlog::debug("{}: receiving {} bytes\n{}", m_ipAddress, m_readBuffer.size() + sizeof(SizeField), result.data);
			completionHandler(std::move(result).data);
		});
	});
}

template <typename CompletionHandler>
void ClientHandler::Send(std::string_view message, CompletionHandler&& completionHandler)
{
	const Gzip::Result result{ Gzip::Compress(message) };

	if (result.error)
	{
		spdlog::error("{}: sending failed", m_ipAddress);
		return;
	}

	const std::size_t size{ sizeof(SizeField) + result.data.size() };

	if (size > m_writeBuffer.capacity())
	{
		spdlog::error("{}: sending {} bytes failed (buffer overflow)", m_ipAddress, size);
		return;
	}

	m_writeBuffer.resize(size);

	const SizeField sizeField{ std::byteswap(static_cast<SizeField>(result.data.size())) };
	std::memcpy(m_writeBuffer.data(), &sizeField, sizeof(SizeField));
	std::memcpy(m_writeBuffer.data() + sizeof(SizeField), result.data.data(), result.data.size());

	spdlog::debug("{}: sending {} bytes\n{}", m_ipAddress, m_writeBuffer.size(), message);

	boost::asio::async_write(m_socket, boost::asio::buffer(m_writeBuffer),
		[this, self = shared_from_this(), completionHandler = std::forward<CompletionHandler>(completionHandler)](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
	{
		if (error)
		{
			if (error != boost::asio::error::connection_reset)
			{
				spdlog::error("{}: sending failed ({})", m_ipAddress, error.message());
			}
			return;
		}
		
		completionHandler();
	});
}
