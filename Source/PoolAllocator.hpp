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

//#include <memory>

// TODO
//using PoolAllocator = std::allocator<T>;

template <typename T>
class PoolAllocator
{
public:
	using value_type = T;

	[[nodiscard]] T* allocate(std::size_t /*n*/) { return nullptr; }
	void deallocate(T* /*p*/, std::size_t /*n*/) {}

private:


};