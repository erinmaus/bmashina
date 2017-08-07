// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_DECORATOR_HPP
#define BMASHINA_DECORATOR_HPP

#include "bmashina/node.hpp"

namespace bmashina
{
	template <typename M>
	class BasicDecorator : public BasicNode<M>
	{
	public:
		BasicDecorator() = default;
		~BasicDecorator() = default;
	};
}

#endif
