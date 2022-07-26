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
#include <cstring>

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
		std::allocator<T>{}.deallocate(m_objectList, m_objectCount);
	}

	void Init(std::size_t count)
	{
		if (n == 0)
		{
			throw std::bad_alloc{};
		}

		m_n = n;
		m_p = std::allocator<T>{}.allocate(n);
		m_next = m_p;

		for (std::size_t i{ 0 }; i != n - 1; ++i)
		{
			const T* next{ m_p + i + 1 };
			std::memcpy(m_p + i, &next, sizeof(T*));
		}

		const T* next{ nullptr };
		std::memcpy(m_p + n - 1, &next, sizeof(T*));
	}

	[[nodiscard]] T* allocate(std::size_t n)
	{
		if (n != 1)
		{
			throw std::bad_array_new_length{};
		}

		if (m_next == nullptr)
		{
			throw std::bad_alloc{};
		}

		T* p{ m_next };
		std::memcpy(&m_next, m_next, sizeof(T*));
		return p;
	}

	void deallocate(T* p, std::size_t n)
	{
		if (p != nullptr && n == 1)
		{
			std::memcpy(p, &m_next, sizeof(T*));
			m_next = p;
		}
	}

private:
	std::size_t m_objectCount{};
	T* m_objectList{};
	T* m_freeListHead{};
};
