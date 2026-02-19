-- Semantic checking diagnostics (part 6) - Differentiation, Modifiers, GLSL/HLSL specifics,
-- Interfaces, Control flow, Enums, Generics, and misc errors
-- Converted from slang-diagnostic-defs.h lines 127-400

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning
local fatal = helpers.fatal or function(name, code, message, primary_span, ...)
    helpers.add_diagnostic(name, code, "fatal", message, primary_span, ...)
end

-- 3115x-3116x - Differentiation continued

err(
    "primal substitute target must have higher differentiability level",
    31158,
    "primal substitute requires differentiable target",
    span { loc = "attr:Modifier", message = "primal substitute function for differentiable method must also be differentiable. Use [Differentiable] or [TreatAsDifferentiable] (for empty derivatives)" }
)

warning(
    "no derivative on non differentiable this type",
    31159,
    "no derivative for member on non-differentiable struct",
    span { loc = "member:Expr", message = "There is no derivative calculated for member '~member_decl:Decl' because the parent struct is not differentiable. If this is intended, consider using [NoDiffThis] on the function '~func:Decl' to suppress this warning. Alternatively, users can mark the parent struct as [Differentiable] to propagate derivatives." }
)

err(
    "invalid address of",
    31160,
    "invalid __getAddress usage",
    span { loc = "expr:Expr", message = "'__getAddress' only supports groupshared variables and members of groupshared/device memory." }
)

-- 312xx - Modifiers and Deprecation

warning(
    "deprecated usage",
    31200,
    "use of deprecated declaration",
    span { loc = "location", message = "~decl_name:Name has been deprecated: ~message" }
)

err(
    "modifier not allowed",
    31201,
    "modifier not allowed",
    span { loc = "modifier:Modifier", message = "modifier '~modifier' is not allowed here." }
)

err(
    "duplicate modifier",
    31202,
    "duplicate modifier",
    span { loc = "modifier:Modifier", message = "modifier '~modifier' is redundant or conflicting with existing modifier '~existing_modifier:Modifier'" }
)

err(
    "cannot export incomplete type",
    31203,
    "cannot export incomplete type",
    span { loc = "decl:Decl", message = "cannot export incomplete type '~decl'" }
)

err(
    "memory qualifier not allowed on a non image type parameter",
    31206,
    "invalid memory qualifier",
    span { loc = "param:Decl", message = "modifier ~modifier:Modifier is not allowed on a non image type parameter." }
)

err(
    "require input decorated var for parameter",
    31208,
    "shader input required",
    span { loc = "expr:Expr", message = "~func:Decl expects for argument ~param_number:Int a type which is a shader input (`in`) variable." }
)

-- 3121x - Derivative group requirements

err(
    "derivative group quad must be multiple 2 for xy threads",
    31210,
    "derivative group quad thread count error",
    span { loc = "location", message = "compute derivative group quad requires thread dispatch count of X and Y to each be at a multiple of 2" }
)

err(
    "derivative group linear must be multiple 4 for total thread count",
    31211,
    "derivative group linear thread count error",
    span { loc = "location", message = "compute derivative group linear requires total thread dispatch count to be at a multiple of 4" }
)

err(
    "only one of derivative group linear or quad can be set",
    31212,
    "conflicting derivative group settings",
    span { loc = "location", message = "cannot set compute derivative group linear and compute derivative group quad at the same time" }
)

-- CUDA

err(
    "cuda kernel must return void",
    31213,
    "CUDA kernel return type error",
    span { loc = "decl:Decl", message = "return type of a CUDA kernel function cannot be non-void." }
)

err(
    "differentiable kernel entry point cannot have differentiable params",
    31214,
    "differentiable kernel param restriction",
    span { loc = "param:Decl", message = "differentiable kernel entry point cannot have differentiable parameters. Consider using DiffTensorView to pass differentiable data, or marking this parameter with 'no_diff'" }
)

err(
    "cannot use unsized type in constant buffer",
    31215,
    "unsized type in constant buffer",
    span { loc = "field:Decl", message = "cannot use unsized type '~type:Type' in a constant buffer." }
)

-- GLSL layout qualifiers

err(
    "unrecognized glsl layout qualifier",
    31216,
    "unrecognized GLSL layout qualifier",
    span { loc = "attr:Modifier", message = "GLSL layout qualifier is unrecognized" }
)

