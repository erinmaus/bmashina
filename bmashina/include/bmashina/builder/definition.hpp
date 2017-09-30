// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_BUILDER_DEFINITION_HPP
#define BMASHINA_BUILDER_DEFINITION_HPP

#include <cassert>
#include <tuple>
#include "bmashina/node.hpp"
#include "bmashina/tree.hpp"
#include "bmashina/state/reference.hpp"

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
#include <stdexcept>
#endif

namespace bmashina
{
	template <typename M, typename K>
	class BasicNodeDictionaryDefinition
	{
	public:
		typedef M Mashina;
		typedef K Key;
		typedef BasicTree<Mashina> Tree;
		typedef BasicNode<Mashina> Node;
		typedef BasicNodeDictionaryDefinition<Mashina, Key> Definition;

		BasicNodeDictionaryDefinition(Mashina& mashina);
		virtual ~BasicNodeDictionaryDefinition() = default;

		virtual Node& construct(Tree& tree) const = 0;
		virtual Node& construct(Tree& tree, Node& parent) const = 0;

		bool has_property(const Key& key) const;
		template <typename V>
		const Reference<V>& get_property(const Key& key) const;

	protected:
		void add_property(
			const Key& key,
			const detail::BaseReference* property);

	private:
		typedef UnorderedMap<Mashina, Key, const detail::BaseReference*> PropertyMap;
		typename PropertyMap::Type properties;
	};
}

template <typename M, typename K>
bmashina::BasicNodeDictionaryDefinition<M, K>::BasicNodeDictionaryDefinition(Mashina& mashina) :
	properties(PropertyMap::construct(mashina))
{
	// Nothing.
}

template <typename M, typename K>
bool bmashina::BasicNodeDictionaryDefinition<M, K>::has_property(const Key& key) const
{
	return properties.count(key) != 0;
}

template <typename M, typename K>
template <typename V>
const bmashina::Reference<V>&
bmashina::BasicNodeDictionaryDefinition<M, K>::get_property(const Key& key) const
{
	assert(has_property(key));

#ifdef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has_property(key))
	{
		throw std::runtime_error("definition does not have property");
	}
#endif

	auto& property_tag = properties.find(key)->second;
	auto reference = std::get<0>(property_tag);
	auto tag = std::get<1>(property_tag);

	assert(tag == &Reference<V>::TAG);

#ifdef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (tag != Reference<V>::TAG)
	{
		throw std::runtime_error("get type does not match property definition type");
	}
#endif

	return *(static_cast<const Reference<V>*>(reference));
}

template <typename M, typename K>
void bmashina::BasicNodeDictionaryDefinition<M, K>::add_property(
	const Key& key,
	const detail::BaseReference* property)
{
	assert(!has_property(key));

#ifdef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (has_property(key))
	{
		throw std::runtime_error("property already defined");
	}
#endif

	properties[key] = property;
}

#endif
