// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_COMPOSITE_HPP
#define BMASHINA_COMPOSITE_HPP

#include "bmashina/node.hpp"

namespace bmashina
{
	template <typename M>
	class BasicComposite : public BasicNode<M>
	{
	public:
		BasicComposite() = default;
		~BasicComposite() = default;
	};
}

#endif
