// BMASHINA
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright 2017 [bk]door.maus

#ifndef BMASHINA_BUILDER_NATIVE_BUILDER_HPP
#define BMASHINA_BUILDER_NATIVE_BUILDER_HPP

#include "bmashina/tree.hpp"
#include "bmashina/node.hpp"
#include "bmashina/state/property.hpp"
#include "bmashina/state/reference.hpp"

namespace bmashina
{
	class NativeTreeBuilder
	{
	public:
		NativeTreeBuilder() = default;
		~NativeTreeBuilder() = default;

		template <typename M>
		class NativeTreeBuilderProxy;

		template <typename M>
		NativeTreeBuilderProxy<M> begin(BasicTree<M>& tree);

		template <typename M>
		class NativeTreeBuilderProxy
		{
		public:
			typedef M Mashina;
			typedef BasicTree<M> Tree;
			typedef BasicNode<M> Node;
			typedef typename Tree::Channel Channel;

			NativeTreeBuilderProxy(Tree& tree);
			~NativeTreeBuilderProxy() = default;

			template <typename V, typename... Arguments>
			NativeTreeBuilderProxy local(const Reference<V>*& reference, Arguments&&... arguments);

			template <typename V>
			NativeTreeBuilderProxy constant(const Reference<V>*& reference, const Property<V>& value);

			template <typename V>
			NativeTreeBuilderProxy inout(const Reference<V>& input);

			template <typename V>
			NativeTreeBuilderProxy in(const Reference<V>& input);

			template <typename V>
			NativeTreeBuilderProxy out(const Reference<V>& output);

			template <typename N, typename... Arguments>
			NativeTreeBuilderProxy child(Arguments&&... arguments);
			NativeTreeBuilderProxy child(Tree& tree);

			NativeTreeBuilderProxy channel(const Channel& channel);
			NativeTreeBuilderProxy channel(const Channel& channel, Tree& value);

			template <typename V>
			NativeTreeBuilderProxy inout(const Reference<V>& input_output, const Reference<V>& reference);

			template <typename V>
			NativeTreeBuilderProxy in(const Reference<V>& input, const Reference<V>& output);

			template <typename V>
			NativeTreeBuilderProxy out(const Reference<V>& output, const Reference<V>& input);

			NativeTreeBuilderProxy end();

		private:
			NativeTreeBuilderProxy* parent = nullptr;
			Tree* tree;
			Node* current_node = nullptr;
			Channel current_channel;
			bool is_channel = false;
			Tree* current_tree = nullptr;
		};
	};
}

template <typename M>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::begin(BasicTree<M>& tree)
{
	return NativeTreeBuilderProxy<M>(tree);
}

template <typename M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::NativeTreeBuilderProxy(Tree& tree) :
	tree(&tree)
{
	// Nothing.
}

template <typename M>
template <typename V, typename... Arguments>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::local(const Reference<V>*& reference, Arguments&&... arguments)
{
	assert(current_node == nullptr);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (current_node != nullptr)
	{
		throw std::runtime_error("can only create local at root level");
	}
#endif

	reference = &tree->template local<V>(std::forward<Arguments>(arguments)...);
	return *this;
}

template <typename M>
template <typename V>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::constant(const Reference<V>*& reference, const Property<V>& value)
{
	assert(current_node == nullptr);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (current_node != nullptr)
	{
		throw std::runtime_error("can only create local at root level");
	}
#endif

	reference = &tree->template constant<V>(value);
	return *this;
}

template <typename M>
template <typename V>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::inout(const Reference<V>& input_output)
{
	assert(current_node == nullptr);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (current_node != nullptr)
	{
		throw std::runtime_error("cannot bind singular inout to node");
	}
#endif

	tree->input(input_output);
	tree->output(input_output);

	return *this;
}

template <typename M>
template <typename V>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::in(const Reference<V>& input)
{
	assert(current_node == nullptr);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (current_node != nullptr)
	{
		throw std::runtime_error("cannot bind singular input to node");
	}
#endif

	tree->input(input);

	return *this;
}

