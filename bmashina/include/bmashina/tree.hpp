// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_TREE_HPP
#define BMASHINA_TREE_HPP

#include "bmashina/channel.hpp"
#include "bmashina/composite.hpp"
#include "bmashina/config.hpp"
#include "bmashina/decorator.hpp"
#include "bmashina/executor.hpp"
#include "bmashina/node.hpp"
#include "bmashina/status.hpp"

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
		typedef typename BasicChannel<Mashina>::Type Channel;
		typedef BasicNode<Mashina> Node;
		typedef BasicComposite<Mashina> Composite;
		typedef BasicDecorator<Mashina> Decorator;

		BasicTree(Mashina& mashina);
		BasicTree(const Tree& other) = delete;
		~BasicTree();

		bool parented() const;
		bool compatible(const Tree& other) const;

		template <typename N, typename... Arguments>
		N& root(Arguments&&... arguments);

		template <typename N, typename... Arguments>
		N& child(Node& parent, Arguments&&... arguments);
		void child(Node& parent, const Channel& channel);

		void assign(const Channel& channel, Tree& tree);
		bool assigned(const Channel& channel) const;
		void unassign(const Channel& channel);

		bool has(Node& node) const;
		bool has(const Channel& channel) const;

		void clear();
		bool empty() const;

		Status execute(Executor& execute);

		Tree& operator =(const Tree& other) = delete;

	private:
		Mashina* mashina;

		typedef typename Allocator<Mashina>::Type AllocatorType;
		AllocatorType allocator;

		Tree* parent = nullptr;

		typedef UnorderedSet<Mashina, Node*> NodeSet;
		typename NodeSet::Type nodes;
		Node* root_node = nullptr;

		template <typename N, typename... Arguments>
		N* create(Arguments&&... arguments);

		typedef UnorderedSet<Mashina, Channel> ChannelSet;
		typename ChannelSet::Type channels;

		typedef UnorderedMap<Mashina, Channel, Tree*> ChannelAssignments;
		typename ChannelAssignments::Type assignments;

		typedef Vector<Mashina, Node*> NodeList;
		typename NodeList::Type empty_node_list;

		typedef UnorderedMap<Mashina, Node*, typename NodeList::Type> CompositeChildren;
		typename CompositeChildren::Type children;
		typename NodeList::Type& get_children(Node& node);

		class ChannelProxyNode : public Node
		{
		public:
			ChannelProxyNode(const Channel& channel);
			~ChannelProxyNode() = default;

		protected:
			void preupdate(Mashina& mashina) override;
			void postupdate(Mashina& mashina) override;
			Status update(Mashina& mashina) override;

		private:
			Channel channel;
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
	assignments(ChannelAssignments::construct(mashina)),
	children(CompositeChildren::construct(mashina)),
	empty_node_list(NodeList::construct(mashina))
{
	// Nothing.
}

template <typename M>
bmashina::BasicTree<M>::~BasicTree()
{
	clear();
}

template <typename M>
bool bmashina::BasicTree<M>::parented() const
{
	return parent != nullptr;
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
}

template <typename M>
void bmashina::BasicTree<M>::assign(const Channel& channel, Tree& tree)
{
	assert(has(channel));
	assert(compatible(tree));
	assert(!tree.parented());

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (!has(channel))
	{
		throw std::runtime_error("channel does not exist");
	}

	if (!compatible(tree))
	{
		throw std::runtime_error("assigning incompatible tree to channel");
	}

	if (tree.parented())
	{
		throw std::runtime_error("trying to assign previously parented tree");
	}
#endif

	if (assigned(channel))
	{
		unassign(channel);
	}

	assignments.emplace(channel, &tree);
	tree.parent = this;
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

	return assignments.count(channel) != 0;
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

	auto iter = assignments.find(channel);
	iter->second->parent = nullptr;
	assignments.erase(iter);
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

	root_node = nullptr;

	channels.clear();
	assignments.clear();
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
	{
		result = root_node->update(executor);
	}
	executor.leave(*this);

	return result;
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
void bmashina::BasicTree<M>::ChannelProxyNode::preupdate(Mashina& mashina)
{
	auto iter = Node::tree().assignments.find(channel);
	if (iter != Node::tree().assignments.end() && !iter->second->empty())
	{
		return iter->second->root_node->before_step(mashina);
	}
}

template <typename M>
void bmashina::BasicTree<M>::ChannelProxyNode::postupdate(Mashina& mashina)
{
	auto iter = Node::tree().assignments.find(channel);
	if (iter != Node::tree().assignments.end() && !iter->second->empty())
	{
		return iter->second->root_node->after_step(mashina);
	}
}

template <typename M>
bmashina::Status bmashina::BasicTree<M>::ChannelProxyNode::update(Mashina& mashina)
{
	auto iter = Node::tree().assignments.find(channel);
	if (iter != Node::tree().assignments.end() && !iter->second->empty())
	{
		return iter->second->root_node->step(mashina);
	}

	return bmashina::Status::failure;
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
