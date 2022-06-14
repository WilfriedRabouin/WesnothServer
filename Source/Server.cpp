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

#include <boost/asio.hpp>
#include "Server.hpp"

void AsyncAccept(boost::asio::ip::tcp::acceptor& acceptor)
{
	acceptor.async_accept(
		[&acceptor](const boost::system::error_code& error, boost::asio::ip::tcp::socket socket)
		{
			if (!error)
			{
				// TODO
			}
			AsyncAccept(acceptor);
		});
}

void RunServer()
{
	const boost::asio::ip::tcp::endpoint endpoint{ boost::asio::ip::tcp::v4(), 15000 };
	boost::asio::io_context ioContext{};
	boost::asio::ip::tcp::acceptor acceptor{ ioContext, endpoint };
	AsyncAccept(acceptor);
	ioContext.run();
}
