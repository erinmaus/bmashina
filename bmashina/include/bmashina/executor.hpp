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

		BasicExecutor(Mashina& mashina);
		~BasicExecutor();

		Mashina& mashina();
		State& state();

		void enter(Tree& tree);
		void leave(Tree& tree);

		Mashina* operator ->();
		Mashina& operator *();

	private:
		void finish();

		Mashina* mashina_instance;

		typedef typename Allocator<Mashina>::Type AllocatorType;
		AllocatorType allocator;

		struct StateFrame
		{
			StateFrame(
				Mashina& mashina,
				AllocatorType& allocator,
				Tree* tree,
				StateFrame* parent = nullptr);
			~StateFrame();

			AllocatorType* allocator;
			StateFrame* parent;
			Tree* tree;
			State state;
			std::size_t index = 0;

			void shrink(std::size_t new_index);

			typedef Vector<Mashina, StateFrame*> Children;
			typename Children::Type children;
		};
		StateFrame* frames = nullptr;
		StateFrame* current_frame = nullptr;

		void push_frame(Tree& tree);
		void leave_frame(Tree& tree);
		StateFrame* new_frame(Tree& tree);

		std::size_t current_depth = 0;
	};
}

template <typename M>
bmashina::BasicExecutor<M>::BasicExecutor(Mashina& mashina) :
	mashina_instance(&mashina),
	allocator(mashina),
	frames(BasicAllocator::create<StateFrame>(allocator, mashina, allocator, nullptr)),
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
void bmashina::BasicExecutor<M>::enter(Tree& tree)
{
	push_frame(tree);
	++current_depth;
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

	leave_frame(tree);
	current_frame->index = 0;
	--current_depth;
}

template <typename M>
void bmashina::BasicExecutor<M>::push_frame(Tree& tree)
{
	auto index = current_frame->index;
	++current_frame->index;

	if (index < current_frame->children.size())
	{
		auto next_tree = current_frame->children[index]->tree;
		if (next_tree != &tree)
		{
			auto frame = new_frame(tree);
			current_frame->shrink(index);
			current_frame->children.push_back(frame);
			current_frame = frame;
		}
		else
		{
			current_frame = current_frame->children[index];
		}
	}
	else
	{
		auto frame = new_frame(tree);
		current_frame->children.emplace_back(frame);
		current_frame = frame;
	}
}

template <typename M>
void bmashina::BasicExecutor<M>::leave_frame(Tree& tree)
{
	assert(current_frame != nullptr);

	current_frame->index = 0;
	current_frame = current_frame->parent;
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
typename bmashina::BasicExecutor<M>::StateFrame*
bmashina::BasicExecutor<M>::new_frame(Tree& tree)
{
	return BasicAllocator::create<StateFrame>(allocator, *mashina_instance, allocator, &tree, current_frame);
}

template <typename M>
bmashina::BasicExecutor<M>::StateFrame::StateFrame(Mashina& mashina, AllocatorType& allocator, Tree* tree, StateFrame* parent) :
	allocator(&allocator),
	parent(parent),
	tree(tree),
	state(mashina),
	children(Children::construct(mashina))
{
	// Nothing.
}

template <typename M>
bmashina::BasicExecutor<M>::StateFrame::~StateFrame()
{
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
