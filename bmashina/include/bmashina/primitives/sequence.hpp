// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_PRIMITIVES_SEQUENCE_HPP
#define BMASHINA_PRIMITIVES_SEQUENCE_HPP

#include "bmashina/composite.hpp"

namespace bmashina
{
	template <typename M>
	class Sequence : public BasicComposite<M>
	{
	public:
		using typename BasicComposite<M>::Mashina;
		using typename BasicComposite<M>::Tree;

		Sequence() = default;
		~Sequence() = default;

	protected:
		Status update(Mashina& mashina) override;
	};
}

template <typename M>
bmashina::Status bmashina::Sequence<M>::update(Mashina& mashina)
{
	auto current = this->tree().children_begin(*this);
	auto end = this->tree().children_end(*this);

	while (current != end)
	{
		auto result = current->step(mashina);
		if (result == bmashina::Status::working)
		{
			return bmashina::Status::working;
		}
		else if (result == bmashina::Status::failure)
		{
			return bmashina::Status::failure;
		}

		++current;
	}

	return bmashina::Status::success;
}

#endif
