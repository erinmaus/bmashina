// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_BUILDER_DICTIONARY_HPP
#define BMASHINA_BUILDER_DICTIONARY_HPP

#include <algorithm>
#include "bmashina/config.hpp"
#include "bmashina/node.hpp"
#include "bmashina/tree.hpp"
#include "bmashina/builder/definition.hpp"

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
#include <stdexcept>
#endif

namespace bmashina
{
	template <typename M, typename K, typename D>
	class BasicNodeDictionary
	{
	public:
		typedef M Mashina;
		typedef K Key;
		typedef D Definition;

		BasicNodeDictionary(Mashina& mashina);

		bool has(const Key& key) const;
		const Definition& get(const Key& key) const;

	protected:
		Mashina& mashina();

		// XXX: 'get' on a non-const BasicNodeDictionary would resolves to this
		// method if it were also named 'get', so we have to name it something
		// else.
		Definition& get_mutable(const Key& key);
		void add(const Key& key, Definition& definition);

	private:
		Mashina* mashina_instance;

		typedef UnorderedMap<Mashina, Key, Definition> DefinitionMap;
		typename DefinitionMap::Type definitions;
	};
}

template <typename M, typename K, typename D>
bmashina::BasicNodeDictionary<M, K, D>::BasicNodeDictionary(Mashina& mashina) :
	mashina_instance(&mashina),
	definitions(DefinitionMap::construct(mashina))
{
	// Nothing.
}

template <typename M, typename K, typename D>
bool bmashina::BasicNodeDictionary<M, K, D>::has(const Key& key) const
{
	return definitions.count(key) != 0;
}

template <typename M, typename K, typename D>
const typename bmashina::BasicNodeDictionary<M, K, D>::Definition&
bmashina::BasicNodeDictionary<M, K, D>::get(const Key& key) const
{
	assert(has(key));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(key))
	{
		throw std::runtime_error("key not in dictionary");
	}
#endif

	return definitions.find(key)->second;
}

template <typename M, typename K, typename D>
typename bmashina::BasicNodeDictionary<M, K, D>::Mashina&
bmashina::BasicNodeDictionary<M, K, D>::mashina()
{
	return *mashina_instance;
}

template <typename M, typename K, typename D>
typename bmashina::BasicNodeDictionary<M, K, D>::Definition&
bmashina::BasicNodeDictionary<M, K, D>::get_mutable(const Key& key)
{
	assert(has(key));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(key))
	{
		throw std::runtime_error("key not in dictionary");
	}
#endif

	return definitions.find(key)->second;
}

template <typename M, typename K, typename D>
void bmashina::BasicNodeDictionary<M, K, D>::add(
	const Key& key,
	Definition& definition)
{
	assert(!has(key));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (has(key))
	{
		throw std::runtime_error("dictionary already has key");
	}
#endif

	definitions.emplace(key, std::move(definition));
}

#endif
