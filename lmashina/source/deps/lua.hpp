// MAPP
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright [bk]door.maus

#ifndef MAPP_DEPS_LUA_HPP
#define MAPP_DEPS_LUA_HPP

extern "C"
{
	#include "lua.h"
}

#define SOL_SAFE_USERTYPE 1
#define SOL_SAFE_FUNCTION 1
#define SOL_CHECK_ARGUMENTS 1
#define SOL_SAFE_GETTER 1
#include "sol.hpp"

#endif
