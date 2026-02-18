-- Semantic checking diagnostics (part 2) - 3xxxx series
-- Converted from slang-diagnostic-defs.h lines 102-302

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 3xxxx - Semantic analysis (continued)
--

-- `dyn` and `some` errors

err(
    "cannot have generic dyn interface",
    33072,
    "dyn interfaces cannot be generic",
    span { loc = "decl:Decl", message = "dyn interfaces cannot be generic: '~decl'." }
)

err(
    "cannot have associated type in dyn interface",
    33073,
    "dyn interfaces cannot have associatedType members",
    span { loc = "member:Decl", message = "dyn interfaces cannot have associatedType members." }
)

err(
    "cannot have generic method in dyn interface",
    33074,
    "dyn interfaces cannot have generic methods",
    span { loc = "member:Decl", message = "dyn interfaces cannot have generic methods." }
)

err(
    "cannot have mutating method in dyn interface",
    33075,
    "dyn interfaces cannot have [mutating] methods",
    span { loc = "member:Decl", message = "dyn interfaces cannot have [mutating] methods." }
)

err(
    "cannot have differentiable method in dyn interface",
    33076,
    "dyn interfaces cannot have [Differentiable] methods",
    span { loc = "member:Decl", message = "dyn interfaces cannot have [Differentiable] methods." }
)

err(
    "dyn interface cannot inherit non dyn interface",
    33077,
    "dyn interface inheritance error",
    span { loc = "inheritance:Decl", message = "dyn interface '~dyn_interface:Decl' may only inherit 'dyn' interfaces. '~inherited:Decl' is not a dyn interface." }
)

err(
    "cannot use extension to make type conform to dyn interface",
    33078,
    "extension cannot conform to dyn interface",
    span { loc = "extension:Decl", message = "cannot use a extension to conform to a dyn interface '~interface:Decl'." }
)

err(
    "cannot have unsized member when inheriting dyn interface",
    33079,
    "unsized member with dyn interface inheritance",
    span { loc = "member:Decl", message = "cannot have unsized member '~member' when inheriting from dyn interface '~interface:Decl'." }
)

err(
    "cannot have opaque member when inheriting dyn interface",
    33080,
    "opaque member with dyn interface inheritance",
    span { loc = "member:Decl", message = "cannot have opaque member '~member' when inheriting from dyn interface '~interface:Decl'." }
)

err(
    "cannot have non copyable member when inheriting dyn interface",
    33081,
    "non-copyable member with dyn interface inheritance",
    span { loc = "member:Decl", message = "cannot have non-copyable member '~member' when inheriting from dyn interface '~interface:Decl'." }
)

err(
    "cannot conform generic to dyn interface",
    33082,
    "generic type cannot conform to dyn interface",
    span { loc = "inheritance:Decl", message = "cannot conform generic type '~generic:Decl' to dyn interface '~interface:Decl'." }
)

-- Conversion diagnostics
-- Note: noteExplicitConversionPossible is kept in slang-diagnostic-defs.h because
-- it's used with diagnoseWithoutSourceView which doesn't support rich diagnostics

err(
    "ambiguous conversion",
    30080,
    "ambiguous conversion",
    span { loc = "expr:Expr", message = "more than one conversion exists from '~from_type:Type' to '~to_type:Type'" }
)

warning(
    "unrecommended implicit conversion",
    30081,
    "implicit conversion not recommended",
    span { loc = "expr:Expr", message = "implicit conversion from '~from_type:Type' to '~to_type:Type' is not recommended" }
)

warning(
    "implicit conversion to double",
    30082,
    "implicit float-to-double conversion",
    span { loc = "expr:Expr", message = "implicit float-to-double conversion may cause unexpected performance issues, use explicit cast if intended." }
)

-- try/throw diagnostics

err(
    "try clause must apply to invoke expr",
    30090,
    "expression in 'try' must be a call",
    span { loc = "expr:Expr", message = "expression in a 'try' clause must be a call to a function or operator overload." }
)

err(
    "try invoke callee should throw",
    30091,
    "callee in 'try' does not throw",
    span { loc = "expr:Expr", message = "'~callee:Decl' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'" }
)

err(
    "callee of try call must be func",
    30092,
    "callee in 'try' must be a function",
    span { loc = "expr:Expr", message = "callee in a 'try' clause must be a function" }
)

err(
    "uncaught try call in non throw func",
    30093,
    "uncaught 'try' in non-throwing function",
    span { loc = "expr:Expr", message = "the current function or environment is not declared to throw any errors, but the 'try' clause is not caught" }
)

err(
    "must use try clause to call a throw func",
    30094,
    "callee may throw, use 'try'",
    span { loc = "invoke:Expr", message = "the callee may throw an error, and therefore must be called within a 'try' clause" }
)

err(
    "error type of callee incompatible with caller",
    30095,
    "incompatible error types",
    span { loc = "expr:Expr", message = "the error type `~callee_error_type:Type` of callee `~callee:Decl` is not compatible with the caller's error type `~caller_error_type:Type`." }
)

-- Differentiable diagnostics

