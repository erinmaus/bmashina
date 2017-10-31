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
	class BasicTree;

	template <typename M>
	class BasicState
	{
	public:
		typedef M Mashina;
		typedef BasicState<M> State;
		typedef BasicTree<M> Tree;

		BasicState(Mashina& mashina);
		BasicState(const State& other) = delete;
		~BasicState();

		bool has(const detail::BaseReference& reference) const;

		template <typename R>
		typename R::Type get(const R& reference) const;

		template <typename R>
		typename R::Type get(
			const R& reference,
			const typename R::Type& default_value) const;

		template <typename R>
		void set(const R& reference, const Property<typename R::Type>& value);

		void reserve(const detail::BaseReference& reference);

		void unset(const detail::BaseReference& reference);
		void clear();

		State& operator =(const State& other) = delete;

		void set_locals_key(const void* key);
		void invalidate_locals(const void* key);

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

		const void* current_locals_key = nullptr;
		typedef UnorderedSet<Mashina, const detail::BaseReference*> LocalSet;
		typedef UnorderedMap<Mashina, const void*, typename LocalSet::Type> LocalMap;
		typename LocalMap::Type locals_by_key;
		typename LocalSet::Type locals;

		typedef UnorderedMap<Mashina, const detail::BaseReference*, detail::BaseProperty*> ValueMap;
		typename ValueMap::Type values;

		template <typename V>
		void set_value(const Reference<V>& reference, const Property<V>& value);

		template <typename V>
		void set_value(const Local<V>& local, const Property<V>& value);

		void remove_value(const detail::BaseReference* key);
	};
}

template <typename M>
bmashina::BasicState<M>::BasicState(Mashina& mashina) :
	mashina(&mashina),
	allocator(mashina),
	locals_by_key(LocalMap::construct(mashina)),
	locals(LocalSet::construct(mashina)),
	values(ValueMap::construct(mashina))
{
	set_locals_key(nullptr);
}

template <typename M>
bmashina::BasicState<M>::~BasicState()
{
	clear();
}

template <typename M>
bool bmashina::BasicState<M>::has(const detail::BaseReference& reference) const
{
	auto iter = values.find(&reference);
	if (iter == values.end() || iter->second == nullptr)
	{
		return false;
	}

	return true;
}

template <typename M>
template <typename R>
typename R::Type
bmashina::BasicState<M>::get(const R& reference) const
{
	auto iter = values.find(&reference);

	assert(iter != values.end());
	assert(iter->second != nullptr);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (iter == values.end())
	{
		throw std::runtime_error("property not in state");
	}

	if (iter->second == nullptr)
	{
		throw std::runtime_error("property no longer in state");
	}
#endif

	return (static_cast<Property<typename R::Type>*>(iter->second))->get();
}

template <typename M>
template <typename R>
typename R::Type
bmashina::BasicState<M>::get(const R& reference, const typename R::Type& default_value) const
{
	auto value = values.find(&reference);
	if (value == values.end() || value->second == nullptr)
	{
		return default_value;
	}
	else
	{
		return (static_cast<Property<typename R::Type>*>(value->second))->get();
	}
}

template <typename M>
void bmashina::BasicState<M>::reserve(const detail::BaseReference& reference)
{
	if (values.count(&reference) == 0)
	{
		values[&reference] = nullptr;
	}
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
	set_value<typename R::Type>(reference, value);
}

template <typename M>
template <typename V>
void bmashina::BasicState<M>::set_value(const Reference<V>& reference, const Property<V>& value)
{
	remove_value(&reference);

	auto property = BasicAllocator::create<Property<V>>(allocator, value);
	values[&reference] = property;

#ifndef BMASHINA_DISABLE_DEBUG
	auto printer = [](Mashina& mashina, const State& state, const detail::BaseReference* reference)
	{
		auto value = state.values.find(reference);
		assert(value != state.values.end());

		auto property = static_cast<Property<V>*>(value->second);
		return PropertyPrinter<Mashina, V>::print(mashina, *property);
	};

	value_printers[&reference] = printer;
#endif
}

