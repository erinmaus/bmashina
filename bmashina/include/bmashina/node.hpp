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
#include "bmashina/state/reference.hpp"

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
#include <stdexcept>
#endif

namespace bmashina
{
	template <typename M>
	class BasicTree;

	template <typename M>
	class BasicExecutor;

	template <typename M>
	class BasicNode
	{
	public:
		typedef M Mashina;
		typedef BasicNode<Mashina> Node;
		typedef BasicTree<Mashina> Tree;
		typedef BasicExecutor<Mashina> Executor;

		BasicNode() = default;
		BasicNode(const BasicNode& other) = delete;
		virtual ~BasicNode() = default;

		void attach(Tree& tree);
		bool attached() const;

		void visit(Executor& executor);
		void drop(Executor& executor);
		virtual Status update(Executor& executor);

		BasicNode& operator =(const BasicNode& other) = delete;

		Tree& tree();
		const Tree& tree() const;

	protected:
		virtual void activated(Executor& executor);
		virtual void deactivated(Executor& executor);

		void deactivate(Executor& executor);

	private:
		Tree* tree_instance = nullptr;
		Local<bool> visited = Local<bool>("_node_visited");
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
void bmashina::BasicNode<M>::visit(Executor& executor)
{
	if (!executor.state().has(visited))
	{
		executor.state().set(visited, true);

		activated(executor);
	}
}

template <typename M>
void bmashina::BasicNode<M>::drop(Executor& executor)
{
	if (executor.state().has(visited))
	{
		deactivated(executor);
		executor.state().unset(visited);
	}
}

template <typename M>
bmashina::Status bmashina::BasicNode<M>::update(Executor& executor)
{
	return Status::success;
}

template <typename M>
void bmashina::BasicNode<M>::activated(Executor& executor)
{
	// Nothing.
}

template <typename M>
void bmashina::BasicNode<M>::deactivated(Executor& executor)
{
	// Nothing.
}

template <typename M>
void bmashina::BasicNode<M>::deactivate(Executor& executor)
{
	executor.state().unset(visited);
	deactivated(executor);
}

#endif
