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

#ifndef BMASHINA_DISABLE_DEBUG
#include <functional>
#include "bmashina/debug/propertyPrinter.hpp"
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

		void unset(const detail::BaseReference& reference);
		void clear();

		State& operator =(const State& other) = delete;

		static void copy(const State& source, State& destination);
		static void copy(
			const State& source, State& destination,
			const detail::BaseReference& reference);
		static void copy(
			const State& source, State& destination,
			const detail::BaseReference& source_reference,
			const detail::BaseReference& destination_reference);

#ifndef BMASHINA_DISABLE_DEBUG
		typedef typename String<M>::Type StringType;
		typedef std::function<void(const StringType& key, const StringType& value)> PropertyIter;
		void for_each_property(const PropertyIter& callback) const;
#endif

	private:
		Mashina* mashina;

		typedef typename Allocator<Mashina>::Type AllocatorType;
		AllocatorType allocator;

#ifndef BMASHINA_DISABLE_DEBUG
		typedef std::function<StringType(Mashina&, const State&, const detail::BaseReference*)> PropertyPrintFunc;
		typedef UnorderedMap<Mashina, const detail::BaseReference*, PropertyPrintFunc> ValuePrinterMap;
		typename ValuePrinterMap::Type value_printers;
#endif

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
void bmashina::BasicState<M>::unset(const detail::BaseReference& reference)
{
	remove_value(&reference);
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

#ifndef BMASHINA_DISABLE_DEBUG
	auto printer = [](Mashina& mashina, const State& state, const detail::BaseReference* reference)
	{
		auto value = state.values.find(reference);
		assert(value != state.values.end());

		auto property = static_cast<Property<typename R::Type>*>(value->second);
		return PropertyPrinter<Mashina, typename R::Type>::print(mashina, *property);
	};

	value_printers[&reference] = printer;
#endif

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
	State& destination)
{
	for (auto i: source.values)
	{
		copy(source, destination, *i.first);
	}
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

#ifndef BMASHINA_DISABLE_DEBUG
		auto printer = source.value_printers.find(&reference);
		assert(printer != source.value_printers.end());
		destination.value_printers[&reference] = printer->second;
#endif
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

#ifndef BMASHINA_DISABLE_DEBUG
		auto source_printer = source.value_printers.find(&source_reference);
		assert(source_printer != source.value_printers.end());
		destination.value_printers[&destination_reference] = source_printer->second;
#endif
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

#ifndef BMASHINA_DISABLE_DEBUG
		value_printers.erase(key);
#endif
	}
}

#ifndef BMASHINA_DISABLE_DEBUG
#include <cstdio>

template <typename M>
void bmashina::BasicState<M>::for_each_property(const PropertyIter& callback) const
{
	for (auto& i: values)
	{
		assert(value_printers.count(i.first) != 0);

		auto reference = i.first;
		auto& printer = value_printers.find(i.first)->second;
		auto value = printer(*mashina, *this, i.first);

		if (reference->name == nullptr)
		{
			char name[32];
			std::snprintf(name, sizeof(name), "%p", reference);

			callback(String<M>::construct(*mashina, name), value);
		}
		else
		{
			callback(String<M>::construct(*mashina, reference->name), value);
		}
	}
}
#endif

#endif
