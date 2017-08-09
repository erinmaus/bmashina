// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_BUILDER_NATIVE_DEFINITION_HPP
#define BMASHINA_BUILDER_NATIVE_DEFINITION_HPP

#include <functional>
#include "bmashina/config.hpp"
#include "bmashina/node.hpp"
#include "bmashina/tree.hpp"
#include "bmashina/builder/definition.hpp"

namespace bmashina
{
	template <typename M, typename K = typename String<M>::Type>
	class NativeNodeDictionaryDefinition :
		public BasicNodeDictionaryDefinition<M, K>
	{
	public:
		typedef M Mashina;
		typedef K Key;
		typedef BasicTree<Mashina> Tree;
		typedef BasicNode<Mashina> Node;
		typedef std::function<Node&(Tree&)> RootConstructor;
		typedef std::function<Node&(Tree&, Node&)> ChildConstructor;

		NativeNodeDictionaryDefinition(
			Mashina& mashina,
			const RootConstructor& root,
			const ChildConstructor& child);
		~NativeNodeDictionaryDefinition() = default;

		Node& construct(Tree& tree) const override;
		Node& construct(Tree& tree, Node& parent) const override;

		template <typename R>
		NativeNodeDictionaryDefinition& property(const Key& key, const R& reference);

	private:
		RootConstructor root;
		ChildConstructor child;
	};
}

template <typename M, typename K>
bmashina::NativeNodeDictionaryDefinition<M, K>::NativeNodeDictionaryDefinition(
	Mashina& mashina,
	const RootConstructor& root,
	const ChildConstructor& child) :
	bmashina::BasicNodeDictionaryDefinition<M, K>(mashina),
	root(root),
	child(child)
{
	// Nothing.
}

template <typename M, typename K>
typename bmashina::NativeNodeDictionaryDefinition<M, K>::Node&
bmashina::NativeNodeDictionaryDefinition<M, K>::construct(Tree& tree) const
{
	return root(tree);
}

template <typename M, typename K>
typename bmashina::NativeNodeDictionaryDefinition<M, K>::Node&
bmashina::NativeNodeDictionaryDefinition<M, K>::construct(Tree& tree, Node& parent) const
{
	return child(tree, parent);
}

template <typename M, typename K>
template <typename R>
bmashina::NativeNodeDictionaryDefinition<M, K>&
bmashina::NativeNodeDictionaryDefinition<M, K>::property(const Key& key, const R& reference)
{
	this->add_property(key, &R::TAG, &reference);
	return *this;
}

#endif
