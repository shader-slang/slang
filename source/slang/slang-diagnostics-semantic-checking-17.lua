-- Semantic checking diagnostics (part 17) - Standalone notes for cross-referencing
-- These are notes that can be attached to various error diagnostics to point
-- to related declarations/definitions

return function(helpers)

local span = helpers.span
local note = helpers.note
local standalone_note = helpers.standalone_note
local variadic_span = helpers.variadic_span
local variadic_note = helpers.variadic_note
local err = helpers.err
local warning = helpers.warning
local internal = helpers.internal
local fatal = helpers.fatal

--
-- -1 - Notes that decorate another diagnostic.
--
-- These are standalone notes used after various error diagnostics to point
-- to related locations (definitions, declarations, etc.)
--

-- Note: These notes use Decl* which provides both the name (via getName()) and
-- location (via loc). The diagnostic system will extract the name for display.

standalone_note(
    "see definition of",
    -1,
    "see definition of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see definition of struct",
    -1,
    "see definition of struct '~name'",
    span { loc = "location" }
)

standalone_note(
    "see constant buffer definition",
    -1,
    "see constant buffer definition.",
    span { loc = "location" }
)

-- Note: seeUsingOf takes both a decl (for name) and a location (for where it's used)
-- This allows pointing to the call site while showing the name of what's being used
standalone_note(
    "see using of",
    -1,
    "see using of '~decl:Decl'",
    span { loc = "location" }
)

standalone_note(
    "see call of func",
    -1,
    "see call to '~name'",
    span { loc = "location" }
)

standalone_note(
    "see previous definition",
    -1,
    "see previous definition",
    span { loc = "location" }
)

standalone_note(
    "see previous definition of",
    -1,
    "see previous definition of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see declaration of",
    -1,
    "see declaration of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see declaration of interface requirement",
    -1,
    "see interface requirement declaration of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see overload considered",
    -1,
    "see overloads considered: '~decl:Decl'.",
    span { loc = "decl:Decl" }
)

standalone_note(
    "see previous declaration of",
    -1,
    "see previous declaration of '~decl:Decl'",
    span { loc = "decl:Decl" }
)

standalone_note(
    "note explicit conversion possible",
    -1,
    "explicit conversion from '~from_type:Type' to '~to_type:Type' is possible",
    span { loc = "location" }
)

-- IR-specific variants for use in IR linking and lowering passes
-- These take IRInst* instead of Decl*

standalone_note(
    "see declaration of ir",
    -1,
    "see declaration of '~inst:IRInst'",
    span { loc = "inst:IRInst" }
)

standalone_note(
    "see call of func ir",
    -1,
    "see call to '~inst:IRInst'",
    span { loc = "location" }
)

-- Modifier variant for notes
standalone_note(
    "see declaration of modifier",
    -1,
    "see declaration of '~modifier:Modifier'",
    span { loc = "modifier:Modifier" }
)

-- ASTNodeType variant for generic references
standalone_note(
    "see using of node type",
    -1,
    "see using of '~node_type'",
    span { loc = "location" }
)

end