template <typename M>
template <typename V>
void bmashina::BasicState<M>::set_value(const Local<V>& local, const Property<V>& value)
{
	remove_value(&local);

	auto property = BasicAllocator::create<Property<V>>(allocator, value);
	values[&local] = property;
	locals_by_key[current_locals_key].insert(&local);
	locals.insert(&local);

#ifndef BMASHINA_DISABLE_DEBUG
	auto printer = [](Mashina& mashina, const State& state, const detail::BaseReference* local)
	{
		auto value = state.values.find(local);
		assert(value != state.values.end());

		auto property = static_cast<Property<V>*>(value->second);
		return PropertyPrinter<Mashina, V>::print(mashina, *property);
	};

	value_printers[&local] = printer;
#endif
}

template <typename M>
void bmashina::BasicState<M>::clear()
{
	for (auto& value: values)
	{
		if (value.second != nullptr)
		{
			BasicAllocator::destroy<detail::BaseProperty>(allocator, value.second);
		}
	}
	values.clear();
	locals_by_key.clear();
	locals.clear();
#ifndef BMASHINA_DISABLE_DEBUG
	value_printers.clear();
#endif
}

template <typename M>
void bmashina::BasicState<M>::set_locals_key(const void* key)
{
	current_locals_key = key;
	if (locals_by_key.count(key) == 0)
	{
		locals_by_key.emplace(key, LocalSet::construct(*mashina));
	}
}

template <typename M>
void bmashina::BasicState<M>::invalidate_locals(const void* key)
{
	auto iter = locals_by_key.find(key);
	if (iter != locals_by_key.end())
	{
		for (auto i: iter->second)
		{
			locals.erase(i);
			values.erase(i);
#ifndef BMASHINA_DISABLE_DEBUG
			value_printers.erase(i);
#endif
		}

		locals_by_key.erase(iter);
	}
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

	if (source.values.count(&reference) != 0)
	{
		auto value = source.values.find(&reference);
		assert(value != source.values.end());

		destination.remove_value(&reference);
		if (value->second != nullptr)
		{
			destination.values[&reference] = value->second->clone(destination.allocator);
			if (source.locals.count(&reference) != 0)
			{
				destination.locals.insert(&reference);
				destination.locals_by_key[destination.current_locals_key].insert(&reference);
			}
		}

#ifndef BMASHINA_DISABLE_DEBUG
		auto printer = source.value_printers.find(&reference);
		if (printer == source.value_printers.end())
		{
			destination.value_printers.erase(&reference);
		}
		else
		{
			destination.value_printers[&reference] = printer->second;
		}
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

	if (source.values.count(&source_reference) != 0)
	{
		auto source_value = source.values.find(&source_reference);

		assert(source_value != source.values.end());

		destination.remove_value(&destination_reference);
		if (source_value->second != nullptr)
		{
			destination.values[&destination_reference] = source_value->second->clone(destination.allocator);
		}

#ifndef BMASHINA_DISABLE_DEBUG
		auto printer = source.value_printers.find(&source_reference);
		if (printer == source.value_printers.end())
		{
			destination.value_printers.erase(&destination_reference);
		}
		else
		{
			destination.value_printers[&destination_reference] = printer->second;
		}
#endif
	}
}

template <typename M>
void bmashina::BasicState<M>::remove_value(const detail::BaseReference* key)
{
	auto iter = values.find(key);
	if (iter != values.end() && iter->second != nullptr)
	{
		BasicAllocator::destroy<detail::BaseProperty>(allocator, iter->second);
		value_printers.erase(key);
		values.erase(iter);
	}
}

#ifndef BMASHINA_DISABLE_DEBUG
#include <cstdio>

template <typename M>
void bmashina::BasicState<M>::for_each_property(const PropertyIter& callback) const
{
	for (auto& i: values)
	{
		auto reference = i.first;
		typename String<M>::Type value;
		{
			auto iter = value_printers.find(reference);
			if (iter != value_printers.end() && i.second != nullptr )
			{
				value = iter->second(*mashina, *this, reference);
			}
			else
			{
				if (i.second == nullptr)
				{
					value = String<M>::construct(*mashina, "(null)");
				}
				else
				{
					value = String<M>::construct(*mashina, "(unknown)");
				}
			}
		}

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
