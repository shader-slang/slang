-- We define the conceptual lexeme hierarchy once, then use it to generate kind tags and
-- the C++ types for atomic lexemes and tokens with trivia. A `LexemeClass` declaration
-- names its parent directly; only an abstract class can have subclasses.
-- The C++ declarations and functions are written in the headers that use these definitions.

-- The definition environment registers each assignment under its declared name.
-- For example, `Literal` below is a class object whose parent is the `Token` class object.
local function defineLexemeClasses(_ENV)
    Lexeme = LexemeClass{"lexeme", abstract = true}
    Trivia = LexemeClass{Lexeme, "trivia", abstract = true}
    Comment = LexemeClass{Trivia, "comment", abstract = true}
    Token = LexemeClass{Lexeme, "token", abstract = true}
    Literal = LexemeClass{Token, "literal", abstract = true}
    NumericLiteral = LexemeClass{Literal, "numeric literal", abstract = true}
    Punctuation = LexemeClass{Token, "punctuation", abstract = true}

    EndOfFile = LexemeClass{Token, "end of file"}
    Invalid = LexemeClass{Token, "invalid character"}
    Identifier = LexemeClass{Token, "identifier"}
    IntegerLiteral = LexemeClass{NumericLiteral, "integer literal"}
    FloatingPointLiteral = LexemeClass{NumericLiteral, "floating-point literal"}
    StringLiteral = LexemeClass{Literal, "string literal"}
    CharLiteral = LexemeClass{Literal, "character literal"}

    WhiteSpace = LexemeClass{Trivia, "whitespace"}
    NewLine = LexemeClass{Trivia, "end of line"}
    LineComment = LexemeClass{Comment, "line comment"}
    BlockComment = LexemeClass{Comment, "block comment"}

    Semicolon = LexemeClass{Punctuation, "';'"}
    Comma = LexemeClass{Punctuation, "','"}
    Dot = LexemeClass{Punctuation, "'.'"}
    DotDot = LexemeClass{Punctuation, "'..'"}
    Ellipsis = LexemeClass{Punctuation, "'...'"}
    LBrace = LexemeClass{Punctuation, "'{'"}
    RBrace = LexemeClass{Punctuation, "'}'"}
    LBracket = LexemeClass{Punctuation, "'['"}
    RBracket = LexemeClass{Punctuation, "']'"}
    LParent = LexemeClass{Punctuation, "'('"}
    RParent = LexemeClass{Punctuation, "')'"}
    OpAssign = LexemeClass{Punctuation, "'='"}
    OpAdd = LexemeClass{Punctuation, "'+'"}
    OpSub = LexemeClass{Punctuation, "'-'"}
    OpMul = LexemeClass{Punctuation, "'*'"}
    OpDiv = LexemeClass{Punctuation, "'/'"}
    OpMod = LexemeClass{Punctuation, "'%'"}
    OpNot = LexemeClass{Punctuation, "'!'"}
    OpBitNot = LexemeClass{Punctuation, "'~'"}
    OpLsh = LexemeClass{Punctuation, "'<<'"}
    OpRsh = LexemeClass{Punctuation, "'>>'"}
    OpEql = LexemeClass{Punctuation, "'=='"}
    OpNeq = LexemeClass{Punctuation, "'!='"}
    OpGreater = LexemeClass{Punctuation, "'>'"}
    OpLess = LexemeClass{Punctuation, "'<'"}
    OpGeq = LexemeClass{Punctuation, "'>='"}
    OpLeq = LexemeClass{Punctuation, "'<='"}
    OpAnd = LexemeClass{Punctuation, "'&&'"}
    OpOr = LexemeClass{Punctuation, "'||'"}
    OpBitAnd = LexemeClass{Punctuation, "'&'"}
    OpBitOr = LexemeClass{Punctuation, "'|'"}
    OpBitXor = LexemeClass{Punctuation, "'^'"}
    OpInc = LexemeClass{Punctuation, "'++'"}
    OpDec = LexemeClass{Punctuation, "'--'"}
    OpAddAssign = LexemeClass{Punctuation, "'+='"}
    OpSubAssign = LexemeClass{Punctuation, "'-='"}
    OpMulAssign = LexemeClass{Punctuation, "'*='"}
    OpDivAssign = LexemeClass{Punctuation, "'/='"}
    OpModAssign = LexemeClass{Punctuation, "'%='"}
    OpShlAssign = LexemeClass{Punctuation, "'<<='"}
    OpShrAssign = LexemeClass{Punctuation, "'>>='"}
    OpAndAssign = LexemeClass{Punctuation, "'&='"}
    OpOrAssign = LexemeClass{Punctuation, "'|='"}
    OpXorAssign = LexemeClass{Punctuation, "'^='"}
    QuestionMark = LexemeClass{Punctuation, "'?'"}
    Colon = LexemeClass{Punctuation, "':'"}
    RightArrow = LexemeClass{Punctuation, "'->'"}
    DoubleRightArrow = LexemeClass{Punctuation, "'=>'"}
    At = LexemeClass{Punctuation, "'@'"}
    Dollar = LexemeClass{Punctuation, "'$'"}
    DollarDollar = LexemeClass{Punctuation, "'$$'"}
    Pound = LexemeClass{Punctuation, "'#'"}
    PoundPound = LexemeClass{Punctuation, "'##'"}
    Scope = LexemeClass{Punctuation, "'::'"}
    CompletionRequest = LexemeClass{Punctuation, "'#?'"}

    EscapedNewLine = LexemeClass{Trivia, "escaped end of line"}
    AngleBracketHeaderName = LexemeClass{Token, "angle-bracket-enclosed header name"}
    EndOfDirective = LexemeClass{Token, "end of preprocessor directive"}
