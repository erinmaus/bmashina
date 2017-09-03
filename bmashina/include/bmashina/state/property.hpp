// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_STATE_PROPERTY_HPP
#define BMASHINA_STATE_PROPERTY_HPP

#include <string>
#include "bmashina/config.hpp"

namespace bmashina
{
	namespace detail
	{
		class BaseProperty
		{
		public:
			virtual ~BaseProperty() = default;

			virtual BaseProperty* clone(BasicAllocator& allocator) const = 0;
		};
	}

	template <typename V>
	class Property : public detail::BaseProperty
	{
	public:
		typedef V Value;

		Property() = default;
		Property(const Value& value);
		~Property() = default;

		Value& get();
		const Value& get() const;
		void set(const Value& new_value);

		Value* operator ->();
		const Value* operator ->() const;
		Value& operator *();
		const Value& operator *() const;

		BaseProperty* clone(BasicAllocator& allocator) const override;

		operator Value&();
		operator const Value&() const;

	private:
		Value value;
	};

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
	template <typename V>
	class Property<V*> : public detail::BaseProperty
	{
	public:
		typedef V Value;

		Property() = default;
		Property(const Value& value);
		Property(Value* value);
		Property(const std::shared_ptr<Value>& value);
		~Property() = default;

		Value* get();
		Value* get() const;
		void set(const Value& new_value);

		Value* operator ->();
		const Value* operator ->() const;
		Value& operator *();
		const Value& operator *() const;

		BaseProperty* clone(BasicAllocator& allocator) const override;

		operator Value&();
		operator const Value&() const;

	private:
		std::shared_ptr<Value> value;
	};
#endif
}

template <typename V>
bmashina::Property<V>::Property(const Value& value) : value(value)
{
	// Nothing.
}

template <typename V>
typename bmashina::Property<V>::Value&
bmashina::Property<V>::get()
{
	return value;
}

template <typename V>
const typename bmashina::Property<V>::Value&
bmashina::Property<V>::get() const
{
	return value;
}

template <typename V>
void bmashina::Property<V>::set(const Value& new_value)
{
	value = new_value;
}

template <typename V>
typename bmashina::Property<V>::Value*
bmashina::Property<V>::operator ->()
{
	return &get();
}

template <typename V>
const typename bmashina::Property<V>::Value*
bmashina::Property<V>::operator ->() const
{
	return &get();
}

template <typename V>
typename bmashina::Property<V>::Value&
bmashina::Property<V>::operator *()
{
	return get();
}

template <typename V>
const typename bmashina::Property<V>::Value&
bmashina::Property<V>::operator *() const
{
	return get();
}

template <typename V>
bmashina::detail::BaseProperty*
bmashina::Property<V>::clone(BasicAllocator& allocator) const
{
	return BasicAllocator::create<Property<V>>(allocator, *this);
}

template <typename V>
bmashina::Property<V>::operator Value&()
{
	return get();
}

template <typename V>
bmashina::Property<V>::operator const Value&() const
{
	return get();
}

#ifndef BMASHINA_DISABLE_STL_CONTAINERS
template <typename V>
bmashina::Property<V*>::Property(const Value& value) :
	value(std::make_shared<Value>(value))
{
	// Nothing.
}

template <typename V>
bmashina::Property<V*>::Property(Value* value) : value(value)
{
	// Nothing.
}

template <typename V>
bmashina::Property<V*>::Property(const std::shared_ptr<Value>& value) : value(value)
{
	// Nothing.
}

template <typename V>
typename bmashina::Property<V*>::Value*
bmashina::Property<V*>::get()
{
	assert(value.get() != nullptr);
	return value.get();
}

template <typename V>
typename bmashina::Property<V*>::Value*
bmashina::Property<V*>::get() const
{
	assert(value.get() != nullptr);
	return value.get();
}

template <typename V>
void bmashina::Property<V*>::set(const Value& new_value)
{
	value = new_value;
}

template <typename V>
typename bmashina::Property<V*>::Value*
bmashina::Property<V*>::operator ->()
{
	return get();
}

template <typename V>
const typename bmashina::Property<V*>::Value*
bmashina::Property<V*>::operator ->() const
{
	return get();
}

template <typename V>
typename bmashina::Property<V*>::Value&
bmashina::Property<V*>::operator *()
{
	return *get();
}

template <typename V>
const typename bmashina::Property<V*>::Value&
bmashina::Property<V*>::operator *() const
{
	return *get();
}

template <typename V>
bmashina::detail::BaseProperty*
bmashina::Property<V*>::clone(BasicAllocator& allocator) const
{
	return BasicAllocator::create<Property<V*>>(allocator, *this);
}

template <typename V>
bmashina::Property<V*>::operator Value&()
{
	return get();
}

template <typename V>
bmashina::Property<V*>::operator const Value&() const
{
	return get();
}
#endif

#endif
