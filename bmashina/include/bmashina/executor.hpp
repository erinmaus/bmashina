// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_EXECUTOR_HPP
#define BMASHINA_EXECUTOR_HPP

#include <cassert>
#include <cstddef>
#include "bmashina/config.hpp"
#include "bmashina/node.hpp"
#include "bmashina/status.hpp"

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
#include <stdexcept>
#endif

namespace bmashina
{
	template <typename M>
	class BasicTree;

	template <typename M>
	class BasicExecutor
	{
	public:
		typedef M Mashina;
		typedef BasicTree<Mashina> Tree;
		typedef BasicNode<Mashina> Node;

		BasicExecutor(Mashina& mashina);
		~BasicExecutor() = default;

		Mashina& mashina();

		void enter(Tree& tree);
		void leave(Tree& tree);

		Mashina* operator ->();
		Mashina& operator *();

	private:
		Mashina* mashina_instance;
	};
}

template <typename M>
bmashina::BasicExecutor<M>::BasicExecutor(Mashina& mashina) :
	mashina_instance(&mashina)
{
	// Nothing.
}

template <typename M>
typename bmashina::BasicExecutor<M>::Mashina&
bmashina::BasicExecutor<M>::mashina()
{
	return *mashina_instance;
}

template <typename M>
void bmashina::BasicExecutor<M>::enter(Tree& tree)
{
	// Nothing.
}

template <typename M>
void bmashina::BasicExecutor<M>::leave(Tree& tree)
{
	// Nothing.
}

template <typename M>
typename bmashina::BasicExecutor<M>::Mashina*
bmashina::BasicExecutor<M>::operator ->()
{
	return &mashina();
}

template <typename M>
typename bmashina::BasicExecutor<M>::Mashina&
bmashina::BasicExecutor<M>::operator *()
{
	return mashina();
}

#endif
