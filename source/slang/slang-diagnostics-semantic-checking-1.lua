-- Semantic checking diagnostics (part 1) - 3xxxx series
-- Converted from slang-diagnostic-defs.h lines 208-431

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 3xxxx - Semantic analysis
--

err(
    "divide by zero",
    30002,
    "divide by zero",
    span { loc = "location" }
)

err(
    "break outside loop",
    30003,
    "'break' must appear inside loop or switch constructs",
    span { loc = "stmt:Stmt", message = "'break' must appear inside loop or switch constructs." }
)

err(
    "continue outside loop",
    30004,
    "'continue' must appear inside loop constructs",
    span { loc = "stmt:Stmt", message = "'continue' must appear inside loop constructs." }
)

err(
    "return needs expression",
    30006,
    "'return' should have an expression",
    span { loc = "stmt:Stmt", message = "'return' should have an expression." }
)

err(
    "invalid type void",
    30009,
    "invalid type 'void'",
    span { loc = "location", message = "invalid type 'void'." }
)

err(
    "assign non lvalue",
    30011,
    "left of '=' is not an l-value",
    span { loc = "expr:Expr", message = "left of '=' is not an l-value." }
)

err(
    "call operator not found",
    30016,
    "no call operation found for type",
    span { loc = "expr:Expr", message = "no call operation found for type '~type:Type'" }
)

err(
    "undefined identifier 2",
    30015,
    "undefined identifier",
    span { loc = "location", message = "undefined identifier '~name:Name'." }
)

err(
    "invalid array size",
    30025,
    "array size must be non-negative",
    span { loc = "location", message = "array size must be non-negative." }
)

err(
    "disallowed array of non addressable type",
    30028,
    "arrays of non-addressable type not allowed",
    span { loc = "location", message = "Arrays of non-addressable type '~element_type:Type' are not allowed" }
)

err(
    "non addressable type in structured buffer",
    30028,
    "non-addressable type cannot be used in StructuredBuffer",
    span { loc = "location", message = "'~type:Type' is non-addressable and cannot be used in StructuredBuffer" }
)

err(
    "array index out of bounds",
    30029,
    "array index out of bounds",
    span { loc = "location", message = "array index '~index' is out of bounds for array of size '~size'." }
)

err(
    "no member of name in type",
    30027,
    "member not found",
    span { loc = "expr:Expr", message = "'~name:Name' is not a member of '~type:Type'." }
)

err(
    "argument expected lvalue",
    30047,
    "argument must be l-value",
    span { loc = "arg:Expr", message = "argument passed to parameter '~param' must be l-value." }
)

err(
    "argument has more memory qualifiers than param",
    30048,
    "memory qualifier mismatch",
    span { loc = "arg:Expr", message = "argument passed in to parameter has a memory qualifier the parameter type is missing: '~qualifier'" }
)

standalone_note(
    "attempting to assign to const variable",
    30049,
    "attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification",
    span { loc = "expr:Expr" }
)

err(
    "mutating method on immutable value",
    30050,
    "mutating method cannot be called on immutable value",
    span { loc = "location", message = "mutating method '~method_name:Name' cannot be called on an immutable value" }
)

err(
    "break label not found",
    30053,
    "break label not found",
    span { loc = "stmt:Stmt", message = "label '~label:Name' used as break target is not found." }
)

err(
    "target label does not mark breakable stmt",
    30054,
    "invalid break target",
    span { loc = "stmt:Stmt", message = "invalid break target: statement labeled '~label:Name' is not breakable." }
)

err(
    "use of non short circuiting operator in diff func",
    30055,
    "non-short-circuiting `?:` not allowed in differentiable function",
    span { loc = "location", message = "non-short-circuiting `?:` operator is not allowed in a differentiable function, use `select` instead." }
)

warning(
    "use of non short circuiting operator",
    30056,
    "non-short-circuiting `?:` is deprecated",
    span { loc = "location", message = "non-short-circuiting `?:` operator is deprecated, use 'select' instead." }
)

err(
    "assignment in predicate expr",
    30057,
    "assignment in predicate expression not allowed",
    span { loc = "expr:Expr", message = "use an assignment operation as predicate expression is not allowed, wrap the assignment with '()' to clarify the intent." }
)

warning(
    "dangling equality expr",
    30058,
    "result of '==' not used",
    span { loc = "expr:Expr", message = "result of '==' not used, did you intend '='?" }
)

err(
    "expected a type",
    30060,
    "expected a type",
    span { loc = "expr:Expr", message = "expected a type, got a '~what_we_got'" }
)

err(
    "expected a namespace",
    30061,
    "expected a namespace",
    span { loc = "expr:Expr", message = "expected a namespace, got a '~expr.type'" }
)

standalone_note(
    "implicit cast used as lvalue ref",
    30062,
    "argument was implicitly cast from '~from:Type' to '~to:Type', and Slang does not support using an implicit cast as an l-value with a reference",
    span { loc = "expr:Expr" }
)

standalone_note(
    "implicit cast used as lvalue type",
    30063,
    "argument was implicitly cast from '~from:Type' to '~to:Type', and Slang does not support using an implicit cast as an l-value with this type",
    span { loc = "expr:Expr" }
)

standalone_note(
    "implicit cast used as lvalue",
    30064,
    "argument was implicitly cast from '~from:Type' to '~to:Type', and Slang does not support using an implicit cast as an l-value for this usage",
    span { loc = "expr:Expr" }
)

