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
#include <vector>
#include <unordered_map>
#include <unordered_set>
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

	template <typename M>
	struct TreeAllocator
	{
		static_assert(
			detail::enable_default_allocator<M>::value,
			"BMASHINA_DISABLE_DEFAULT_ALLOCATOR: specialization for bmashina::TreeAllocator required");

#ifndef BMASHINA_DISABLE_DEFAULT_ALLOCATOR
		typedef BasicNode<M> Node;

		template <typename N, typename... Arguments>
		static N* allocate(M& mashina, Arguments&&... arguments);

		static void deallocate(M& mashina, Node* node);
#endif
	};
}

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
template <typename M, typename T>
typename bmashina::Vector<M, T>::Type bmashina::Vector<M, T>::construct(M& mashina)
{
	return Type();
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

#ifndef BMASHINA_DISABLE_DEFAULT_ALLOCATOR
template <typename M>
template <typename N, typename... Arguments>
N* bmashina::TreeAllocator<M>::allocate(M& mashina, Arguments&&... arguments)
{
	static_assert(
		std::is_base_of<Node, N>::value,
		"Node must be derived from bmashina::Node");

	return new N(std::forward<Arguments>(arguments)...);
}

template <typename M>
void bmashina::TreeAllocator<M>::deallocate(M& mashina, Node* node)
{
	delete node;
}
#endif

#endif
