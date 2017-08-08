// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_NODE_HPP
#define BMASHINA_NODE_HPP

#include <cassert>
#include "bmashina/status.hpp"

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
#include <stdexcept>
#endif

namespace bmashina
{
	template <typename M>
	class BasicTree;

	template <typename M>
	class BasicNode
	{
	public:
		typedef M Mashina;
		typedef BasicNode<Mashina> Node;
		typedef BasicTree<Mashina> Tree;

		BasicNode() = default;
		BasicNode(const BasicNode& other) = delete;
		virtual ~BasicNode() = default;

		void attach(Tree& tree);
		bool attached() const;

		void before_step(Mashina& mashina);
		void after_step(Mashina& mashina);
		Status step(Mashina& mashina);

		BasicNode& operator =(const BasicNode& other) = delete;

	protected:
		Tree& tree();
		const Tree& tree() const;

		virtual void preupdate(Mashina& mashina);
		virtual void postupdate(Mashina& mashina);
		virtual Status update(Mashina& mashina);

		virtual void interrupted(Mashina& mashina);
		virtual void activated(Mashina& mashina);
		virtual void deactivated(Mashina& mashina);

	private:
		Tree* tree_instance = nullptr;

		bool was_active = false;
		bool is_active = false;
		Status previous_status = Status::none;
	};
}

template <typename M>
void bmashina::BasicNode<M>::attach(Tree& tree)
{
	assert(!attached());

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (attached())
	{
		throw std::runtime_error("node already attached to tree");
	}
#endif

	tree_instance = &tree;
}

template <typename M>
bool bmashina::BasicNode<M>::attached() const
{
	return tree_instance != nullptr;
}

template <typename M>
typename bmashina::BasicNode<M>::Tree&
bmashina::BasicNode<M>::tree()
{
	assert(attached());

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!attached())
	{
		throw std::runtime_error("cannot get tree, node is not attached");
	}
#endif

	return *tree_instance;
}

template <typename M>
const typename bmashina::BasicNode<M>::Tree&
bmashina::BasicNode<M>::tree() const
{
	assert(attached());

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!attached())
	{
		throw std::runtime_error("cannot get tree, node is not attached");
	}
#endif

	return *tree_instance;
}

template <typename M>
void bmashina::BasicNode<M>::before_step(Mashina& mashina)
{
	is_active = false;
	preupdate(mashina);
}

template <typename M>
void bmashina::BasicNode<M>::after_step(Mashina& mashina)
{
	if (was_active && !is_active)
	{
		if (previous_status == Status::working)
		{
			interrupted(mashina);
		}

		deactivated(mashina);
	}

	was_active = is_active;
	postupdate(mashina);
}

template <typename M>
bmashina::Status bmashina::BasicNode<M>::step(Mashina& mashina)
{
	if (!was_active)
	{
		activated(mashina);
	}

	is_active = true;
	previous_status = update(mashina);

	return previous_status;
}

template <typename M>
void bmashina::BasicNode<M>::preupdate(Mashina& mashina)
{
	// Nothing.
}

template <typename M>
void bmashina::BasicNode<M>::postupdate(Mashina& mashina)
{
	// Nothing.
}

template <typename M>
bmashina::Status bmashina::BasicNode<M>::update(Mashina& mashina)
{
	return Status::success;
}


template <typename M>
void bmashina::BasicNode<M>::interrupted(Mashina& mashina)
{
	// Nothing.
}

template <typename M>
void bmashina::BasicNode<M>::activated(Mashina& mashina)
{
	// Nothing.
}

template <typename M>
void bmashina::BasicNode<M>::deactivated(Mashina& mashina)
{
	// Nothing.
}

#endif
