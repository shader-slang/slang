-- Semantic checking diagnostics (part 12) - IL code generation
-- Converted from slang-diagnostic-defs.h lines 152-262

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 4xxxx - IL code generation.
--

err(
    "unimplemented system value semantic",
    40006,
    "unknown system-value semantic",
    span { loc = "location", message = "unknown system-value semantic '~semantic_name'" }
)

err(
    "unknown system value semantic",
    49999,
    "unknown system-value semantic",
    span { loc = "location", message = "unknown system-value semantic '~semantic_name'" }
)

err(
    "ir validation failed",
    40007,
    "IR validation failed",
    span { loc = "location", message = "IR validation failed: ~message" }
)

err(
    "invalid l value for ref parameter",
    40008,
    "the form of this l-value argument is not valid for a `ref` parameter",
    span { loc = "location", message = "the form of this l-value argument is not valid for a `ref` parameter" }
)

err(
    "need compile time constant",
    40012,
    "expected a compile-time constant",
    span { loc = "location", message = "expected a compile-time constant" }
)

err(
    "arg is not constexpr",
    40013,
    "argument is not a compile-time constant",
    span { loc = "location", message = "arg ~arg_index:Int in '~func_name' is not a compile-time constant" }
)

err(
    "cannot unroll loop",
    40020,
    "loop unrolling failed",
    span { loc = "location", message = "loop does not terminate within the limited number of iterations, unrolling is aborted." }
)

err(
    "function never returns fatal",
    40030,
    "function never returns",
    span { loc = "location", message = "function '~func_name' never returns, compilation ceased." }
)

-- 41000 - IR-level validation issues

warning(
    "unreachable code",
    41000,
    "unreachable code detected",
    span { loc = "stmt:Stmt", message = "unreachable code detected" }
)

err(
    "recursive type",
    41001,
    "type contains cyclic reference",
    span { loc = "location", message = "type '~type_name' contains cyclic reference to itself." }
)

err(
    "cyclic interface dependency",
    41002,
    "interface has cyclic dependency on itself",
    span { loc = "interface_type:IRInst", message = "interface '~interface_type' has cyclic dependency on itself through its implementations." }
)

err(
    "missing return error",
    41009,
    "non-void function must return",
    span { loc = "location", message = "non-void function must return in all cases for target '~target_name'" }
)

warning(
    "missing return",
    41010,
    "non-void function does not return in all cases",
    span { loc = "location", message = "non-void function does not return in all cases" }
)

err(
    "profile incompatible with target switch",
    41011,
    "__target_switch has no compatible target",
    span { loc = "location", message = "__target_switch has no compatable target with current profile '~profile'" }
)

warning(
    "profile implicitly upgraded",
    41012,
    "profile implicitly upgraded",
    span { loc = "location", message = "entry point '~entry_point' uses additional capabilities that are not part of the specified profile '~profile'. The profile setting is automatically updated to include these capabilities: '~capabilities'" }
)

err(
    "profile implicitly upgraded restrictive",
    41012,
    "entry point uses capabilities not in specified profile",
    span { loc = "location", message = "entry point '~entry_point' uses capabilities that are not part of the specified profile '~profile'. Missing capabilities are: '~capabilities'" }
)

warning(
    "using uninitialized out",
    41015,
    "use of uninitialized out parameter",
    span { loc = "location", message = "use of uninitialized out parameter '~param_name'" }
)

warning(
    "using uninitialized variable",
    41016,
    "use of uninitialized variable",
    span { loc = "location", message = "use of uninitialized variable '~var_name'" }
)

warning(
    "using uninitialized value",
    41016,
    "use of uninitialized value",
    span { loc = "location", message = "use of uninitialized value of type '~type_name'" }
)

warning(
    "using uninitialized global variable",
    41017,
    "use of uninitialized global variable",
    span { loc = "location", message = "use of uninitialized global variable '~var_name'" }
)

warning(
    "returning with uninitialized out",
    41018,
    "returning without initializing out parameter",
    span { loc = "location", message = "returning without initializing out parameter '~param_name'" }
)

warning(
    "constructor uninitialized field",
    41020,
    "exiting constructor without initializing field",
    span { loc = "location", message = "exiting constructor without initializing field '~field_name'" }
)

warning(
    "field not default initialized",
    41021,
    "default initializer will not initialize field",
    span { loc = "location", message = "default initializer for '~type_name' will not initialize field '~field_name'" }
)

warning(
    "comma operator used in expression",
    41024,
    "comma operator used in expression",
    span { loc = "expr:Expr", message = "comma operator used in expression (may be unintended)" }
)

warning(
    "switch fallthrough restructured",
    41026,
    "switch fall-through will be restructured",
    span { loc = "location", message = "switch fall-through is not supported by this target and will be restructured; this may affect wave/subgroup convergence if the duplicated code contains wave operations" }
)

err(
    "cannot default initialize resource",
    41024,
    "cannot default-initialize resource type",
    span { loc = "location", message = "cannot default-initialize ~resource_name with '{}'. Resource types must be explicitly initialized" }
)

err(
    "cannot default initialize struct with uninitialized resource",
    41024,
    "cannot default-initialize struct with uninitialized resource",
    span { loc = "location", message = "cannot default-initialize struct '~struct_name' with '{}' because it contains an uninitialized ~resource_name field" }
)

err(
    "cannot default initialize struct containing resources",
    41024,
    "cannot default-initialize struct containing resource fields",
    span { loc = "location", message = "cannot default-initialize struct '~struct_name' with '{}' because it contains resource fields" }
)

end
