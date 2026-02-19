-- Semantic checking diagnostics (part 7) - Link Time, Cyclic Refs, Generics, Initializers, Variables, Parameters, Inheritance, Extensions, Subscripts
-- Converted from slang-diagnostic-defs.h lines 340-590

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning

--
-- 310xx: link time specialization
--
warning(
    "link time constant array size",
    31000,
    "Link-time constant sized arrays are a work in progress feature, some aspects of the reflection API may not work",
    span { loc = "decl:Decl" }
)

--
-- 39xxx: cyclic references
--
-- TODO: need to assign numbers to all these extra diagnostics...
err(
    "cyclic reference",
    39999,
    "cyclic reference '~decl'",
    span { loc = "decl:Decl", message = "cyclic reference '~decl'" }
)

err(
    "cyclic reference in inheritance",
    39999,
    "cyclic reference in inheritance graph '~decl'",
    span { loc = "decl:Decl", message = "cyclic reference in inheritance graph '~decl'" }
)

err(
    "variable used in its own definition",
    39999,
    "the initial-value expression for variable '~decl' depends on the value of the variable itself",
    span { loc = "decl:Decl", message = "the initial-value expression for variable '~decl' depends on the value of the variable itself" }
)

err(
    "cannot process include",
    39901,
    "internal compiler error: cannot process '__include' in the current semantic checking context",
    span { loc = "location", message = "internal compiler error: cannot process '__include' in the current semantic checking context" }
)

--
-- 304xx: generics
--
err(
    "generic type needs args",
    30400,
    "generic type '~type:Type' used without argument",
    span { loc = "type_exp:Expr", message = "generic type '~type' used without argument" }
)

err(
    "invalid type for constraint",
    30401,
    "type '~type:Type' cannot be used as a constraint",
    span { loc = "type_exp:Expr", message = "type '~type' cannot be used as a constraint" }
)

err(
    "invalid constraint sub type",
    30402,
    "type '~type:Type' is not a valid left hand side of a type constraint",
    span { loc = "type_exp:Expr", message = "type '~type' is not a valid left hand side of a type constraint" }
)

err(
    "required constraint is not checked",
    30403,
    "the constraint providing '~decl:Decl' is optional and must be checked with an 'is' statement before usage",
    span { loc = "location", message = "the constraint providing '~decl' is optional and must be checked with an 'is' statement before usage" }
)

err(
    "invalid equality constraint sup type",
    30404,
    "type '~type:Type' is not a proper type to use in a generic equality constraint",
    span { loc = "type_exp:Expr", message = "type '~type' is not a proper type to use in a generic equality constraint" }
)

err(
    "no valid equality constraint sub type",
    30405,
    "generic equality constraint requires at least one operand to be dependant on the generic declaration",
    span { loc = "decl:Decl", message = "generic equality constraint requires at least one operand to be dependant on the generic declaration" }
)

standalone_note(
    "invalid equality constraint sub type",
    30402,
    "type '~type:Type' cannot be constrained by a type equality",
    span { loc = "type_exp:Expr", message = "type '~type' cannot be constrained by a type equality" }
)

warning(
    "failed equality constraint canonical order",
    30407,
    "failed to resolve canonical order of generic equality constraint",
    span { loc = "decl:Decl", message = "failed to resolve canonical order of generic equality constraint" }
)

--
-- 305xx: initializer lists
--
-- Note: tooManyInitializers (30500) already exists in the main lua file

err(
    "cannot use initializer list for vector of unknown size",
    30502,
    "cannot use initializer list for vector of statically unknown size '~element_count:Val'",
    span { loc = "init_list:Expr", message = "cannot use initializer list for vector of statically unknown size '~element_count'" }
)

err(
    "cannot use initializer list for matrix of unknown size",
    30503,
    "cannot use initializer list for matrix of statically unknown size '~row_count:Val' rows",
    span { loc = "init_list:Expr", message = "cannot use initializer list for matrix of statically unknown size '~row_count' rows" }
)

err(
    "cannot use initializer list for type",
    30504,
    "cannot use initializer list for type '~type:Type'",
    span { loc = "init_list:Expr", message = "cannot use initializer list for type '~type'" }
)

