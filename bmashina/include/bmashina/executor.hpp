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

		Status update(Node&);

		void finish();
		void reset();

		Mashina* operator ->();
		Mashina& operator *();

	private:
		Mashina* mashina_instance;

		void enter(Node& node);
		void leave(Node& node);

		bool is_in_interrupt = false;

		typedef Node* NodePointer;
		typedef typename bmashina::Vector<M, NodePointer> NodePath;
		typename NodePath::Type active_nodes;
		typename NodePath::Type previous_active_nodes;
		std::size_t current_depth = 0;
	};
}

template <typename M>
bmashina::BasicExecutor<M>::BasicExecutor(Mashina& mashina) :
	mashina_instance(&mashina),
	active_nodes(NodePath::construct(mashina)),
	previous_active_nodes(NodePath::construct(mashina))
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
bmashina::Status bmashina::BasicExecutor<M>::update(Node& node)
{
	Status result;
#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	enter(node);
	try
	{
		result = node.update(*this);
	}
	catch (...)
	{
		leave(node);
		throw;
	}
	leave(node);
#else
	enter(node);
	{
		result = node.update(*this);
	}
	leave(node);
#endif

	return result;
}

template <typename M>
void bmashina::BasicExecutor<M>::finish()
{
	assert(current_depth == 0);

	std::swap(previous_active_nodes, active_nodes);
	active_nodes.clear();
}

template <typename M>
void bmashina::BasicExecutor<M>::reset()
{
	is_in_interrupt = true;
	for (auto node: previous_active_nodes)
	{
#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
		try
		{
			node->interrupt(*this);
		}
		catch (...)
		{
			is_in_interrupt = false;
			throw;
		}
#else
		node->interrupt();
#endif
	}
	is_in_interrupt = false;
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

template <typename M>
void bmashina::BasicExecutor<M>::enter(Node& node)
{
	assert(!is_in_interrupt);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (is_in_interrupt)
	{
		throw std::runtime_error("cannot enter node while in interrupt");
	}
#endif

	if (current_depth < active_nodes.size())
	{
		if (current_depth < previous_active_nodes.size())
		{
			auto previous_node = previous_active_nodes[current_depth];
			if (previous_node != &node)
			{
				is_in_interrupt = true;
				for (auto i = current_depth; i < previous_active_nodes.size(); ++i)
				{
#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
					try
					{
						previous_active_nodes[i]->interrupt(*this);
					}
					catch (...)
					{
						is_in_interrupt = false;
						throw;
					}
#else
					previous_active_nodes[i]->interrupt();
#endif
				}
				is_in_interrupt = false;
			}
		}

		active_nodes.resize(current_depth + 1);
		active_nodes[current_depth] = &node;
	}
	else
	{
		active_nodes.push_back(&node);
	}

	++current_depth;
}

template <typename M>
void bmashina::BasicExecutor<M>::leave(Node& node)
{
	assert(current_depth > 0);
	assert(active_nodes[current_depth - 1] == &node);

	--current_depth;
}

#endif
