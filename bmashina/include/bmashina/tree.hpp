// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_TREE_HPP
#define BMASHINA_TREE_HPP

#include <tuple>
#include "bmashina/channel.hpp"
#include "bmashina/composite.hpp"
#include "bmashina/config.hpp"
#include "bmashina/decorator.hpp"
#include "bmashina/executor.hpp"
#include "bmashina/node.hpp"
#include "bmashina/status.hpp"
#include "bmashina/state/state.hpp"

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
#include <stdexcept>
#endif

namespace bmashina
{
	template <typename M>
	class BasicTree
	{
	public:
		typedef M Mashina;
		typedef BasicTree<Mashina> Tree;
		typedef BasicExecutor<Mashina> Executor;
		typedef BasicState<Mashina> State;
		typedef typename BasicChannel<Mashina>::Type Channel;
		typedef BasicNode<Mashina> Node;
		typedef BasicComposite<Mashina> Composite;
		typedef BasicDecorator<Mashina> Decorator;

		BasicTree(Mashina& mashina);
		BasicTree(const Tree& other) = delete;
		~BasicTree();

		template <typename V, typename... Arguments>
		const Local<V>& local(Arguments&&... arguments);

		template <typename V>
		const Local<V>& constant(const Property<V>& value);

		void input(const detail::BaseReference& referece);
		void input(
			Node& node,
			const detail::BaseReference& from,
			const detail::BaseReference& to);
		void input(
			const Channel& channel,
			const detail::BaseReference& from,
			const detail::BaseReference& to);
		void output(const detail::BaseReference& reference);
		void output(
			Node& node,
			const detail::BaseReference& from,
			const detail::BaseReference& to);
		void output(
			const Channel& channel,
			const detail::BaseReference& from,
			const detail::BaseReference& to);

		bool compatible(const Tree& other) const;

		const State& state() const;
		State& state();

		template <typename N, typename... Arguments>
		N& root(Arguments&&... arguments);

		Node& root() const;

		template <typename N, typename... Arguments>
		N& child(Node& parent, Arguments&&... arguments);
		Node& child(Node& parent, Tree& tree);
		void child(Node& parent, const Channel& channel);

		Node& assign(const Channel& channel, Tree& tree);
		bool assigned(const Channel& channel) const;
		void unassign(const Channel& channel);

		bool has(Node& node) const;
		bool has(const Channel& channel) const;

		void clear();
		bool empty() const;

		Status execute(Executor& executor);

		Status update(Executor& executor, Node& node);

		Tree& operator =(const Tree& other) = delete;

	private:
		void before_update(Executor& executor, Node& node);
		void after_update(Executor& executor, Node& node, Status status);

		Mashina* mashina;

		typedef typename Allocator<Mashina>::Type AllocatorType;
		AllocatorType allocator;

		typedef UnorderedSet<Mashina, Node*> NodeSet;
		typename NodeSet::Type nodes;
		Node* root_node = nullptr;

		template <typename N, typename... Arguments>
		N* create(Arguments&&... arguments);

		typedef UnorderedSet<Mashina, Channel> ChannelSet;
		typename ChannelSet::Type channels;

		typedef UnorderedMap<Mashina, Channel, Tree*> ChannelAssignments;
		typename ChannelAssignments::Type channel_assignments;

		typedef UnorderedMap<Mashina, Channel, Node*> ChannelNodes;
		typename ChannelNodes::Type channel_nodes;

		typedef Vector<Mashina, Node*> NodeList;
		typename NodeList::Type empty_node_list;

		typedef UnorderedMap<Mashina, Node*, typename NodeList::Type> NodeChildren;
		typename NodeChildren::Type children;
		typename NodeList::Type& get_children(Node& node);

		typedef std::tuple<const detail::BaseReference*, const detail::BaseReference*> FromToTuple;
		typedef Vector<Mashina, FromToTuple> WireList;

		typedef UnorderedMap<Mashina, Node*, typename WireList::Type> NodeWires;
		typename NodeWires::Type node_inputs;
		typename NodeWires::Type node_outputs;

		typedef UnorderedSet<Mashina, const detail::BaseReference*> ReferenceList;
		typename ReferenceList::Type inputs;
		typename ReferenceList::Type outputs;

		typedef UnorderedSet<Mashina, detail::BaseReference*> LocalSet;
		typename LocalSet::Type locals;
		typename LocalSet::Type constants;
		State constant_values;

		class ChannelProxyNode : public Node
		{
		public:
			ChannelProxyNode(const Channel& channel);
			~ChannelProxyNode() = default;

