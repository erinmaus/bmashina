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
		void wire(const R& from, const R& to);

		State& operator = (const State& other) = delete;

	private:
		Mashina* mashina;

		typedef typename Allocator<Mashina>::Type AllocatorType;
		AllocatorType allocator;

		typedef UnorderedMap<Mashina, const void*, detail::BaseProperty*> ValueMap;
		typename ValueMap::Type values;

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

	auto property = values[key];
	return *(static_cast<Property<typename R::Type>>(property));
}

template <typename M>
template <typename R>
bmashina::Property<typename R::Type>
bmashina::BasicState<M>::get(const R& reference, const typename R::Type& default_value) const
{
	const void* key = resolve_alias(&reference);

	auto value = values.find(key);
	if (value == values.end(key))
	{
		return default_value;
	}
	else
	{
		return *(static_cast<Property<typename R::Type>>(value->second));
	}
}

template <typename M>
template <typename R>
void bmashina::BasicState<M>::set(const R& reference, const Property<typename R::Type>& value)
{
	auto key = resolve_alias(&reference);
	auto property = BasicAllocator::create<Property<typename R::Type>>(value);

	auto iter = values.find(key);
	if (iter == values.end())
	{
		values.insert(key, property);
	}
	else
	{
		BasicAllocator::destroy<detail::BaseProperty>(allocator, iter->second);
		iter->second = property;
	}
}

template <typename M>
template <typename R>
void bmashina::BasicState<M>::wire(const R& from, const R& to)
{
	if (has(from))
	{
		set(to, get(from));
	}

	aliases[&from] = &to;
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
