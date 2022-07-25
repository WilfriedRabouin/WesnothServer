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
#include <type_traits>

#include <spdlog/spdlog.h>

#include "ClientHandler.hpp"
#include "Gzip.hpp"
#include "ProgramOptions.hpp"

using SizeField = std::uint32_t;

constexpr std::string_view versionMessage
{
	"[version]\n"
	"[/version]"
};

constexpr std::string_view mustloginMessage
{
	"[mustlogin]\n"
	"[/mustlogin]"
};

constexpr std::string_view joinLobbyMessage
{
	"[join_lobby]\n"
	"is_moderator=\"no\"\n"
	"profile_url_prefix=\"\"\n"
	"[/join_lobby]"
};

constexpr std::string_view gamelistMessage
{
	"[gamelist]\n"
	"[/gamelist]"
};

std::size_t ClientHandler::s_instanceCountTotal{};
std::unordered_map<std::string, std::size_t> ClientHandler::s_instanceCountIpAddress{};

PoolAllocator<ClientHandler> ClientHandler::s_allocator{};

[[nodiscard]] std::shared_ptr<ClientHandler> ClientHandler::Create(boost::asio::ip::tcp::socket&& socket)
{
	class EnableMakeShared : public ClientHandler
	{
	public:
		explicit EnableMakeShared(boost::asio::ip::tcp::socket&& socket) :
			ClientHandler{ std::move(socket) }
		{}
	};

	return std::allocate_shared<EnableMakeShared>(s_allocator, std::move(socket));
}

[[nodiscard]] std::size_t ClientHandler::GetInstanceCountTotal()
{
	return s_instanceCountTotal;
}

[[nodiscard]] std::size_t ClientHandler::GetInstanceCountIpAddress(const std::string& ipAddress)
{
	if (s_instanceCountIpAddress.contains(ipAddress))
	{
		return s_instanceCountIpAddress[ipAddress];
	}
	else
	{
		return 0;
	}
}

ClientHandler::~ClientHandler()
{
	spdlog::info("{} ({:x}) > disconnected", m_ipAddress, m_id);

	--s_instanceCountTotal;
	--s_instanceCountIpAddress[m_ipAddress];

	if (s_instanceCountIpAddress[m_ipAddress] == 0)
	{
		s_instanceCountIpAddress.erase(m_ipAddress);
	}
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
				spdlog::error("{} ({:x}) > handshake request failed ({})", m_ipAddress, m_id, error.message());
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
							spdlog::error("{} ({:x}) > handshake response failed ({})", m_ipAddress, m_id, error.message());
						}
						return;
					}

					spdlog::info("{} ({:x}) > handshake successful", m_ipAddress, m_id);
					StartLogin();
				});
		}
		else if (std::ranges::equal(m_readBuffer, tlsConnectionRequest))
		{
			spdlog::warn("{} ({:x}) > handshake request failed (TLS not implemented yet)", m_ipAddress, m_id);
			// TODO: implement TLS
		}
		else
		{
			spdlog::error("{} ({:x}) > handshake request failed (wrong data)", m_ipAddress, m_id);
		}
	});
}

ClientHandler::ClientHandler(boost::asio::ip::tcp::socket&& socket) :
	m_id{ reinterpret_cast<std::uintptr_t>(this) },
	m_ipAddress{ socket.remote_endpoint().address().to_string() },
	m_socket{ std::move(socket) }
{
	spdlog::info("{} ({:x}) > connected", m_ipAddress, m_id);

	++s_instanceCountTotal;
	++s_instanceCountIpAddress[m_ipAddress];

	const ProgramOptions::Config& config{ ProgramOptions::GetConfig() };
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
													spdlog::info("{} ({:x}) > login successful", m_ipAddress, m_id);

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
				spdlog::error("{} ({:x}) > receiving failed ({})", m_ipAddress, m_id, error.message());
			}
			return;
		}

		const SizeField sizeField = [this]
		{
			SizeField sizeField{};
			std::memcpy(&sizeField, m_readBuffer.data(), sizeof(SizeField));
			return sizeField;
		}();

		static_assert(std::endian::native == std::endian::little);
		const std::size_t size{ std::byteswap(sizeField) };

		if (size > m_readBuffer.capacity())
		{
			spdlog::error("{} ({:x}) > receiving {} bytes failed (buffer overflow)", m_ipAddress, m_id, size);
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
					spdlog::error("{} ({:x}) > receiving failed ({})", m_ipAddress, m_id, error.message());
				}
				return;
			}

			static_assert(std::is_same_v<unsigned char, std::uint8_t>);
			const std::string_view data{ reinterpret_cast<char*>(m_readBuffer.data()), m_readBuffer.size() };
			Gzip::Result result{ Gzip::Uncompress(data) };

			if (result.error)
			{
				spdlog::error("{} ({:x}) > receiving failed", m_ipAddress, m_id);
				return;
			}

			spdlog::debug("{} ({:x}) > receiving {} bytes\n{}", m_ipAddress, m_id, m_readBuffer.size() + sizeof(SizeField), result.data);
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
		spdlog::error("{} ({:x}) > sending failed", m_ipAddress, m_id);
		return;
	}

	const std::size_t size{ sizeof(SizeField) + result.data.size() };

	if (size > m_writeBuffer.capacity())
	{
		spdlog::error("{} ({:x}) > sending {} bytes failed (buffer overflow)", m_ipAddress, m_id, size);
		return;
	}

	m_writeBuffer.resize(size);

	static_assert(std::endian::native == std::endian::little);
	const SizeField sizeField{ std::byteswap(static_cast<SizeField>(result.data.size())) };
	std::memcpy(m_writeBuffer.data(), &sizeField, sizeof(SizeField));
	std::memcpy(m_writeBuffer.data() + sizeof(SizeField), result.data.data(), result.data.size());

	spdlog::debug("{} ({:x}) > sending {} bytes\n{}", m_ipAddress, m_id, m_writeBuffer.size(), message);

	boost::asio::async_write(m_socket, boost::asio::buffer(m_writeBuffer),
		[this, self = shared_from_this(), completionHandler = std::forward<CompletionHandler>(completionHandler)](const boost::system::error_code& error, std::size_t /*bytesTransferred*/)
	{
		if (error)
		{
			if (error != boost::asio::error::connection_reset)
			{
				spdlog::error("{} ({:x}) > sending failed ({})", m_ipAddress, m_id, error.message());
			}
			return;
		}

		completionHandler();
	});
}
