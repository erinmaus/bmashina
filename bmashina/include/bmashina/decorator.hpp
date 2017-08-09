// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_DECORATOR_HPP
#define BMASHINA_DECORATOR_HPP

#include <cassert>
#include "bmashina/node.hpp"

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
#include <stdexcept>
#endif

namespace bmashina
{
	template <typename M>
	class BasicDecorator : public BasicNode<M>
	{
	public:
		using typename BasicNode<M>::Node;
		using typename BasicNode<M>::Executor;

		BasicDecorator() = default;
		~BasicDecorator() = default;

		Status update(Executor& executor) override;

	protected:
		bool has_child();
		Node& child();
	};
}

template <typename M>
bmashina::Status bmashina::BasicDecorator<M>::update(Executor& executor)
{
	if (has_child())
	{
		return child().update(executor);
	}

	return Status::success;
}

template <typename M>
bool bmashina::BasicDecorator<M>::has_child()
{
	if (!this->attached())
	{
		return false;
	}

	return this->tree().children_begin(*this) != this->tree().children_end(*this);
}

template <typename M>
typename bmashina::BasicDecorator<M>::Node& bmashina::BasicDecorator<M>::child()
{
	assert(has_child());

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has_child())
	{
		throw std::runtime_error("decorator has no child");
	}
#endif

	return *this->tree().children_begin(*this);
}

#endif
