return {
	ir_enums = function()
		local insts = require("source/slang/slang-ir-insts.lua").insts
		local output = {}

		local function traverse(tbl)
			local first_child = nil
			local last_child = nil

			-- First, process all children
			for _, i in ipairs(tbl) do
				local key, value = next(i)
				if value.is_leaf then
					-- Leaf instruction
					table.insert(output, "    kIROp_" .. value.struct_name .. ",")

					-- Track first and last child
					if first_child == nil then
						first_child = "kIROp_" .. value.struct_name
					end
					last_child = "kIROp_" .. value.struct_name
				else
					-- Parent instruction - recurse first
					local child_first, child_last = traverse(value)

					-- Track first and last child across all children
					if first_child == nil then
						first_child = child_first
					end
					if child_last then
						last_child = child_last
					end

					-- Then add parent entries
					if child_first and child_last then
						table.insert(output, "    kIROp_First" .. value.struct_name .. " = " .. child_first .. ",")
						table.insert(output, "    kIROp_Last" .. value.struct_name .. " = " .. child_last .. ",")
					end
				end
			end

			return first_child, last_child
		end

		traverse(insts)
		return table.concat(output, "\n")
	end,
}