err(
    "new can only be used to initialize a class",
    30065,
    "`new` can only be used to initialize a class",
    span { loc = "expr:Expr", message = "`new` can only be used to initialize a class" }
)

err(
    "class can only be initialized with new",
    30066,
    "class can only be initialized by `new`",
    span { loc = "expr:Expr", message = "a class can only be initialized by a `new` clause" }
)

err(
    "mutating method on function input parameter error",
    30067,
    "mutating method called on `in` parameter",
    span { loc = "location", message = "mutating method '~method:Name' called on `in` parameter '~param:Name'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended" }
)

warning(
    "mutating method on function input parameter warning",
    30068,
    "mutating method called on `in` parameter",
    span { loc = "location", message = "mutating method '~method:Name' called on `in` parameter '~param:Name'; changes will not be visible to caller. copy the parameter into a local variable if this behavior is intended" }
)

err(
    "unsized member must appear last",
    30070,
    "unsized member must be last",
    span { loc = "decl:Decl", message = "member with unknown size at compile time can only appear as the last member in a composite type." }
)

err(
    "var cannot be unsized",
    30071,
    "cannot instantiate unsized type",
    span { loc = "decl:Decl", message = "cannot instantiate a variable of unsized type." }
)

err(
    "param cannot be unsized",
    30072,
    "function parameter cannot be unsized",
    span { loc = "decl:Decl", message = "function parameter cannot be unsized." }
)

err(
    "cannot specialize generic",
    30075,
    "cannot specialize generic",
    span { loc = "location", message = "cannot specialize generic '~generic:Decl' with the provided arguments." }
)

err(
    "global var cannot have opaque type",
    30076,
    "global variable cannot have opaque type",
    span { loc = "decl:Decl", message = "global variable cannot have opaque type." }
)

err(
    "concrete argument to output interface",
    30077,
    "concrete type passed to interface-typed output parameter",
    span { loc = "location", message = "argument passed to parameter '~param_name' is of concrete type '~arg_type:QualType', but interface-typed output parameters require interface-typed arguments. To allow passing a concrete type to this function, you can replace '~param_type:Type ~param_name' with a generic 'T ~param_name' and a 'where T : ~param_type:Type' constraint." }
)

standalone_note(
    "do you mean static const",
    -1,
    "do you intend to define a `static const` instead?",
    span { loc = "decl:Decl" }
)

standalone_note(
    "do you mean uniform",
    -1,
    "do you intend to define a `uniform` parameter instead?",
    span { loc = "decl:Decl" }
)

err(
    "coherent keyword on a pointer",
    30078,
    "cannot have coherent qualifier on pointer",
    span { loc = "decl:Decl", message = "cannot have a `globallycoherent T*` or a `coherent T*`, use explicit methods for coherent operations instead" }
)

err(
    "cannot take constant pointers",
    30079,
    "cannot take address of immutable object",
    span { loc = "expr:Expr", message = "Not allowed to take the address of an immutable object" }
)

err(
    "cannot specialize generic with existential",
    33180,
    "cannot specialize generic with existential type",
    span { loc = "location", message = "specializing '~generic' with an existential type is not allowed. All generic arguments must be statically resolvable at compile time." }
)

err(
    "static ref to non static member",
    30100,
    "type cannot refer to non-static member",
    span { loc = "location", message = "type '~type:Type' cannot be used to refer to non-static member '~member:Name'" }
)

err(
    "cannot dereference type",
    30101,
    "cannot dereference type",
    span { loc = "location", message = "cannot dereference type '~type:Type', do you mean to use '.'?" }
)

err(
    "static ref to this",
    30102,
    "static function cannot refer to non-static member via `this`",
    span { loc = "location", message = "static function cannot refer to non-static member `~member:Name` via `this`" }
)

err(
    "redeclaration",
    30200,
    "conflicting declaration",
    span { loc = "decl:Decl", message = "declaration of '~decl' conflicts with existing declaration" }
)

err(
    "function redeclaration with different return type",
    30202,
    "function return type mismatch",
    span { loc = "decl:Decl", message = "function '~decl' declared to return '~new_return_type:Type' was previously declared to return '~prev_return_type:Type'" }
)

err(
    "is operator value must be interface type",
    30300,
    "'is'/'as' operator requires interface-typed expression",
    span { loc = "expr:Expr", message = "'is'/'as' operator requires an interface-typed expression." }
)

err(
    "is operator cannot use interface as rhs",
    30301,
    "cannot use 'is' with interface on right-hand side",
    span { loc = "expr:Expr", message = "cannot use 'is' operator with an interface type as the right-hand side without a corresponding optional constraint. Use a concrete type instead, or add an optional constraint for the interface type." }
)

err(
    "as operator cannot use interface as rhs",
    30302,
    "cannot use 'as' with interface on right-hand side",
    span { loc = "expr:Expr", message = "cannot use 'as' operator with an interface type as the right-hand side. Use a concrete type instead. If you want to use an optional constraint, use an 'if (T is IInterface)' block instead." }
)

err(
    "expected function",
    33070,
    "expected a function",
    span { loc = "expr:Expr", message = "expected a function, got '~expr.type'" }
)

err(
    "expected a string literal",
    33071,
    "expected a string literal",
    span { loc = "expr:Expr", message = "expected a string literal" }
)

end
