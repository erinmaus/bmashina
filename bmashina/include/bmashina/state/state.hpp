// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_STATE_STATE_HPP
#define BMASHINA_STATE_STATE_HPP

#include "bmashina/config.hpp"
#include "bmashina/state/property.hpp"
#include "bmashina/state/reference.hpp"

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
#include <stdexcept>
#endif

namespace bmashina
{
	template <typename M>
	class BasicState
	{
	public:
		typedef M Mashina;
		typedef BasicState<M> State;

		BasicState(Mashina& mashina);
		BasicState(const State& other) = delete;
		~BasicState();

		template <typename R>
		bool has(const R& reference) const;

		template <typename R>
		Property<typename R::Type> get(const R& reference) const;

		template <typename R>
		Property<typename R::Type> get(
			const R& reference,
			const typename R::Type& default_value) const;

		template <typename R>
		void set(const R& reference, const Property<typename R::Type>& value);

		template <typename R>
		void reserve(const R& reference);

		void wire(const detail::BaseReference& from, const detail::BaseReference& to);
		void snip(const detail::BaseReference& from);

		State& operator = (const State& other) = delete;

	private:
		Mashina* mashina;

		typedef typename Allocator<Mashina>::Type AllocatorType;
		AllocatorType allocator;

		typedef UnorderedMap<Mashina, const void*, detail::BaseProperty*> ValueMap;
		typename ValueMap::Type values;
		void remove_value(const void* key);

		typedef UnorderedMap<Mashina, const void*, detail::BaseReference::Tag> TagMap;
		typename TagMap::Type tags;

		typedef UnorderedMap<Mashina, const void*, const void*> AliasMap;
		typename AliasMap::Type aliases;
		const void* resolve_alias(const void* reference) const;
	};
}

template <typename M>
bmashina::BasicState<M>::BasicState(Mashina& mashina) :
	mashina(&mashina),
	allocator(mashina),
	values(ValueMap::construct(mashina)),
	tags(TagMap::construct(mashina)),
	aliases(AliasMap::construct(mashina))
{
	// Nothing.
}

template <typename M>
bmashina::BasicState<M>::~BasicState()
{
	for (auto& value: values)
	{
		BasicAllocator::destroy<detail::BaseProperty>(allocator, value.second);
	}
	values.clear();
	aliases.clear();
}

template <typename M>
template <typename R>
bool bmashina::BasicState<M>::has(const R& reference) const
{
	const void* key = resolve_alias(&reference);
	return values.count(key) != 0;
}

template <typename M>
template <typename R>
typename bmashina::Property<typename R::Type>
bmashina::BasicState<M>::get(const R& reference) const
{
	const void* key = resolve_alias(&reference);

	assert(values.count(key) != 0);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (values.count(key) == 0)
	{
		throw std::runtime_error("property not in state");
	}
#endif

	auto property = values.find(key)->second;
	return *(static_cast<Property<typename R::Type>*>(property));
}

template <typename M>
template <typename R>
bmashina::Property<typename R::Type>
bmashina::BasicState<M>::get(const R& reference, const typename R::Type& default_value) const
{
	const void* key = resolve_alias(&reference);

	auto value = values.find(key);
	if (value == values.end())
	{
		return default_value;
	}
	else
	{
		return *(static_cast<Property<typename R::Type>*>(value->second));
	}
}

template <typename M>
template <typename R>
void bmashina::BasicState<M>::reserve(const R& reference)
{
	const void* key = &reference;
	tags[key] = R::TAG;
}

template <typename M>
template <typename R>
void bmashina::BasicState<M>::set(const R& reference, const Property<typename R::Type>& value)
{
	auto key = resolve_alias(&reference);
	auto property = BasicAllocator::create<Property<typename R::Type>>(allocator, value);

	remove_value(key);
	auto iter = values.find(key);
	assert(values.count(key) == 0);
	
	values.emplace(key, property);

	reserve<R>(reference);
}

template <typename M>
void bmashina::BasicState<M>::wire(
	const detail::BaseReference& from,
	const detail::BaseReference& to)
{
	if (resolve_alias(&from) == &to)
	{
		return;
	}

	if (has(from))
	{
		remove_value(&to);
		assert(values.count(&to) == 0);

		values[&to] = values[&from]->clone(allocator);
		remove_value(&from);
	}

	aliases[&from] = &to;
}

template <typename M>
void bmashina::BasicState<M>::snip(const detail::BaseReference& from)
{
	aliases.erase(&from);
}

template <typename M>
void bmashina::BasicState<M>::remove_value(const void* key)
{
	auto iter = values.find(key);
	if (iter != values.end())
	{
		BasicAllocator::destroy<detail::BaseProperty>(allocator, iter->second);
		values.erase(iter);
	}
}

template <typename M>
const void* bmashina::BasicState<M>::resolve_alias(const void* reference) const
{
	auto alias = aliases.find(reference);
	if (alias == aliases.end())
	{
		return reference;
	}
	else
	{
		return alias->second;
	}
}

#endif