			Status update(Executor& executor) override;

		private:
			Channel channel;
		};

		class TreeProxyNode : public Node
		{
		public:
			TreeProxyNode(Tree& tree);
			~TreeProxyNode() = default;

			Status update(Executor& executor) override;

		private:
			Tree* tree;
		};

	public:
		struct ChildrenIterator : public NodeList::Type::iterator
		{
			typedef Node value_type;
			typedef value_type& reference_type;
			typedef value_type* pointer_type;
			ChildrenIterator(typename NodeList::Type::iterator value);
			reference_type operator *() const;
			pointer_type operator ->() const;
		};

		ChildrenIterator children_begin(Node& node);
		ChildrenIterator children_end(Node& node);
	};
}

template <typename M>
bmashina::BasicTree<M>::BasicTree(Mashina& mashina) :
	mashina(&mashina),
	allocator(mashina),
	nodes(NodeSet::construct(mashina)),
	channels(ChannelSet::construct(mashina)),
	channel_assignments(ChannelAssignments::construct(mashina)),
	channel_nodes(ChannelNodes::construct(mashina)),
	empty_node_list(NodeList::construct(mashina)),
	children(NodeChildren::construct(mashina)),
	node_inputs(NodeWires::construct(mashina)),
	node_outputs(NodeWires::construct(mashina)),
	inputs(ReferenceList::construct(mashina)),
	outputs(ReferenceList::construct(mashina)),
	locals(LocalSet::construct(mashina)),
	constants(LocalSet::construct(mashina)),
	constant_values(mashina)
{
	// Nothing.
}

template <typename M>
bmashina::BasicTree<M>::~BasicTree()
{
	clear();
}

template <typename M>
bool bmashina::BasicTree<M>::compatible(const Tree& other) const
{
	return mashina == other.mashina;
}

template <typename M>
template <typename N, typename... Arguments>
N& bmashina::BasicTree<M>::root(Arguments&&... arguments)
{
	if (!empty())
	{
		clear();
	}

	auto result = create<N>(std::forward<Arguments>(arguments)...);
	root_node = result;

	return *result;
}

template <typename M>
typename bmashina::BasicTree<M>::Node&
bmashina::BasicTree<M>::root() const
{
	assert(root_node != nullptr);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (root_node == nullptr)
	{
		throw std::runtime_error("tree is empty");
	}
#endif

	return *root_node;
}

template <typename M>
template <typename N, typename... Arguments>
N& bmashina::BasicTree<M>::child(Node& parent, Arguments&&... arguments)
{
	assert(has(parent));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(parent))
	{
		throw std::runtime_error("parent not in tree");
	}
#endif

	auto child = create<N>(std::forward<Arguments>(arguments)...);
	auto& children = get_children(parent);
	children.emplace_back(child);

	return *child;
}

template <typename M>
void bmashina::BasicTree<M>::child(Node& parent, const Channel& channel)
{
	assert(has(parent));
	assert(!has(channel));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(parent))
	{
		throw std::runtime_error("parent not in tree");
	}

	if (has(channel))
	{
		throw std::runtime_error("trying to add duplicate channel");
	}
#endif

	channels.insert(channel);

	auto child = create<ChannelProxyNode>(channel);
	auto& children = get_children(parent);
	children.emplace_back(child);
	channel_nodes.emplace(channel, child);
}

template <typename M>
typename bmashina::BasicTree<M>::Node&
bmashina::BasicTree<M>::child(Node& parent, Tree& tree)
{
	return child<TreeProxyNode>(parent, tree);
}

template <typename M>
typename bmashina::BasicTree<M>::Node&
bmashina::BasicTree<M>::assign(const Channel& channel, Tree& tree)
{
	assert(has(channel));
	assert(compatible(tree));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(channel))
	{
		throw std::runtime_error("channel does not exist");
	}

	if (!compatible(tree))
	{
		throw std::runtime_error("assigning incompatible tree to channel");
	}
#endif

	if (assigned(channel))
	{
		unassign(channel);
	}

	channel_assignments.emplace(channel, &tree);

	return *channel_nodes.find(channel)->second;
}

template <typename M>
bool bmashina::BasicTree<M>::assigned(const Channel& channel) const
{
	assert(has(channel));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(channel))
	{
		throw std::runtime_error("channel does not exist in tree");
	}
#endif

	return channel_assignments.count(channel) != 0;
}

