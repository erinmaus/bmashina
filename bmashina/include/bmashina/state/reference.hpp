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
			BaseReference() = default;
			BaseReference(const char* name);

			const char* name = nullptr;
		};
	}

	template <typename V>
	struct Reference : public detail::BaseReference
	{
		typedef V Type;

		explicit Reference(const char* name = nullptr);
	};

	template <typename V>
	struct Local : public detail::BaseReference
	{
		typedef V Type;

		explicit Local(const char* name = nullptr);
	};
}

inline bmashina::detail::BaseReference::BaseReference(const char* name) :
	name(name)
{
	// Nothing.
}

template <typename V>
bmashina::Reference<V>::Reference(const char* name) :
	detail::BaseReference(name)
{
	// Nothing.
}

template <typename V>
bmashina::Local<V>::Local(const char* name) :
	detail::BaseReference(name)
{
	// Nothing.
}

#endif
