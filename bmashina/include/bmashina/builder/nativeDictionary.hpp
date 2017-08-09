// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_BUILDER_NATIVE_DICTIONARY_HPP
#define BMASHINA_BUILDER_NATIVE_DICTIONARY_HPP

#include "bmashina/config.hpp"
#include "bmashina/node.hpp"
#include "bmashina/tree.hpp"
#include "bmashina/builder/dictionary.hpp"
#include "bmashina/builder/nativeDefinition.hpp"

namespace bmashina
{
	template <typename M, typename K = typename String<M>::Type>
	class NativeNodeDictionary :
		public BasicNodeDictionary<M, K, NativeNodeDictionaryDefinition<M, K>>
	{
	public:
		typedef M Mashina;
		typedef K Key;
		typedef NativeNodeDictionaryDefinition<M, K> Definition;
		typedef BasicTree<Mashina> Tree;
		typedef BasicNode<Mashina> Node;

		NativeNodeDictionary(Mashina& mashina);

		template <typename N>
		Definition& define(const Key& key);
	};
}

template <typename M, typename K>
bmashina::NativeNodeDictionary<M, K>::NativeNodeDictionary(Mashina& mashina) :
	BasicNodeDictionary<Mashina, Key, Definition>(mashina)
{
	// Nothing.
}

template <typename M, typename K>
template <typename N>
typename bmashina::NativeNodeDictionary<M, K>::Definition&
bmashina::NativeNodeDictionary<M, K>::define(const Key& key)
{
	if (!this->has(key))
	{
		auto root = [](Tree& tree) -> Node&
		{
			return tree.template root<N>();
		};
		auto child = [](Tree& tree, Node& parent) -> Node&
		{
			return tree.template child<N>(parent);
		};

		Definition definition(this->mashina(), root, child);
		this->add(key, definition);
	}

	return this->get_mutable(key);
}

#endif
