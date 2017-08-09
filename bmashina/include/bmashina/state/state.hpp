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

		bool has(const detail::BaseReference& reference) const;

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

		void clear();

		State& operator =(const State& other) = delete;

		static void copy(
			const State& source, State& destination,
			const detail::BaseReference& reference);
		static void copy(
			const State& source, State& destination,
			const detail::BaseReference& source_reference,
			const detail::BaseReference& destination_reference);

	private:
		Mashina* mashina;

		typedef typename Allocator<Mashina>::Type AllocatorType;
		AllocatorType allocator;

		typedef UnorderedMap<Mashina, const detail::BaseReference*, detail::BaseProperty*> ValueMap;
		typename ValueMap::Type values;
		void remove_value(const detail::BaseReference* key);

		typedef UnorderedMap<Mashina, const detail::BaseReference*, detail::BaseReference::Tag> TagMap;
		typename TagMap::Type tags;

		typedef UnorderedMap<Mashina, const detail::BaseReference*, const detail::BaseReference*> AliasMap;
		typename AliasMap::Type aliases;
		const detail::BaseReference* resolve_alias(const detail::BaseReference* reference) const;
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
	clear();
}

template <typename M>
bool bmashina::BasicState<M>::has(const detail::BaseReference& reference) const
{
	return values.count(&reference) != 0;
}

template <typename M>
template <typename R>
typename bmashina::Property<typename R::Type>
bmashina::BasicState<M>::get(const R& reference) const
{
	assert(values.count(&reference) != 0);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (values.count(&reference) == 0)
	{
		throw std::runtime_error("property not in state");
	}
#endif

	auto property = values.find(&reference)->second;
	return *(static_cast<Property<typename R::Type>*>(property));
}

template <typename M>
template <typename R>
bmashina::Property<typename R::Type>
bmashina::BasicState<M>::get(const R& reference, const typename R::Type& default_value) const
{
	auto value = values.find(&reference);
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
	tags[&reference] = R::TAG;
}

template <typename M>
template <typename R>
void bmashina::BasicState<M>::set(const R& reference, const Property<typename R::Type>& value)
{
	remove_value(&reference);
	auto iter = values.find(&reference);
	assert(values.count(&reference) == 0);

	auto property = BasicAllocator::create<Property<typename R::Type>>(allocator, value);
	values.emplace(&reference, property);

	reserve<R>(reference);
}

template <typename M>
void bmashina::BasicState<M>::clear()
{
	for (auto& value: values)
	{
		BasicAllocator::destroy<detail::BaseProperty>(allocator, value.second);
	}
	values.clear();
	tags.clear();
}

template <typename M>
void bmashina::BasicState<M>::copy(
	const State& source,
	State& destination,
	const detail::BaseReference& reference)
{
	if (&source == &destination)
	{
		return;
	}

	if (source.has(reference))
	{
		auto value = source.values.find(&reference);
		auto tag = source.tags.find(&reference);

		assert(value != source.values.end());
		assert(tag != source.tags.end());

		destination.remove_value(&reference);
		destination.values[&reference] = value->second->clone(destination.allocator);
		destination.tags[&reference] = tag->second;
	}
}

template <typename M>
void bmashina::BasicState<M>::copy(
	const State& source,
	State& destination,
	const detail::BaseReference& source_reference,
	const detail::BaseReference& destination_reference)
{
	if (&source == &destination && &source_reference == &destination_reference)
	{
		return;
	}

	if (source.has(source_reference))
	{
		auto source_value = source.values.find(&source_reference);
		auto source_tag = source.tags.find(&source_reference);

		assert(source_value != source.values.end());
		assert(source_tag != source.tags.end());

		destination.remove_value(&destination_reference);
		destination.values[&destination_reference] = source_value->second->clone(destination.allocator);
		destination.tags[&destination_reference] = source_tag->second;
	}
}

template <typename M>
void bmashina::BasicState<M>::remove_value(const detail::BaseReference* key)
{
	auto iter = values.find(key);
	if (iter != values.end())
	{
		BasicAllocator::destroy<detail::BaseProperty>(allocator, iter->second);
		values.erase(iter);

		assert(tags.count(key) != 0);
		tags.erase(key);
	}
}

#endif