template <typename M>
void bmashina::BasicTree<M>::unassign(const Channel& channel)
{
	assert(has(channel));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(channel))
	{
		throw std::runtime_error("channel does not exist in tree");
	}
#endif

	auto iter = channel_nodes.find(channel);
	node_inputs.erase(iter->second);
	node_outputs.erase(iter->second);
	channel_assignments.erase(channel);
}

template <typename M>
bool bmashina::BasicTree<M>::has(Node& node) const
{
	return nodes.count(&node) != 0;
}

template <typename M>
bool bmashina::BasicTree<M>::has(const Channel& channel) const
{
	return channels.count(channel) != 0;
}

template <typename M>
void bmashina::BasicTree<M>::clear()
{
	for (auto node: nodes)
	{
		BasicAllocator::destroy<Node>(allocator, node);
	}
	nodes.clear();
	children.clear();
	node_inputs.clear();
	node_outputs.clear();

	root_node = nullptr;

	for (auto local: locals)
	{
		BasicAllocator::destroy<detail::BaseReference>(allocator, local);
	}
	locals.clear();
	inputs.clear();
	outputs.clear();
	constants.clear();
	constant_values.clear();

	channels.clear();
	channel_assignments.clear();
	channel_nodes.clear();
}

template <typename M>
bool bmashina::BasicTree<M>::empty() const
{
	return nodes.empty();
}

template <typename M>
bmashina::Status bmashina::BasicTree<M>::execute(Executor& executor)
{
	if (empty())
	{
		return Status::failure;
	}

	Status result;
	executor.enter(*this);

	auto& state = executor.state();
	for (auto constant: constants)
	{
		State::copy(constant_values, state, *constant);
	}

	result = executor.update(*root_node);

	executor.leave(*this);

	return result;
}

template <typename M>
bmashina::Status bmashina::BasicTree<M>::update(Executor& executor, Node& node)
{
	assert(has(node));

	Status status;
	before_update(executor, node);
	{
		node.visit(executor);
		status = node.update(executor);
	}
	after_update(executor, node, status);

	return status;
}

template <typename M>
void bmashina::BasicTree<M>::before_update(Executor& executor, Node& node)
{
	executor.enter(node);
	auto& state = executor.state();

	auto iter = node_inputs.find(&node);
	if (iter != node_inputs.end())
	{
		auto& inputs = iter->second;
		for (auto i: inputs)
		{
			auto from = std::get<0>(i);
			auto to = std::get<1>(i);

			State::copy(state, state, *from, *to);
		}
	}
}

template <typename M>
void bmashina::BasicTree<M>::after_update(Executor& executor, Node& node, Status status)
{
	auto& state = executor.state();
	executor.leave(node, status);

	auto outputs_iter = node_outputs.find(&node);
	if (outputs_iter != node_outputs.end())
	{
		auto& outputs = outputs_iter->second;
		for (auto i: outputs)
		{
			auto from = std::get<0>(i);
			auto to = std::get<1>(i);

			State::copy(state, state, *from, *to);
			state.unset(*from);
		}
	}

	auto inputs_iter = node_inputs.find(&node);
	if (inputs_iter != node_inputs.end())
	{
		auto& inputs = inputs_iter->second;
		for (auto i: inputs)
		{
			auto to = std::get<1>(i);
			state.unset(*to);
		}
	}
}

template <typename M>
typename bmashina::BasicTree<M>::ChildrenIterator
bmashina::BasicTree<M>::children_begin(Node& parent)
{
	assert(has(parent));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(parent))
	{
		throw std::runtime_error("node does not exist in tree");
	}
#endif

	if (children.count(&parent) == 0)
	{
		return empty_node_list.begin();
	}

	return ChildrenIterator(get_children(parent).begin());
}

template <typename M>
typename bmashina::BasicTree<M>::ChildrenIterator
bmashina::BasicTree<M>::children_end(Node& parent)
{
	assert(has(parent));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(parent))
	{
		throw std::runtime_error("node does not exist in tree");
	}
#endif

	if (children.count(&parent) == 0)
	{
		return empty_node_list.end();
	}

	return ChildrenIterator(get_children(parent).end());
}

template <typename M>
template <typename N, typename... Arguments>
N* bmashina::BasicTree<M>::create(Arguments&&... arguments)
{
	auto node = BasicAllocator::create<N>(allocator, std::forward<Arguments>(arguments)...);
	nodes.emplace(node);

	node->attach(*this);
	return node;
}

