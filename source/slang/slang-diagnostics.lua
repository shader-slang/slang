-- Lua-based diagnostic definitions for the Slang compiler
-- This file defines diagnostics which will be processed
-- by slang-fiddle to generate C++ diagnostic structures and builders.

-- Schema definition:
-- {
--     name = "diagnostic name",        -- Space-separated name (becomes PascalCase in C++, snake-case for flags)
--     code = 12345,                    -- Integer diagnostic code
--     severity = "error",              -- "error", "warning", "note", "fatal"
--     message = "text with {param : type} interpolation",
--     primary_span = {
--         location = "location_name",  -- Name of location parameter
--         message = "span message",
--     },
--     secondary_spans = {              -- Optional additional spans
--         {
--             location = "location_name",
--             message = "span message",
--         },
--     },
-- }

-- Examples of inline type interpolation:
-- "invalid conversion from {from : type} to {to : type}"
-- "wrong number of arguments, expected {expected : int} got {got : int}"

diagnostics = {
	{
		name = "function return type mismatch",
		code = 30007,
		severity = "error",
		message = "expression type '{expression_type : type}' does not match function's return type '{return_type : type}'",
		primary_span = {
			location = "expression_location",
			message = "expression type",
		},
		secondary_spans = {
			{
				location = "function_location",
				message = "function return type",
			},
		},
	},
}