end

-- A class definition records its parent, diagnostic description, and abstractness.
local LexemeClass = {}
LexemeClass.__index = LexemeClass
local specificationKeys = {[1] = true, [2] = true, abstract = true}

-- Determine whether a class is `base` or a subclass of `base`.
-- Every parent is registered before its subclasses, so following parents terminates.
function LexemeClass:isSubclassOf(base)
    local current = self
    while current do
        if current == base then return true end
        current = current.base
    end
    return false
end

-- Construct a class from `{parent, description, abstract = true}`.
-- The root omits the parent; omitting `abstract` makes a class concrete.
local function createLexemeClass(specification)
    local base = specification[1]
    local description = specification[2]
    if type(base) == "string" then
        -- Only the root has a description in the first positional argument.
        assert(description == nil, "a root class has only a description")
        description = base
        base = nil
    else
        assert(getmetatable(base) == LexemeClass, "a parent must be a declared lexeme class")
    end

    -- We reject misspelled options instead of quietly treating a class as concrete.
    for key in pairs(specification) do
        assert(specificationKeys[key], "unknown lexeme class option: " .. tostring(key))
    end
    assert(type(description) == "string", "a lexeme class needs a diagnostic description")
    local isAbstract = specification.abstract
    if isAbstract == nil then isAbstract = false end
    assert(type(isAbstract) == "boolean", "abstract must be a boolean")

    return setmetatable({
        base = base,
        description = description,
        isAbstract = isAbstract,
    }, LexemeClass)
end

local classes = {}
local classesByName = {}

-- Register a named class and validate its place in the hierarchy before exposing it.
-- Requiring an already registered abstract parent prevents cycles and subclasses of
-- concrete classes.
local function registerLexemeClass(_, name, class)
    assert(getmetatable(class) == LexemeClass, "a definition must use LexemeClass")
    assert(not classesByName[name], "duplicate lexeme class: " .. name)
    assert(not class.name, "a lexeme class cannot be registered under more than one name")
    assert(name ~= "Unknown", "Unknown is reserved for empty values")
    assert(name ~= "Count", "Count is reserved for the kind count")
    if class.base then
        assert(class.base.name, "a parent must be registered before its subclasses")
        assert(class.base.isAbstract,
            "a concrete class cannot have subclasses: " .. class.base.name)
    else
        assert(name == "Lexeme", "Lexeme must be the only root class")
        assert(class.isAbstract, "Lexeme must be abstract")
    end

    class.name = name
    classesByName[name] = class
    table.insert(classes, class)
end

-- Resolve a parent name or the constructor while evaluating the declarations above.
-- An unknown name is a definition error, rather than a second spelling for the root.
local function findLexemeDefinition(_, name)
    if name == "LexemeClass" then return createLexemeClass end
    local class = classesByName[name]
    assert(class, "unknown lexeme class: " .. name)
    return class
end

-- We use a separate, empty environment so every assignment invokes registration,
-- including an erroneous redefinition. Definitions do not add globals to Fiddle's Lua state.
defineLexemeClasses(setmetatable({}, {
    __index = findLexemeDefinition,
    __newindex = registerLexemeClass,
}))

-- Derive C++ type names from the two branches. Atomic token types use `Raw`, whereas
-- only the token branch has a second set of types for tokens with attached trivia.
local root = classesByName.Lexeme
local trivia = classesByName.Trivia
local token = classesByName.Token
assert(trivia.base == root, "Trivia must derive directly from Lexeme")
assert(token.base == root, "Token must derive directly from Lexeme")
local rawClasses = {}
local tokenClasses = {}
for _, class in ipairs(classes) do
    if class == root then
        class.rawTypeName = "Lexeme"
    elseif class == trivia then
        class.rawTypeName = "Trivia"
    elseif class:isSubclassOf(trivia) then
        class.rawTypeName = class.name .. "Trivia"
    else
        assert(class:isSubclassOf(token),
            "a class must be either trivia or a token: " .. class.name)
        class.rawTypeName = "Raw" .. class.name
    end
    if class ~= root then table.insert(rawClasses, class) end

    if class == token then
        class.tokenTypeName = "Token"
    elseif class:isSubclassOf(token) then
        class.tokenTypeName = class.name .. "Token"
        table.insert(tokenClasses, class)
    end
end

return {classes = classes, rawClasses = rawClasses, tokenClasses = tokenClasses}
