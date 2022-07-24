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

#include <new>
#include <memory>

template <typename T>
class PoolAllocator
{
public:
	using value_type = T;

	PoolAllocator() = default;

	template <typename U>
	PoolAllocator(const PoolAllocator<U>&)
	{
		// TODO
	}

	~PoolAllocator()
	{
		std::allocator<T>{}.deallocate(m_p, m_n);
	}

	void init(std::size_t n)
	{
		m_n = n;
		m_p = std::allocator<T>{}.allocate(n);
		m_next = m_p;

		for (std::size_t i{ 0 }; i != n; ++i)
		{
			// TODO: create freelist
		}
	}

	[[nodiscard]] T* allocate(std::size_t n)
	{
		if (m_n == 0)
		{
			throw std::bad_alloc{};
		}

		if (n != 1)
		{
			throw std::bad_array_new_length{};
		}

		T* p = m_next;

		// TODO: update m_next

		return p;
	}

	void deallocate(T* /*p*/, std::size_t n)
	{
		if (n != 1)
		{
			return;
		}

		// TODO: update freelist
	}

private:
	std::size_t m_n{};
	T* m_p{};
	T* m_next{};
};
