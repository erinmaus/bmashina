// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_STATUS_HPP
#define BMASHINA_STATUS_HPP

namespace bmashina
{
	enum class Status
	{
		none,

		/// The node succeeded.
		success,

		/// The node failed.
		failure,

		/// The node has yet to finish executing.
		working
	};
}

#endif
