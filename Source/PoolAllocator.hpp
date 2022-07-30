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
	static_assert(sizeof(T) >= sizeof(T*));

	using value_type = T;

	PoolAllocator() = default;

	template <typename U>
	PoolAllocator(const PoolAllocator<U>&)
	{
		// TODO
	}

	~PoolAllocator()
	{
		std::allocator<T>{}.deallocate(m_pointer, m_count);
	}

	void Init(std::size_t count)
	{
		if (count == 0)
		{
			throw std::bad_alloc{};
		}

		m_count = count;
		m_pointer = std::allocator<T>{}.allocate(count);
		m_freeList = m_pointer;

		for (std::size_t i{ 0 }; i != count - 1; ++i)
		{
			const T* next{ m_pointer + i + 1 };
			std::memcpy(m_pointer + i, &next, sizeof(T*));
		}

		const T* next{ nullptr };
		std::memcpy(m_pointer + count - 1, &next, sizeof(T*));
	}

	[[nodiscard]] T* allocate(std::size_t count)
	{
		if (count != 1)
		{
			throw std::bad_array_new_length{};
		}

		if (m_freeList == nullptr)
		{
			throw std::bad_alloc{};
		}

		T* pointer{ m_freeList };
		std::memcpy(&m_freeList, pointer, sizeof(T*));
		return pointer;
	}

	void deallocate(T* pointer, std::size_t count)
	{
		if (pointer != nullptr && count == 1)
		{
			std::memcpy(pointer, &m_freeList, sizeof(T*));
			m_freeList = pointer;
		}
	}

private:
	std::size_t m_count{};
	T* m_pointer{};
	T* m_freeList{};
};
