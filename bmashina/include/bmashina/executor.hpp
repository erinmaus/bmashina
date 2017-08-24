// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_EXECUTOR_HPP
#define BMASHINA_EXECUTOR_HPP

#include <utility>
#include <cassert>
#include <cstddef>
#include "bmashina/config.hpp"
#include "bmashina/node.hpp"
#include "bmashina/status.hpp"
#include "bmashina/state/state.hpp"

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
		typedef BasicState<Mashina> State;
		typedef BasicExecutor<Mashina> Executor;

		BasicExecutor(Mashina& mashina);
		~BasicExecutor();

		Mashina& mashina();
		State& state();

		void reset();

		void enter(Tree& tree);
		void leave(Tree& tree);

		void enter(Node& node);
		void leave(Node& node, Status status);
		Status update(Node& node);

		Mashina* operator ->();
		Mashina& operator *();

#ifndef BMASHINA_DISABLE_DEBUG
		class Preview
		{
		public:
			virtual void before_enter_tree(Tree* tree) = 0;
			virtual void after_enter_tree(Tree* tree) = 0;
			virtual void before_leave_tree(Tree* tree) = 0;
			virtual void after_leave_tree(Tree* tree) = 0;
			virtual void before_update_node(Node& node) = 0;
			virtual void after_update_node(Node& node, Status status) = 0;
		};

		std::size_t get_current_depth() const;
		std::size_t get_current_index() const;

		void set_preview(Preview* value);
#endif

	private:
		Mashina* mashina_instance;

		typedef typename Allocator<Mashina>::Type AllocatorType;
		AllocatorType allocator;

		struct StateFrame
		{
			StateFrame(
				Executor& executor,
				AllocatorType& allocator,
				Tree* tree,
				Node* node,
				StateFrame* parent = nullptr);
			~StateFrame();

			Executor* executor;
			AllocatorType* allocator;
			StateFrame* parent;
			Tree* tree;
			Node* node;
			State state;
			State* tree_state;
			std::size_t index = 0;

			void shrink(std::size_t new_index);

			typedef Vector<Mashina, StateFrame*> Children;
			typename Children::Type children;
		};
		StateFrame* frames = nullptr;
		StateFrame* current_frame = nullptr;

		void push_frame(Tree& tree, Node* node = nullptr);
		void leave_frame(Tree& tree, Node* node = nullptr);
		StateFrame* new_frame(Tree& tree, Node* node);

		std::size_t current_depth = 0;

#ifndef BMASHINA_DISABLE_DEBUG
		Preview* preview = nullptr;
#endif
	};
}

template <typename M>
bmashina::BasicExecutor<M>::BasicExecutor(Mashina& mashina) :
	mashina_instance(&mashina),
	allocator(mashina),
	frames(BasicAllocator::create<StateFrame>(allocator, *this, allocator, nullptr, nullptr)),
	current_frame(frames)
{
	// Nothing.
}

template <typename M>
bmashina::BasicExecutor<M>::~BasicExecutor()
{
	BasicAllocator::destroy(allocator, frames);
}

template <typename M>
typename bmashina::BasicExecutor<M>::Mashina&
bmashina::BasicExecutor<M>::mashina()
{
	return *mashina_instance;
}

template <typename M>
typename bmashina::BasicExecutor<M>::State&
bmashina::BasicExecutor<M>::state()
{
	assert(current_frame != nullptr);
	return current_frame->state;
}

template <typename M>
void bmashina::BasicExecutor<M>::reset()
{
	assert(current_depth == 0);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (current_depth == 0)
	{
		throw std::runtime_error("cannot reset while executing");
	}
#endif

	assert(current_frame != nullptr);
	this->frames->shrink(0);
	this->current_frame = this->frames;
}

template <typename M>
void bmashina::BasicExecutor<M>::enter(Tree& tree)
{
#ifndef BMASHINA_DISABLE_DEBUG
	if (preview != nullptr)
	{
		preview->before_enter_tree(current_frame->tree);
	}
#endif

	push_frame(tree);

#ifndef BMASHINA_DISABLE_DEBUG
	if (preview != nullptr)
	{
		preview->after_enter_tree(&tree);
	}
#endif
}

