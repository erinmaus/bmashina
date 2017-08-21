// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_DEBUG_PROPERTY_PRINTER_HPP
#define BMASHINA_DEBUG_PROPERTY_PRINTER_HPP

#include "bmashina/config.hpp"

namespace bmashina
{
	template <typename M, typename V>
	struct PropertyPrinter
	{
		static typename String<M>::Type print(M& mashina, const Property<V>& property);
	};
}

template <typename M, typename V>
typename bmashina::String<M>::Type
bmashina::PropertyPrinter<M, V>::print(M& mashina, const Property<V>& property)
{
	return ToString<M, V>::get(mashina, property.get());
}

#endif