err(
    "cannot use initializer list for coop vector of unknown size",
    30505,
    "cannot use initializer list for CoopVector of statically unknown size '~element_count:Val'",
    span { loc = "init_list:Expr", message = "cannot use initializer list for CoopVector of statically unknown size '~element_count'" }
)

warning(
    "interface default initializer",
    30506,
    "initializing an interface variable with defaults is deprecated and may cause unexpected behavior",
    span { loc = "expr:Expr", message = "initializing an interface variable with defaults is deprecated and may cause unexpected behavior. Please provide a compatible initializer or leave the variable uninitialized" }
)

--
-- 3062x: variables
--
err(
    "var without type must have initializer",
    30620,
    "a variable declaration without an initial-value expression must be given an explicit type",
    span { loc = "decl:Decl", message = "a variable declaration without an initial-value expression must be given an explicit type" }
)

err(
    "param without type must have initializer",
    30621,
    "a parameter declaration without an initial-value expression must be given an explicit type",
    span { loc = "decl:Decl", message = "a parameter declaration without an initial-value expression must be given an explicit type" }
)

err(
    "ambiguous default initializer for type",
    30622,
    "more than one default initializer was found for type '~type:Type'",
    span { loc = "decl:Decl", message = "more than one default initializer was found for type '~type'" }
)

err(
    "cannot have initializer",
    30623,
    "'~decl:Decl' cannot have an initializer because it is ~reason",
    span { loc = "decl:Decl", message = "'~decl' cannot have an initializer because it is ~reason" }
)

err(
    "generic value parameter must have type",
    30623,
    "a generic value parameter must be given an explicit type",
    span { loc = "decl:Decl", message = "a generic value parameter must be given an explicit type" }
)

err(
    "generic value parameter type not supported",
    30624,
    "generic value parameter type '~type:Type' is not supported; only integer and enum types are allowed",
    span { loc = "decl:Decl", message = "generic value parameter type '~type' is not supported; only integer and enum types are allowed" }
)

--
-- 307xx: parameters
--
err(
    "output parameter cannot have default value",
    30700,
    "an 'out' or 'inout' parameter cannot have a default-value expression",
    span { loc = "init_expr:Expr", message = "an 'out' or 'inout' parameter cannot have a default-value expression" }
)

err(
    "system value semantic invalid type",
    30701,
    "type '~type:Type' is not valid for system value semantic '~semantic'; expected '~expected_types'",
    span { loc = "location" }
)

err(
    "system value semantic invalid direction",
    30702,
    "system value semantic '~semantic' cannot be used as ~direction in '~stage' shader stage",
    span { loc = "location" }
)

--
-- 308xx: inheritance
--
err(
    "base of interface must be interface",
    30810,
    "interface '~decl:Decl' cannot inherit from non-interface type '~base_type:Type'",
    span { loc = "inheritance_decl:Decl", message = "interface '~decl' cannot inherit from non-interface type '~base_type'" }
)

err(
    "base of struct must be interface",
    30811,
    "struct '~decl:Decl' cannot inherit from non-interface type '~base_type:Type'",
    span { loc = "inheritance_decl:Decl", message = "struct '~decl' cannot inherit from non-interface type '~base_type'" }
)

err(
    "base of enum must be integer or interface",
    30812,
    "enum '~decl:Decl' cannot inherit from type '~base_type:Type' that is neither an interface not a builtin integer type",
    span { loc = "inheritance_decl:Decl", message = "enum '~decl' cannot inherit from type '~base_type' that is neither an interface not a builtin integer type" }
)

err(
    "base of extension must be interface",
    30813,
    "extension cannot inherit from non-interface type '~base_type:Type'",
    span { loc = "inheritance_decl:Decl", message = "extension cannot inherit from non-interface type '~base_type'" }
)

err(
    "base of class must be class or interface",
    30814,
    "class '~decl:Decl' cannot inherit from type '~base_type:Type' that is neither a class nor an interface",
    span { loc = "inheritance_decl:Decl", message = "class '~decl' cannot inherit from type '~base_type' that is neither a class nor an interface" }
)

err(
    "circularity in extension",
    30815,
    "circular extension is not allowed",
    span { loc = "decl:Decl", message = "circular extension is not allowed" }
)

