// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_STATE_REFERENCE_HPP
#define BMASHINA_STATE_REFERENCE_HPP

#include <cstddef>

namespace bmashina
{
	namespace detail
	{
		struct BaseReference
		{
			typedef std::size_t Tag;
		};
	}

	template <typename V>
	struct Reference : public detail::BaseReference
	{
		typedef V Type;

		const static Tag TAG;
	};
}

template <typename V>
const bmashina::detail::BaseReference::Tag bmashina::Reference<V>::TAG = 0;

#endif