template <typename M>
template <typename V>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::out(const Reference<V>& output)
{
	assert(current_node == nullptr);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (current_node != nullptr)
	{
		throw std::runtime_error("cannot bind singular input to node");
	}
#endif

	tree->output(output);

	return *this;
}

template <typename M>
template <typename N, typename... Arguments>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::child(Arguments&&... arguments)
{
	assert(!is_channel);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (is_channel)
	{
		throw std::runtime_error("channels cannot have children");
	}
#endif

	NativeTreeBuilderProxy result = *this;
	result.parent = this;
	if (current_node == nullptr)
	{
		result.current_node = &tree->template root<N>(std::forward<Arguments>(arguments)...);
	}
	else
	{
		result.current_node = &tree->template child<N>(*result.current_node, std::forward<Arguments>(arguments)...);
	}
	return result;
}

template <typename M>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::child(Tree& child_tree)
{
	assert(parent != nullptr);
	assert(!is_channel);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (parent == nullptr)
	{
		throw std::runtime_error("tree cannot be root node");
	}

	if (is_channel)
	{
		throw std::runtime_error("channels cannot have children");
	}
#endif

	NativeTreeBuilderProxy result = *this;
	result.parent = this;
	result.current_node = &tree->child(*result.current_node, child_tree);
	result.current_tree = &child_tree;
	return result;
}

template <typename M>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::channel(const Channel& channel)
{
	assert(parent != nullptr);
	assert(!is_channel);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
	if (parent == nullptr)
	{
		throw std::runtime_error("channel cannot be root node");
	}

	if (is_channel)
	{
		throw std::runtime_error("channels cannot have children");
	}
#endif

	tree->child(*current_node, channel);

	NativeTreeBuilderProxy result = *this;
	result.parent = this;
	result.current_node = nullptr;
	result.current_channel = channel;
	result.is_channel = true;
	return result;
}

template <typename M>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::channel(
	const Channel& channel, Tree& child_tree)
{
	if (current_tree == nullptr)
	{
		assert(parent == nullptr);

#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
		if (parent != nullptr)
		{
			throw std::runtime_error("can only assign channel at root level or after child tree");
		}
#endif

		tree->assign(channel, child_tree);
	}
	else
	{
		current_tree->assign(channel, child_tree);
	}

	return *this;
}

template <typename M>
template <typename V>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::inout(const Reference<V>& input_output, const Reference<V>& reference)
{
	if (is_channel)
	{
		tree->input(current_channel, input_output, reference);
		tree->output(current_channel, reference, input_output);
	}
	else
	{
#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
		if (current_node == nullptr)
		{
			throw std::runtime_error("cannot bind wired inout to tree");
		}
#endif

		tree->input(*current_node, input_output, reference);
		tree->output(*current_node, reference, input_output);
	}

	return *this;
}

template <typename M>
template <typename V>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::in(const Reference<V>& from, const Reference<V>& to)
{
	if (is_channel)
	{
		tree->input(current_channel, from, to);
	}
	else
	{
#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
		if (current_node == nullptr)
		{
			throw std::runtime_error("cannot bind wired input to tree");
		}
#endif

		tree->input(*current_node, from, to);
	}

	return *this;
}

template <typename M>
template <typename V>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::out(const Reference<V>& from, const Reference<V>& to)
{
	if (is_channel)
	{
		tree->output(current_channel, from, to);
	}
	else
	{
#ifndef BMASHINA_DISABLE_EXCEPTION_HANDLING
		if (current_node == nullptr)
		{
			throw std::runtime_error("cannot bind wired output to tree");
		}
#endif

		tree->output(*current_node, from, to);
	}

	return *this;
}

template <typename M>
typename bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>
bmashina::NativeTreeBuilder::NativeTreeBuilderProxy<M>::end()
{
	if (parent == nullptr)
	{
		return *this;
	}
	else
	{
		return *parent;
	}
}

#endif
