// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_PRIMITIVES_SELECTOR_HPP
#define BMASHINA_PRIMITIVES_SELECTOR_HPP

#include "bmashina/composite.hpp"

namespace bmashina
{
	template <typename M>
	class Selector : public BasicComposite<M>
	{
	public:
		using typename BasicComposite<M>::Mashina;
		using typename BasicComposite<M>::Tree;

		Selector() = default;
		~Selector() = default;

	protected:
		Status update(Mashina& mashina) override;
	};
}

template <typename M>
bmashina::Status bmashina::Selector<M>::update(Mashina& mashina)
{
	auto current = this->tree().children_begin(*this);
	auto end = this->tree().children_end(*this);

	while (current != end)
	{
		auto result = current->step(mashina);
		if (result != bmashina::Status::failure)
		{
			return result;
		}

		++current;
	}

	return bmashina::Status::failure;
}

#endif
