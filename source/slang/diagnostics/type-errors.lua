-- diagnostics/type-errors.lua
-- Lua diagnostic descriptions for type-related errors
-- This file is processed by slang-fiddle to generate C++ diagnostic structures

-- Guinea pig diagnostic: non-overloaded function call type mismatch
-- This diagnostic is used to prototype the new multi-span diagnostic system
diagnostic "argument_type_mismatch" {
    code = "E30019",
    severity = "error",
    flag = "type-mismatch",

    -- Main error message
    message = "cannot convert argument of type `{found}` to parameter of type `{expected}`",

    -- Parameters that will be passed to the diagnostic
    params = {
        { name = "func_name", type = "String" },
        { name = "param_name", type = "String" },
        { name = "param_index", type = "int" },
        { name = "expected", type = "Type" },
        { name = "found", type = "Type" },
    },

    -- Primary label: marks the main error location
    primary_label = {
        loc = "arg_loc",
        message = "expected `{expected}`, found `{found}`",
    },

    -- Secondary labels: provide additional context
    secondary_labels = {
        {
            loc = "param_loc",
            message = "parameter `{param_name}` declared as `{expected}` here",
        },
        {
            loc = "func_loc",
            message = "in call to function `{func_name}`",
        },
    },

    -- Notes provide additional context
    notes = {
        "no implicit conversion exists from `{found}` to `{expected}`",
    },

    -- Helps suggest fixes
    helps = {
        "add explicit cast: `({expected}){found_expr}`",
    },
}