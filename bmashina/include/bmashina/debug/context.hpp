// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_DEBUG_DEBUG_CONTEXT_HPP
#define BMASHINA_DEBUG_DEBUG_CONTEXT_HPP

#ifdef BMASHINA_DISABLE_DEBUG
#error unset BMASHINA_DISABLE_DEBUG to use debugging facilities
#endif

#include <boost/coroutine2/all.hpp>
#include "bmashina/executor.hpp"
#include "bmashina/node.hpp"
#include "bmashina/tree.hpp"
#include "bmashina/status.hpp"
#include "bmashina/state/state.hpp"

namespace bmashina
{
	enum class Break
	{
		before,
		after
	};

	template <typename M>
	class BasicContext
	{
	public:
		typedef M Mashina;
		typedef BasicContext<Mashina> Context;
		typedef BasicExecutor<Mashina> Executor;
		typedef BasicNode<Mashina> Node;
		typedef BasicTree<Mashina> Tree;
		typedef BasicState<Mashina> State;

		BasicContext(Tree& tree, Executor& executor);
		BasicContext(const Context& other) = delete;
		~BasicContext();

		bool done() const;
		Break type() const;

		Tree& tree() const;
		Status status() const;
		Node& node() const;

		State& state() const;
		Mashina& mashina() const;

		std::size_t depth() const;
		std::size_t index() const;

		bool next();
		bool out();
		bool over();
		void finish();

		BasicContext& operator =(const Context& other) = delete;

	private:
		Tree* tree_instance;
		Executor* executor;

		struct Frame
		{
			Break type;
			Status status;
			Tree* tree;
			Node* node;
			State* state;
			std::size_t depth;
			std::size_t index;
		};
		Frame current;
		void pull();

		typedef boost::coroutines2::coroutine<Frame> StepFunction;
		typename StepFunction::pull_type step;
		static void execute(Context& context, typename StepFunction::push_type& push);

		class Preview : public Executor::Preview
		{
		public:
			Preview(Context& context, typename StepFunction::push_type& push);
			~Preview() = default;

			void before_enter_tree(Tree* tree) override;
			void after_enter_tree(Tree* tree) override;
			void before_leave_tree(Tree* tree) override;
			void after_leave_tree(Tree* tree) override;
			void before_update_node(Node& node) override;
			void after_update_node(Node& node, Status status) override;

		private:
			Context* context;
			typename StepFunction::push_type& push;

			Tree* current_tree = nullptr;
		};
	};
}

template <typename M>
bmashina::BasicContext<M>::BasicContext(Tree& tree, Executor& executor) :
	tree_instance(&tree),
	executor(&executor),
	step([this](auto& push) { this->execute(*this, push); })
{
	pull();
}

template <typename M>
bmashina::BasicContext<M>::~BasicContext()
{
	finish();
}

template <typename M>
bool bmashina::BasicContext<M>::done() const
{
	return !step;
}

template <typename M>
bmashina::Break bmashina::BasicContext<M>::type() const
{
	assert(!done());
	return current.type;
}

template <typename M>
typename bmashina::BasicContext<M>::Tree&
bmashina::BasicContext<M>::tree() const
{
	assert(!done());
	return *current.tree;
}

template <typename M>
bmashina::Status bmashina::BasicContext<M>::status() const
{
	return current.status;
}

template <typename M>
typename bmashina::BasicContext<M>::Node&
bmashina::BasicContext<M>::node() const
{
	assert(!done());
	return *current.node;
}

template <typename M>
typename bmashina::BasicContext<M>::State&
bmashina::BasicContext<M>::state() const
{
	assert(!done());
	return *current.state;
}

template <typename M>
typename bmashina::BasicContext<M>::Mashina&
bmashina::BasicContext<M>::mashina() const
{
	assert(!done());
	return *current.mashina;
}

template <typename M>
std::size_t bmashina::BasicContext<M>::depth() const
{
	assert(!done());
	return current.depth;
}

template <typename M>
std::size_t bmashina::BasicContext<M>::index() const
{
	assert(!done());
	return current.index;
}

template <typename M>
bool bmashina::BasicContext<M>::next()
{
	assert(!done());

	pull();
	if (done())
	{
		return false;
	}

	return true;
}

template <typename M>
bool bmashina::BasicContext<M>::out()
{
	assert(!done());

	std::size_t target_depth = current.depth;
	do
	{
		pull();
		if (done())
		{
			return false;
		}
	} while(current.depth >= target_depth);

	return true;
}

template <typename M>
bool bmashina::BasicContext<M>::over()
{
	assert(!done());

	std::size_t current_depth = current.depth;
	do
	{
		pull();
		if (done())
		{
			return false;
		}
	} while(current.depth > current_depth);

	return true;
}

template <typename M>
void bmashina::BasicContext<M>::finish()
{
	while (!done())
	{
		pull();
	}
}

template <typename M>
void bmashina::BasicContext<M>::pull()
{
	if (step)
	{
		current = step.get();
		step();
	}
}

template <typename M>
void bmashina::BasicContext<M>::execute(
	Context& context,
	typename StepFunction::push_type& push)
{
	Preview preview(context, push);
	context.executor->set_preview(&preview);

	context.tree_instance->execute(*context.executor);
	context.executor->set_preview(nullptr);
}

template<typename M>
bmashina::BasicContext<M>::Preview::Preview(
	Context& context,
	typename StepFunction::push_type& push) :
	context(&context),
	push(push)
{
	// Nothing.
}

template<typename M>
void bmashina::BasicContext<M>::Preview::before_enter_tree(Tree* tree)
{
	current_tree = tree;
}

template<typename M>
void bmashina::BasicContext<M>::Preview::after_enter_tree(Tree* tree)
{
	current_tree = tree;
}

template<typename M>
void bmashina::BasicContext<M>::Preview::before_leave_tree(Tree* tree)
{
	current_tree = tree;
}

template<typename M>
void bmashina::BasicContext<M>::Preview::after_leave_tree(Tree* tree)
{
	current_tree = tree;
}

template<typename M>
void bmashina::BasicContext<M>::Preview::before_update_node(Node& node)
{
	Frame frame;
	frame.type = Break::before;
	frame.status = Status::none;
	frame.tree = current_tree;
	frame.node = &node;
	frame.state = &context->executor->state();
	frame.depth = context->executor->get_current_depth();
	frame.index = context->executor->get_current_index();

	push(frame);
}

template<typename M>
void bmashina::BasicContext<M>::Preview::after_update_node(Node& node, Status status)
{
	Frame frame;
	frame.type = Break::after;
	frame.status = status;
	frame.tree = current_tree;
	frame.node = &node;
	frame.state = &context->executor->state();
	frame.depth = context->executor->get_current_depth();
	frame.index = context->executor->get_current_index();

	push(frame);
}

#endif
