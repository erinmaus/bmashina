// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_CONFIG_HPP
#define BMASHINA_CONFIG_HPP

#include <type_traits>

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#endif

namespace bmashina
{
	template <typename M>
	class Node;

	namespace detail
	{
#ifdef BMASHINA_DISABLE_STL_CONTAINERS
		template <typename M>
		struct enable_stl_containers : std::false_type
		{
			// Nothing.
		};
#else
		template <typename M>
		struct enable_stl_containers : std::true_type
		{
			// nothing.
		};
#endif

#ifdef BMASHINA_DISABLE_DEFAULT_ALLOCATOR
		template <typename M>
		struct enable_default_allocator : std::false_type
		{
			// Nothing.
		};
#else
		template <typename M>
		struct enable_default_allocator : std::true_type
		{
			// nothing.
		};
#endif
	}

	template <typename M, typename T>
	struct Vector
	{
		static_assert(
			detail::enable_stl_containers<M>::value,
			"BMASHINA_DISABLE_STL_CONTAINERS: specialization for bmashina::Vector required");

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
		typedef typename std::vector<T> Type;
		static Type construct(M& mashina);
#endif
	};

	template <typename M>
	struct String
	{
		static_assert(
			detail::enable_stl_containers<M>::value,
			"BMASHINA_DISABLE_STL_CONTAINERS: specialization for bmashina::String required");

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
		typedef typename std::string Type;
		static Type construct(M& mashina);
		static Type construct(M& mashina, const char* value);
#endif
	};

	template <typename M, typename V>
	struct ToString
	{
		static_assert(
			detail::enable_stl_containers<M>::value,
			"BMASHINA_DISABLE_STL_CONTAINERS: specialization for bmashina::String required");

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
		static typename String<M>::Type get(M& mashina, const V& value);
#endif
	};

	template <typename M, typename T>
	struct UnorderedSet
	{
		static_assert(
			detail::enable_stl_containers<M>::value,
			"BMASHINA_DISABLE_STL_CONTAINERS: specialization for bmashina::UnorderedSet required");

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
		typedef typename std::unordered_set<T> Type;
		static Type construct(M& mashina);
#endif
	};

	template <typename M, typename K, typename V>
	struct UnorderedMap
	{
		static_assert(
			detail::enable_stl_containers<M>::value,
			"BMASHINA_DISABLE_STL_CONTAINERS: specialization for bmashina::UnorderedMap required");

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
		typedef typename std::unordered_map<K, V> Type;
		static Type construct(M& mashina);
#endif
	};

	struct BasicAllocator
	{
		virtual void* allocate(std::size_t num_bytes) = 0;
		virtual void deallocate(void* value) = 0;

		template <typename T, typename... Arguments>
		static T* create(BasicAllocator& allocator, Arguments&&... arguments);

		template <typename T>
		static void destroy(BasicAllocator& allocator, T* value);
	};

	template <typename M>
	struct Allocator
	{
		static_assert(
			detail::enable_default_allocator<M>::value,
			"BMASHINA_DISABLE_DEFAULT_ALLOCATOR: specialization for bmashina::Allocator required");

#ifndef BMASHINA_DISABLE_DEFAULT_ALLOCATOR
		struct Type : public BasicAllocator
		{
			Type(M& mashina);
			void* allocate(std::size_t num_bytes) override;
			void deallocate(void* value) override;
		};
#endif
	};
}

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
template <typename M, typename T>
typename bmashina::Vector<M, T>::Type bmashina::Vector<M, T>::construct(M& mashina)
{
	return Type();
}

template <typename M>
typename bmashina::String<M>::Type bmashina::String<M>::construct(M& mashina)
{
	return Type();
}

template <typename M>
typename bmashina::String<M>::Type
bmashina::String<M>::construct(M& mashina, const char* value)
{
	if (value == nullptr)
	{
		return Type();
	}
	else
	{
		return Type(value);
	}
}

#include <sstream>
template <typename M, typename V>
typename bmashina::String<M>::Type
bmashina::ToString<M, V>::get(M& mashina, const V& value)
{
	std::stringstream stream;
	stream << value;
	return stream.str();
}

template <typename M, typename T>
typename bmashina::UnorderedSet<M, T>::Type bmashina::UnorderedSet<M, T>::construct(M& mashina)
{
	return Type();
}

template <typename M, typename K, typename V>
typename bmashina::UnorderedMap<M, K, V>::Type bmashina::UnorderedMap<M, K, V>::construct(M& mashina)
{
	return Type();
}
#endif

template <typename T, typename... Arguments>
T* bmashina::BasicAllocator::create(BasicAllocator& allocator, Arguments&&... arguments)
{
	return new(allocator.allocate(sizeof(T))) T(std::forward<Arguments>(arguments)...);
}

template <typename T>
void bmashina::BasicAllocator::destroy(BasicAllocator& allocator, T* value)
{
	assert(value != nullptr);

	value->~T();
	allocator.deallocate(value);
}

#ifndef BMASHINA_DISABLE_DEFAULT_ALLOCATOR
#include <cstdlib>

template <typename M>
bmashina::Allocator<M>::Type::Type(M& m)
{
	// Nothing.
}

template <typename M>
void* bmashina::Allocator<M>::Type::allocate(std::size_t num_bytes)
{
	return std::malloc(num_bytes);
}

template <typename M>
void bmashina::Allocator<M>::Type::deallocate(void* value)
{
	return std::free(value);
}

#endif

#endif
