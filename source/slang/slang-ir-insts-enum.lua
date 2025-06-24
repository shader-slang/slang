return {
	ir_enums = function()
		local insts = require("source/slang/slang-ir-insts.lua").insts
		local output = {}

		-- Post-order traversal
		local function traverse_post_order(tbl)
			local first_child = nil
			local last_child = nil
			local child_count = 0

			-- First, process all children
			for key, value in pairs(tbl) do
				if
					type(value) == "table"
					and key ~= "struct_name"
					and key ~= "min_operands"
					and key ~= "hoistable"
					and key ~= "parent"
					and key ~= "useOther"
					and key ~= "global"
				then
					-- Get struct name
					local struct_name = value.struct_name

					if value.is_leaf then
						-- Leaf instruction
						table.insert(output, "    kIROp_" .. struct_name .. ",")

						-- Track first and last child
						if first_child == nil then
							first_child = "kIROp_" .. struct_name
						end
						last_child = "kIROp_" .. struct_name
						child_count = child_count + 1
					else
						-- Parent instruction - recurse first
						local child_first, child_last = traverse_post_order(value)

						-- Track first and last child across all children
						if first_child == nil then
							first_child = child_first
						end
						if child_last then
							last_child = child_last
						end
						child_count = child_count + 1

						-- Then add parent entries
						if child_first and child_last then
							table.insert(output, "    kIROp_First" .. struct_name .. " = " .. child_first .. ",")
							table.insert(output, "    kIROp_Last" .. struct_name .. " = " .. child_last .. ",")
						end
					end
				end
			end

			return first_child, last_child
		end

		-- Start traversal
		traverse_post_order(insts)

		return table.concat(output, "\n")
	end,
}

-- dump(tostring(Slang.IRInst))
-- dump(isLeaf(Slang.IRInst))
-- dump(isLeaf(Slang.IRVoidType))
-- dump(Slang[tostring(Slang.IRInst)].directSubclasses)
-- dump(Slang.IRVoidType.directSubclasses)