template <typename M>
void bmashina::BasicExecutor<M>::leave(Tree& tree)
{
	assert(current_depth > 0);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (current_depth == 0)
	{
		throw std::runtime_error("unbalanced execution stack");
	}
#endif

#ifndef BMASHINA_DISABLE_DEBUG
	if (preview != nullptr)
	{
		preview->before_leave_tree(&tree);
	}
#endif

	leave_frame(tree);

#ifndef BMASHINA_DISABLE_DEBUG
	if (preview != nullptr)
	{
		preview->after_leave_tree(current_frame->tree);
	}
#endif
}

template <typename M>
void bmashina::BasicExecutor<M>::enter(Node& node)
{
	assert(current_depth > 0);

#ifndef BMASHINA_DISABLE_DEBUG
	if (preview != nullptr)
	{
		preview->before_update_node(node);
	}
#endif

	push_frame(*current_frame->tree, &node);
}

template <typename M>
void bmashina::BasicExecutor<M>::leave(Node& node, Status status)
{
	leave_frame(*current_frame->tree, &node);

#ifndef BMASHINA_DISABLE_DEBUG
	if (preview != nullptr)
	{
		preview->after_update_node(node, status);
	}
#endif
}

template <typename M>
bmashina::Status bmashina::BasicExecutor<M>::update(Node& node)
{
	assert(current_depth > 0);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (current_depth == 0)
	{
		throw std::runtime_error("tree is not executing");
	}
#endif

	return current_frame->tree->update(*this, node);
}

template <typename M>
void bmashina::BasicExecutor<M>::push_frame(Tree& tree, Node* node)
{
	auto index = current_frame->index;
	++current_frame->index;
	++current_depth;

	auto previous_frame = current_frame;
	if (index < current_frame->children.size())
	{
		auto next_tree = current_frame->children[index]->tree;
		auto next_node = current_frame->children[index]->node;
		if (next_tree != &tree || next_node != node)
		{
			auto frame = new_frame(tree, node);
			current_frame->shrink(index);
			current_frame->children.push_back(frame);
			current_frame = frame;
		}
		else
		{
			current_frame = current_frame->children[index];
			current_frame->index = 0;
		}
	}
	else
	{
		auto frame = new_frame(tree, node);
		current_frame->children.emplace_back(frame);
		current_frame = frame;
	}

	State::copy(previous_frame->state, current_frame->state);
}

template <typename M>
void bmashina::BasicExecutor<M>::leave_frame(Tree& tree, Node* node)
{
	--current_depth;

	assert(current_frame != nullptr);
	assert(current_frame->tree == &tree);
	assert(current_frame->node == node);

	auto previous_frame = current_frame;
	current_frame = current_frame->parent;

	State::copy(previous_frame->state, current_frame->state);
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
std::size_t bmashina::BasicExecutor<M>::get_current_depth() const
{
	return current_depth;
}

template <typename M>
std::size_t bmashina::BasicExecutor<M>::get_current_index() const
{
	return current_frame->index;
}

template <typename M>
void bmashina::BasicExecutor<M>::set_preview(Preview* value)
{
	preview = value;
}

template <typename M>
typename bmashina::BasicExecutor<M>::StateFrame*
bmashina::BasicExecutor<M>::new_frame(Tree& tree, Node* node)
{
	return BasicAllocator::create<StateFrame>(allocator, *this, allocator, &tree, node, current_frame);
}

template <typename M>
bmashina::BasicExecutor<M>::StateFrame::StateFrame(
	Executor& executor,
	AllocatorType& allocator,
	Tree* tree,
	Node* node,
	StateFrame* parent) :
	executor(&executor),
	allocator(&allocator),
	parent(parent),
	tree(tree),
	node(node),
	state(executor.mashina()),
	children(Children::construct(executor.mashina()))
{
	if (node == nullptr)
	{
		tree_state = &state;
	}
	else
	{
		tree_state = parent->tree_state;
	}
}

template <typename M>
bmashina::BasicExecutor<M>::StateFrame::~StateFrame()
{
	if (node != nullptr)
	{
		node->drop(*executor);
	}
	shrink(0);
}

template <typename M>
void bmashina::BasicExecutor<M>::StateFrame::shrink(std::size_t size)
{
	while (children.size() > size)
	{
		auto back = children.back();
		BasicAllocator::destroy<StateFrame>(*allocator, back);

		children.pop_back();
	}
}

#endif
