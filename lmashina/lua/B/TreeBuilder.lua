--------------------------------------------------------------------------------
-- B/TreeBuilder.lua
--
-- This file is a part of BMASHINA.
--
-- This Source Code Form is subject to the terms of the Mozilla Public
-- License, v. 2.0. If a copy of the MPL was not distributed with this
-- file, You can obtain one at http://mozilla.org/MPL/2.0/.
--------------------------------------------------------------------------------
local B = require "B"

local TreeBuilder = {}
local TreeBuilderNode = {}

local function import(path, args, aliases)
	local real_path = { B._ROOT }
	local last_component = ""
	for i, m in ipairs(path) do
		table.insert(real_path, m)
	end

	local success, result = xpcall(require, debug.traceback, table.concat(real_path, "."))
	if success then
		return result
	else
		error(string.format("Failed to import node %s.", table.concat(path, ".")))
	end
end

local function materialize(tree, definition, class, node, aliases, e, depth)
	e = e or {}
	depth = depth or 0
	if e[definition] and not e.warned then
		if B._DEBUG then
			io.stderr:write("warning: tree possibly recursive\n")
		end
		e.warned = true
	else
		e[definition] = definition
	end

	depth = depth + 1

	local children = definition._arguments
	for i = 1, #children do
		if getmetatable(children[i]) ~= TreeBuilderNode then
			error(string.format("child %d of node %s is not valid", i, table.concat(definition[i]._path)))
		end

		local c = import(children[i]._path, children[i]._arguments, aliases)
		local n = tree:child(node, c)
		materialize(tree, children[i], c, n, aliases, e, depth)
	end

	local wires = definition._arguments
	for key, value in pairs(wires) do
		local v
		local s
		if B.is_reference(value) then
			v = value
			s = B.get_wire_name(v)
		elseif B.is_local(value) then
			v = aliases[value] or value
			s = B.get_wire_name(v)
		elseif getmetatable(value) == B.Output.Type then
			if not value._table then
				v = class[value._key:upper()]
				s = B.get_wire_name(v)
			end
		elseif type(key) ~= 'number' then
			v = tree:constant(value)
			s = tostring(value)
		end

		if v then
			if B.is_reference(key) then
				tree:output(node, v, key)
			elseif B.is_local(key) then
				tree:output(node, v, aliases[key] or key)
			elseif type(key) == "string" then
				local k = class[key:upper()]
				if B.is_reference(k) then
					tree:input(node, v, k)
				end
			end
		end
	end
end

function TreeBuilderNode:__index(key)
	return TreeBuilder.Node(self._path, key)
end

function TreeBuilderNode:__call(args)
	if type(args) ~= "table" then
		errorf("expected table (%s)", table.concat(self._path, "."))
	end

	for key, value in pairs(args) do
		if type(key) ~= "number" then
			self._arguments[key] = value
		end
	end

	for index, value in ipairs(args) do
		table.insert(self._arguments, value)
	end

	return self
end

TreeBuilder.Type = TreeBuilderNode

function TreeBuilder.Root()
	return TreeBuilder.Node()
end

function TreeBuilder.Node(parent, name)
	parent = parent or {}

	local r = {
		_path = { unpack(parent) },
		_children = {},
		_arguments = {}
	}

	table.insert(r._path, name)

	return setmetatable(r, TreeBuilderNode)
end

function TreeBuilder.materialize(mashina, node, aliases)
	if getmetatable(node) ~= TreeBuilderNode then
		error(debug.traceback("expected TreeBuilder"))
	end

	aliases = aliases or {}

	local tree = B.Tree(mashina)
	do
		local root = node._arguments[1]
		local class = import(root._path, root._arguments, aliases)
		local root = tree:child(class)
		materialize(tree, node, class, root, aliases)
	end

	return tree
end

return TreeBuilder
