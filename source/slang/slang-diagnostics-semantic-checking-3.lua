-- Semantic checking diagnostics (part 3) - Include, Visibility, and Capability diagnostics
-- Converted from slang-diagnostic-defs.h lines 303-447

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 305xx - Include
--

err(
    "included file missing implementing",
    30500,
    "missing 'implementing' declaration",
    span { loc = "location", message = "missing 'implementing' declaration in the included source file '~file_name'." }
)

err(
    "included file missing implementing do you mean import",
    30501,
    "missing 'implementing' declaration",
    span { loc = "location", message = "missing 'implementing' declaration in the included source file '~file_name'. The file declares that it defines module '~module_name', do you mean 'import' instead?" }
)

err(
    "included file does not implement current module",
    30502,
    "module name mismatch in included file",
    span { loc = "location", message = "the included source file is expected to implement module '~expected_module:Name', but it is implementing '~actual_module' instead." }
)

err(
    "primary module file cannot start with implementing decl",
    30503,
    "primary module file cannot start with 'implementing'",
    span { loc = "decl:Decl", message = "a primary source file for a module cannot start with 'implementing'." }
)

warning(
    "primary module file must start with module decl",
    30504,
    "primary module file should start with 'module'",
    span { loc = "decl:Decl", message = "a primary source file for a module should start with 'module'." }
)

err(
    "implementing must reference primary module file",
    30505,
    "'implementing' must reference primary module file",
    span { loc = "location", message = "the source file referenced by 'implementing' must be a primary module file starting with a 'module' declaration." }
)

warning(
    "module implementation has file extension",
    30506,
    "file extension in module name",
    span { loc = "location", message = "implementing directive contains file extension in module name '~module_name'. Module names should not include extensions. The compiler will use '~normalized_name' as the module name." }
)

--
-- 306xx - Visibility
--

err(
    "decl is not visible",
    30600,
    "declaration not accessible",
    span { loc = "location", message = "'~decl:Decl' is not accessible from the current context." }
)

err(
    "decl cannot have higher visibility",
    30601,
    "visibility higher than parent",
    span { loc = "decl:Decl", message = "'~decl' cannot have a higher visibility than '~parent:Decl'." }
)

err(
    "invalid use of private visibility",
    30603,
    "invalid private visibility",
    span { loc = "location", message = "'~decl:Decl' cannot have private visibility because it is not a member of a type." }
)

err(
    "use of less visible type",
    30604,
    "references less visible type",
    span { loc = "decl:Decl", message = "'~decl' references less visible type '~type:Type'." }
)

err(
    "invalid visibility modifier on type of decl",
    36005,
    "visibility modifier not allowed",
    span { loc = "location", message = "visibility modifier is not allowed on '~ast_node_type'." }
)

--
-- 361xx - Capability
--

err(
    "conflicting capability due to use of decl",
    36100,
    "conflicting capability requirement",
    span { loc = "location", message = "'~referenced_decl:Decl' requires capability '~required_caps' that is conflicting with the '~context_decl:Decl's current capability requirement '~existing_caps'." }
)

err(
    "conflicting capability due to statement",
    36101,
    "conflicting capability requirement",
    span { loc = "location", message = "statement requires capability '~required_caps' that is conflicting with the '~context's current capability requirement '~existing_caps'." }
)

err(
    "conflicting capability due to statement enclosing func",
    36102,
    "conflicting capability requirement",
    span { loc = "location", message = "statement requires capability '~required_caps' that is conflicting with the current function's capability requirement '~existing_caps'." }
)

err(
    "use of undeclared capability",
    36104,
    "undeclared capability used",
    span { loc = "decl:Decl", message = "'~decl' uses undeclared capability '~caps'" }
)

err(
    "use of undeclared capability of interface requirement",
    36104,
    "capability incompatible with interface",
    span { loc = "decl:Decl", message = "'~decl' uses capability '~caps' that is incompatable with the interface requirement" }
)

err(
    "use of undeclared capability of inheritance decl",
    36104,
    "capability incompatible with supertype",
    span { loc = "decl:Decl", message = "'~decl' uses capability '~caps' that is incompatable with the supertype" }
)

err(
    "unknown capability",
    36105,
    "unknown capability",
    span { loc = "location", message = "unknown capability name '~capability'." }
)

err(
    "expect capability",
    36106,
    "expect a capability name",
    span { loc = "location", message = "expect a capability name." }
)

err(
    "entry point uses unavailable capability",
    36107,
    "unavailable features in entry point",
    span { loc = "decl:Decl", message = "entrypoint '~decl' uses features that are not available in '~stage' stage for '~target' compilation target." }
)

err(
    "decl has dependencies not compatible on target",
    36108,
    "dependencies not compatible on target",
    span { loc = "decl:Decl", message = "'~decl' has dependencies that are not compatible on the required compilation target '~target'." }
)

err(
    "invalid target switch case",
    36109,
    "invalid target_switch case",
    span { loc = "location", message = "'~capability' cannot be used as a target_switch case." }
)

err(
    "unexpected capability",
    36111,
    "disallowed capability",
    span { loc = "location", message = "'~decl' resolves into a disallowed `~capability` Capability." }
)

warning(
    "entry point and profile are incompatible",
    36112,
    "entry point incompatible with profile",
    span { loc = "location", message = "'~decl:Decl' is defined for stage '~stage', which is incompatible with the declared profile '~profile'." }
)

warning(
    "using internal capability name",
    36113,
    "using internal capability name",
    span { loc = "location", message = "'~decl' resolves into a '_Internal' '_~capability' Capability, use '~capability' instead." }
)

err(
    "capability has multiple stages",
    36116,
    "capability targets multiple stages",
    span { loc = "location", message = "Capability '~capability' is targeting stages '~stages', only allowed to use 1 unique stage here." }
)

err(
    "decl has dependencies not compatible on stage",
    36117,
    "dependencies not compatible on stage",
    span { loc = "decl:Decl", message = "'~decl' requires support for stage '~stage', but stage is unsupported." }
)

err(
    "sub type has subset of abstract atoms to super type",
    36118,
    "subtype missing target/stage support",
    span { loc = "decl:Decl", message = "subtype '~decl' must have the same target/stage support as the supertype; '~decl' is missing '~missing_caps'" }
)

err(
    "requirment has subset of abstract atoms to implementation",
    36119,
    "requirement missing target/stage support",
    span { loc = "decl:Decl", message = "requirement '~decl' must have the same target/stage support as the implementation; '~decl' is missing '~missing_caps'" }
)

err(
    "target switch cap cases conflict",
    36120,
    "capability cases conflict in target_switch",
    span { loc = "location", message = "the capability for case '~case_name' is '~case_caps', which is conflicts with previous case which requires '~prev_caps'. In target_switch, if two cases are belong to the same target, then one capability set has to be a subset of the other." }
)

end
