-- Semantic checking diagnostics (part 8) - Accessors, Bit Fields, Integer Constants, Overloads, Switch, Generics, Ambiguity
-- Converted from slang-diagnostic-defs.h lines 135-227

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 311xx: accessors
--
err(
    "accessor must be inside subscript or property",
    31100,
    "invalid accessor declaration location",
    span { loc = "decl:Decl", message = "an accessor declaration is only allowed inside a subscript or property declaration" }
)

err(
    "non set accessor must not have params",
    31101,
    "accessors other than 'set' must not have parameters",
    span { loc = "decl:Decl", message = "accessors other than 'set' must not have parameters" }
)

err(
    "set accessor may not have more than one param",
    31102,
    "a 'set' accessor may not have more than one parameter",
    span { loc = "param:Decl", message = "a 'set' accessor may not have more than one parameter" }
)

err(
    "set accessor param wrong type",
    31102,
    "'set' parameter type mismatch",
    span { loc = "param:Decl", message = "'set' parameter '~param' has type '~actual_type:Type' which does not match the expected type '~expected_type:Type'" }
)

--
-- 313xx: bit fields
--
err(
    "bit field too wide",
    31300,
    "bit-field size exceeds type width",
    span { loc = "location", message = "bit-field size (~field_width:Int) exceeds the width of its type ~type:Type (~type_width:Int)" }
)

err(
    "bit field non integral",
    31301,
    "bit-field type must be integral",
    span { loc = "location", message = "bit-field type (~type:Type) must be an integral type" }
)

--
-- 39999: waiting to be placed in the right range
--
err(
    "expected integer constant not constant",
    39999,
    "expression does not evaluate to a compile-time constant",
    span { loc = "expr:Expr", message = "expression does not evaluate to a compile-time constant" }
)

err(
    "expected integer constant not literal",
    39999,
    "could not extract value from integer constant",
    span { loc = "location", message = "could not extract value from integer constant" }
)

err(
    "expected ray tracing payload object at location but missing",
    39999,
    "raytracing payload missing",
    span { loc = "location", message = "raytracing payload expected at location ~payload_location:Int but it is missing" }
)

err(
    "no applicable overload for name with args",
    39999,
    "no overload for '~name:Name' applicable to arguments of type ~args",
    span { loc = "expr:Expr", message = "no overload for '~name' applicable to arguments of type ~args" }
)

err(
    "no applicable with args",
    39999,
    "no overload applicable to arguments of type ~args",
    span { loc = "expr:Expr", message = "no overload applicable to arguments of type ~args" }
)

-- Note: "ambiguous overload for name with args" already exists in slang-diagnostics.lua
-- with rich diagnostic support (variadic_note). The old diagnostic in slang-diagnostic-defs.h
-- (ambiguousOverloadForNameWithArgs) is kept for backward compatibility when rich diagnostics
-- are not enabled. See slang-check-overload.cpp:2981-3015 for usage.

err(
    "ambiguous overload with args",
    39999,
    "ambiguous call to overloaded operation with arguments of type ~args",
    span { loc = "expr:Expr", message = "ambiguous call to overloaded operation with arguments of type ~args" }
)

standalone_note(
    "overload candidate",
    39999,
    "candidate: ~candidate",
    span { loc = "location" }
)

standalone_note(
    "invisible overload candidate",
    39999,
    "candidate (invisible): ~candidate",
    span { loc = "location" }
)

standalone_note(
    "more overload candidates",
    39999,
    "~count:Int more overload candidates",
    span { loc = "location" }
)

err(
    "case outside switch",
    39999,
    "'case' not allowed outside of a 'switch' statement",
    span { loc = "stmt:Stmt", message = "'case' not allowed outside of a 'switch' statement" }
)

err(
    "default outside switch",
    39999,
    "'default' not allowed outside of a 'switch' statement",
    span { loc = "stmt:Stmt", message = "'default' not allowed outside of a 'switch' statement" }
)

err(
    "expected a generic",
    39999,
    "expected a generic when using '<...>'",
    span { loc = "expr:Expr", message = "expected a generic when using '<...>' (found: '~found:Type')" }
)

err(
    "generic argument inference failed",
    39999,
    "could not specialize generic for arguments of type ~args",
    span { loc = "location", message = "could not specialize generic for arguments of type ~args" }
)

err(
    "ambiguous reference",
    39999,
    "ambiguous reference to '~name'",
    span { loc = "location", message = "ambiguous reference to '~name'" }
)

err(
    "ambiguous expression",
    39999,
    "ambiguous reference",
    span { loc = "expr:Expr", message = "ambiguous reference" }
)

err(
    "declaration didnt declare anything",
    39999,
    "declaration does not declare anything",
    span { loc = "location", message = "declaration does not declare anything" }
)

end