warning(
    "inheritance unstable",
    30816,
    "support for inheritance is unstable and will be removed in future language versions, consider using composition instead",
    span { loc = "inheritance_decl:Decl", message = "support for inheritance is unstable and will be removed in future language versions, consider using composition instead" }
)

err(
    "base struct must be listed first",
    30820,
    "a struct type may only inherit from one other struct type, and that type must appear first in the list of bases",
    span { loc = "inheritance_decl:Decl", message = "a struct type may only inherit from one other struct type, and that type must appear first in the list of bases" }
)

err(
    "tag type must be listed first",
    30821,
    "an enum type may only have a single tag type, and that type must be listed first in the list of bases",
    span { loc = "inheritance_decl:Decl", message = "an enum type may only have a single tag type, and that type must be listed first in the list of bases" }
)

err(
    "base class must be listed first",
    30822,
    "a class type may only inherit from one other class type, and that type must appear first in the list of bases",
    span { loc = "inheritance_decl:Decl", message = "a class type may only inherit from one other class type, and that type must appear first in the list of bases" }
)

err(
    "cannot inherit from explicitly sealed declaration in another module",
    30830,
    "cannot inherit from type '~base_type:Type' marked 'sealed' in module '~module_name:Name'",
    span { loc = "inheritance_decl:Decl", message = "cannot inherit from type '~base_type' marked 'sealed' in module '~module_name'" }
)

err(
    "cannot inherit from implicitly sealed declaration in another module",
    30831,
    "cannot inherit from type '~base_type:Type' in module '~module_name:Name' because it is implicitly 'sealed'",
    span { loc = "inheritance_decl:Decl", message = "cannot inherit from type '~base_type' in module '~module_name' because it is implicitly 'sealed'; mark the base type 'open' to allow inheritance across modules" }
)

err(
    "invalid type for inheritance",
    30832,
    "type '~type:Type' cannot be used for inheritance",
    span { loc = "inheritance_decl:Decl", message = "type '~type' cannot be used for inheritance" }
)

--
-- 308xx: extensions
--
err(
    "invalid extension on type",
    30850,
    "type '~type:Type' cannot be extended",
    span { loc = "type_exp:Expr", message = "type '~type' cannot be extended. `extension` can only be used to extend a nominal type" }
)

err(
    "invalid member type in extension",
    30851,
    "~node_type cannot be a part of an `extension`",
    span { loc = "decl:Decl", message = "~node_type cannot be a part of an `extension`" }
)

err(
    "invalid extension on interface",
    30852,
    "cannot extend interface type '~type:Type'",
    span { loc = "type_exp:Expr", message = "cannot extend interface type '~type'. consider using a generic extension: `extension<T:~type> T {...}`" }
)

err(
    "missing override",
    30853,
    "missing 'override' keyword for methods that overrides the default implementation in the interface",
    span { loc = "decl:Decl", message = "missing 'override' keyword for methods that overrides the default implementation in the interface" }
)

err(
    "override modifier not overriding base decl",
    30854,
    "'~decl:Decl' marked as 'override' is not overriding any base declarations",
    span { loc = "decl:Decl", message = "'~decl' marked as 'override' is not overriding any base declarations" }
)

err(
    "unreferenced generic param in extension",
    30855,
    "generic parameter '~param_name:Name' is not referenced by extension target type '~target_type:Type'",
    span { loc = "decl:Decl", message = "generic parameter '~param_name' is not referenced by extension target type '~target_type'" }
)

warning(
    "generic param in extension not referenced by target type",
    30856,
    "the extension is non-standard and may not work as intended because the generic parameter '~param_name:Name' is not referenced by extension target type '~target_type:Type'",
    span { loc = "decl:Decl", message = "the extension is non-standard and may not work as intended because the generic parameter '~param_name' is not referenced by extension target type '~target_type'" }
)

--
-- 309xx: subscripts
--
err(
    "multi dimensional array not supported",
    30900,
    "multi-dimensional array is not supported",
    span { loc = "expr:Expr", message = "multi-dimensional array is not supported" }
)

err(
    "subscript must have return type",
    30901,
    "__subscript declaration must have a return type specified after '->'",
    span { loc = "location", message = "__subscript declaration must have a return type specified after '->'" }
)

end