err(
    "unrecognized glsl layout qualifier or requires assignment",
    31217,
    "unrecognized GLSL layout qualifier",
    span { loc = "location", message = "GLSL layout qualifier is unrecognized or requires assignment" }
)

-- Specialization and push constants

err(
    "specialization constant must be scalar",
    31218,
    "specialization constant type error",
    span { loc = "modifier:Modifier", message = "specialization constant must be a scalar." }
)

err(
    "push or specialization constant cannot be static",
    31219,
    "push/specialization constant storage class error",
    span { loc = "decl:Decl", message = "push or specialization constants cannot be 'static'." }
)

err(
    "variable cannot be push and specialization constant",
    31220,
    "conflicting constant qualifiers",
    span { loc = "decl:Decl", message = "'~decl' cannot be a push constant and a specialization constant at the same time" }
)

-- HLSL register names

err(
    "invalid hlsl register name",
    31221,
    "invalid HLSL register name",
    span { loc = "location", message = "invalid HLSL register name '~register_name'." }
)

err(
    "invalid hlsl register name for type",
    31222,
    "invalid HLSL register name for type",
    span { loc = "location", message = "invalid HLSL register name '~register_name' for type '~type:Type'." }
)

-- Extern/export and const variables

err(
    "extern and export var decl must be const",
    31223,
    "extern/export requires static const",
    span { loc = "decl:Decl", message = "extern and export variables must be static const: '~decl'" }
)

err(
    "const global var with init requires static",
    31224,
    "global const requires static",
    span { loc = "decl:Decl", message = "global const variable with initializer must be declared static: '~decl'" }
)

err(
    "static const variable requires initializer",
    31225,
    "missing initializer for static const",
    span { loc = "decl:Decl", message = "static const variable '~decl' must have an initializer" }
)

-- Enums (320xx)

err(
    "invalid enum tag type",
    32000,
    "invalid enum tag type",
    span { loc = "location", message = "invalid tag type for 'enum': '~type:Type'" }
)

err(
    "unexpected enum tag expr",
    32003,
    "unexpected enum tag expression",
    span { loc = "expr:Expr", message = "unexpected form for 'enum' tag value expression" }
)

-- 303xx: interfaces and associated types

err(
    "assoc type in interface only",
    30300,
    "associatedtype outside interface",
    span { loc = "decl:Decl", message = "'associatedtype' can only be defined in an 'interface'." }
)

err(
    "global gen param in global scope only",
    30301,
    "type_param outside global scope",
    span { loc = "decl:Decl", message = "'type_param' can only be defined global scope." }
)

err(
    "static const requirement must be int or bool",
    30302,
    "invalid static const requirement type",
    span { loc = "decl:Decl", message = "'static const' requirement can only have int or bool type." }
)

err(
    "value requirement must be compile time const",
    30303,
    "value requirement requires static const",
    span { loc = "decl:Decl", message = "requirement in the form of a simple value must be declared as 'static const'." }
)

err(
    "type is not differentiable",
    30310,
    "type is not differentiable",
    span { loc = "attr:Modifier", message = "type '~type:Type' is not differentiable." }
)

err(
    "non method interface requirement cannot have body",
    30311,
    "interface requirement has body",
    span { loc = "decl:Decl", message = "non-method interface requirement cannot have a body." }
)

err(
    "interface requirement cannot be override",
    30312,
    "interface requirement cannot override",
    span { loc = "decl:Decl", message = "interface requirement cannot override a base declaration." }
)

-- Interop (304xx)

err(
    "cannot define ptr type to managed resource",
    30400,
    "pointer to managed resource invalid",
    span { loc = "type_exp:Expr", message = "pointer to a managed resource is invalid, use `NativeRef<T>` instead" }
)

-- Control flow (305xx)

warning(
    "for loop side effect changing different var",
    30500,
    "for loop modifies wrong variable",
    span { loc = "side_effect:Expr", message = "the for loop initializes and checks variable '~init_var:Decl' but the side effect expression is modifying '~modified_var:Decl'." }
)

warning(
    "for loop predicate checking different var",
    30501,
    "for loop predicate checks wrong variable",
    span { loc = "predicate:Expr", message = "the for loop initializes and modifies variable '~init_var:Decl' but the predicate expression is checking '~predicate_var:Decl'." }
)