template <typename M>
typename bmashina::BasicTree<M>::NodeList::Type&
bmashina::BasicTree<M>::get_children(Node& parent)
{
	assert(nodes.count(&parent) != 0);

	auto iter = children.find(&parent);
	if (iter == children.end())
	{
		iter = children.emplace(&parent, NodeList::construct(*mashina)).first;
	}

	return iter->second;
}

template <typename M>
bmashina::BasicTree<M>::ChannelProxyNode::ChannelProxyNode(const Channel& channel) :
	channel(channel)
{
	// Nothing.
}

template <typename M>
bmashina::Status bmashina::BasicTree<M>::ChannelProxyNode::update(Executor& executor)
{
	auto& parent_tree = this->tree();
	auto iter = parent_tree.channel_assignments.find(channel);
	if (iter != parent_tree.channel_assignments.end() && !iter->second->empty())
	{
		return iter->second->execute(executor);
	}

	return Status::failure;
}

template <typename M>
bmashina::BasicTree<M>::TreeProxyNode::TreeProxyNode(Tree& tree) :
	tree(&tree)
{
	// Nothing.
}

template <typename M>
bmashina::Status bmashina::BasicTree<M>::TreeProxyNode::update(Executor& executor)
{
	return this->tree->execute(executor);
}


template <typename M>
template <typename V, typename... Arguments>
const bmashina::Local<V>& bmashina::BasicTree<M>::local(Arguments&&... arguments)
{
	auto reference = BasicAllocator::template create<Local<V>>(allocator, std::forward<Arguments>(arguments)...);
	locals.insert(reference);

	return *reference;
}

template <typename M>
template <typename V>
const bmashina::Local<V>& bmashina::BasicTree<M>::constant(const Property<V>& value)
{
	auto reference = BasicAllocator::template create<Local<V>>(allocator);
	constants.insert(reference);
	constant_values.set(*reference, value);

	return *reference;
}

template <typename M>
void bmashina::BasicTree<M>::input(const detail::BaseReference& reference)
{
	inputs.insert(&reference);
}

template <typename M>
void bmashina::BasicTree<M>::output(const detail::BaseReference& reference)
{
	outputs.insert(&reference);
}

template <typename M>
void bmashina::BasicTree<M>::input(
	Node& node,
	const detail::BaseReference& from,
	const detail::BaseReference& to)
{
	assert(has(node));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(node))
	{
		throw std::runtime_error("node not in tree");
	}
#endif

	auto iter = node_inputs.find(&node);
	if (iter == node_inputs.end())
	{
		iter = node_inputs.emplace(&node, WireList::construct(*mashina)).first;
	}

	iter->second.push_back(std::make_tuple(&from, &to));
}

template <typename M>
void bmashina::BasicTree<M>::input(
	const Channel& channel,
	const detail::BaseReference& from,
	const detail::BaseReference& to)
{
	assert(has(channel));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(channel))
	{
		throw std::runtime_error("channel does not exist");
	}
#endif

	auto iter = channel_nodes.find(channel);
	input(*iter->second, from, to);
}

template <typename M>
void bmashina::BasicTree<M>::output(
	Node& node,
	const detail::BaseReference& from,
	const detail::BaseReference& to)
{
	assert(has(node));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(node))
	{
		throw std::runtime_error("node not in tree");
	}
#endif

	auto iter = node_outputs.find(&node);
	if (iter == node_outputs.end())
	{
		iter = node_outputs.emplace(&node, WireList::construct(*mashina)).first;
	}

	iter->second.push_back(std::make_tuple(&from, &to));
}

template <typename M>
void bmashina::BasicTree<M>::output(
	const Channel& channel,
	const detail::BaseReference& from,
	const detail::BaseReference& to)
{
	assert(has(channel));

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(channel))
	{
		throw std::runtime_error("channel does not exist");
	}
#endif

	auto iter = channel_nodes.find(channel);
	output(*iter->second, from, to);
}

template <typename M>
bmashina::BasicTree<M>::ChildrenIterator::ChildrenIterator(typename NodeList::Type::iterator value) :
	NodeList::Type::iterator(std::move(value))
{
	// Nothing.
}

template <typename M>
typename bmashina::BasicTree<M>::ChildrenIterator::reference_type
bmashina::BasicTree<M>::ChildrenIterator::operator *() const
{
	auto value = NodeList::Type::iterator::operator *();
	return *value;
}

template <typename M>
typename bmashina::BasicTree<M>::ChildrenIterator::pointer_type
bmashina::BasicTree<M>::ChildrenIterator::operator ->() const
{
	auto value = NodeList::Type::iterator::operator ->();
	return *value;
}

#endif