err(
    "differential type should serve as its own differential type",
    30096,
    "invalid differential type",
    span { loc = "inheritance:Decl", message = "cannot use type '~type:Type' as a `Differential` type. A differential type's differential must be itself. However, the Differential of '~type:Type' is '~diff_type:Type'." }
)

err(
    "function not marked as differentiable",
    30097,
    "function not differentiable",
    span { loc = "expr:Expr", message = "function '~func:Decl' is not marked as ~kind-differentiable." }
)

err(
    "non static member function not allowed as diff operand",
    30098,
    "non-static function reference not allowed",
    span { loc = "expr:Expr", message = "non-static function reference '~func:Decl' is not allowed here." }
)

-- sizeof/countof diagnostics

err(
    "size of argument is invalid",
    30099,
    "invalid sizeof argument",
    span { loc = "expr:Expr", message = "argument to sizeof is invalid" }
)

err(
    "count of argument is invalid",
    30083,
    "invalid countof argument",
    span { loc = "expr:Expr", message = "argument to countof can only be a type pack or tuple" }
)

-- Float bit cast diagnostics

err(
    "float bit cast type mismatch",
    30084,
    "bit cast type mismatch",
    span { loc = "expr:Expr", message = "'~intrinsic' requires a ~expected_type argument" }
)

err(
    "float bit cast requires constant",
    30085,
    "bit cast requires constant",
    span { loc = "expr:Expr", message = "'__floatAsInt' requires a compile-time constant floating-point expression" }
)

-- writeonly diagnostic

err(
    "reading from write only",
    30101,
    "cannot read from writeonly",
    span { loc = "expr:Expr", message = "cannot read from writeonly, check modifiers." }
)

-- differentiable member diagnostic

err(
    "differentiable member should have corresponding field in diff type",
    30102,
    "missing field in differential type",
    span { loc = "location", message = "differentiable member '~member:Name' should have a corresponding field in '~diff_type:Type'. Use [DerivativeMember(...)] or mark as no_diff" }
)

-- type pack diagnostics

err(
    "expect type pack after each",
    30103,
    "expected type pack after 'each'",
    span { loc = "expr:Expr", message = "expected a type pack or a tuple after 'each'." }
)

err(
    "each expr must be inside expand expr",
    30104,
    "'each' must be inside 'expand'",
    span { loc = "expr:Expr", message = "'each' expression must be inside 'expand' expression." }
)

err(
    "expand term captures no type packs",
    30105,
    "'expand' captures no type packs",
    span { loc = "expr:Expr", message = "'expand' term captures no type packs. At least one type pack must be referenced via an 'each' term inside an 'expand' term." }
)

err(
    "improper use of type",
    30106,
    "type cannot be used in this context",
    span { loc = "expr:Expr", message = "type '~type:Type' cannot be used in this context." }
)

err(
    "parameter pack must be const",
    30107,
    "parameter pack must be 'const'",
    span { loc = "modifier:Modifier", message = "a parameter pack must be declared as 'const'." }
)

-- defer diagnostics

err(
    "break inside defer",
    30108,
    "'break' inside defer",
    span { loc = "stmt:Stmt", message = "'break' must not appear inside a defer statement." }
)

err(
    "continue inside defer",
    30109,
    "'continue' inside defer",
    span { loc = "stmt:Stmt", message = "'continue' must not appear inside a defer statement." }
)

err(
    "return inside defer",
    30110,
    "'return' inside defer",
    span { loc = "stmt:Stmt", message = "'return' must not appear inside a defer statement." }
)

-- lambda diagnostics

err(
    "return type mismatch inside lambda",
    30111,
    "lambda return type mismatch",
    span { loc = "stmt:Stmt", message = "returned values must have the same type among all 'return' statements inside a lambda expression: returned '~returned_type:Type' here, but '~previous_type:Type' previously." }
)

err(
    "non copyable type captured in lambda",
    30112,
    "non-copyable type captured",
    span { loc = "expr:Expr", message = "cannot capture non-copyable type '~type:Type' in a lambda expression." }
)

-- uncaught throw diagnostics

err(
    "uncaught throw inside defer",
    30113,
    "'throw' requires 'catch' inside defer",
    span { loc = "stmt:Stmt", message = "'throw' expressions require a matching 'catch' inside a defer statement." }
)

err(
    "uncaught try inside defer",
    30114,
    "'try' requires 'catch' inside defer",
    span { loc = "expr:Expr", message = "'try' expressions require a matching 'catch' inside a defer statement." }
)

err(
    "uncaught throw in non throw func",
    30115,
    "uncaught 'throw' in non-throwing function",
    span { loc = "stmt:Stmt", message = "the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement." }
)

err(
    "throw type incompatible with error type",
    30116,
    "throw type incompatible with error type",
    span { loc = "expr:Expr", message = "the type `~throw_type:Type` of `throw` is not compatible with function's error type `~error_type:Type`." }
)

err(
    "forward reference in generic constraint",
    30117,
    "forward reference in generic constraint",
    span { loc = "expr:Expr", message = "generic constraint for parameter '~param:Type' references type parameter '~referenced:Decl' before it is declared" }
)

end