warning(
    "for loop changing iteration variable in oppsoite direction",
    30502,
    "for loop modifies variable in wrong direction",
    span { loc = "side_effect:Expr", message = "the for loop is modifiying variable '~var:Decl' in the opposite direction from loop exit condition." }
)

warning(
    "for loop not modifying iteration variable",
    30503,
    "for loop step is zero",
    span { loc = "side_effect:Expr", message = "the for loop is not modifiying variable '~var:Decl' because the step size evaluates to 0." }
)

warning(
    "for loop terminates in fewer iterations than max iters",
    30504,
    "MaxIters exceeds actual iterations",
    span { loc = "attr:Modifier", message = "the for loop is statically determined to terminate within ~iterations:Int iterations, which is less than what [MaxIters] specifies." }
)

warning(
    "loop runs for zero iterations",
    30505,
    "loop runs zero times",
    span { loc = "stmt:Stmt", message = "the loop runs for 0 iterations and will be removed." }
)

err(
    "loop in diff func require unroll or max iters",
    30510,
    "loop in differentiable function needs attributes",
    span { loc = "location", message = "loops inside a differentiable function need to provide either '[MaxIters(n)]' or '[ForceUnroll]' attribute." }
)

-- Switch (306xx)

err(
    "switch multiple default",
    30600,
    "multiple default cases in switch",
    span { loc = "stmt:Stmt", message = "multiple 'default' cases not allowed within a 'switch' statement" }
)

err(
    "switch duplicate cases",
    30601,
    "duplicate cases in switch",
    span { loc = "stmt:Stmt", message = "duplicate cases not allowed within a 'switch' statement" }
)

-- 310xx: link time specialization

warning(
    "link time constant array size",
    31000,
    "link-time constant sized arrays warning",
    span { loc = "decl:Decl", message = "Link-time constant sized arrays are a work in progress feature, some aspects of the reflection API may not work" }
)

-- Cyclic references and misc errors (39xxx)

err(
    "cyclic reference",
    39999,
    "cyclic reference",
    span { loc = "decl:Decl", message = "cyclic reference '~decl'." },
    nil,  -- severity will be overridden to fatal at call site
    "fatal"
)

err(
    "cyclic reference in inheritance",
    39998,
    "cyclic reference in inheritance",
    span { loc = "decl:Decl", message = "cyclic reference in inheritance graph '~decl'." }
)

err(
    "variable used in its own definition",
    39997,
    "variable used in own initializer",
    span { loc = "decl:Decl", message = "the initial-value expression for variable '~decl' depends on the value of the variable itself" }
)

err(
    "cannot process include",
    39901,
    "internal error: cannot process include",
    span { loc = "location", message = "internal compiler error: cannot process '__include' in the current semantic checking context." },
    nil,
    "fatal"
)

-- 304xx: generics

err(
    "generic type needs args",
    30410,
    "generic type used without arguments",
    span { loc = "type_exp:Expr", message = "generic type '~type:Type' used without argument" }
)

err(
    "invalid type for constraint",
    30401,
    "invalid constraint type",
    span { loc = "sup:Expr", message = "type '~type:Type' cannot be used as a constraint." }
)

err(
    "invalid constraint sub type",
    30402,
    "invalid constraint left-hand side",
    span { loc = "type_exp:Expr", message = "type '~type:Type' is not a valid left hand side of a type constraint." }
)

err(
    "required constraint is not checked",
    30403,
    "optional constraint not checked",
    span { loc = "location", message = "the constraint providing '~decl_ref:Decl' is optional and must be checked with an 'is' statement before usage." }
)

err(
    "invalid equality constraint sup type",
    30404,
    "invalid equality constraint type",
    span { loc = "sup:Expr", message = "type '~type:Type' is not a proper type to use in a generic equality constraint." }
)

err(
    "no valid equality constraint sub type",
    30405,
    "equality constraint requires dependent type",
    span { loc = "decl:Decl", message = "generic equality constraint requires at least one operand to be dependant on the generic declaration" }
)

-- Note: invalidEqualityConstraintSubType (note variant)
standalone_note(
    "invalid equality constraint sub type note",
    30406,
    "type '~type:Type' cannot be constrained by a type equality",
    span { loc = "type_exp:Expr" }
)

end
