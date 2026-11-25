-- Lexeme declarations describe a tree; their textual order does not assign tags.
-- We resolve the declarations first, then visit the tree depth first so every class
-- occupies one contiguous range of concrete leaf tags.

-- Declares the lexeme hierarchy in the supplied schema environment.
local function defineLexemeClasses(_ENV)
    Lexeme = LexemeClass{"lexeme", abstract = true}
    Trivia = LexemeClass{Lexeme, "trivia", abstract = true}
    Comment = LexemeClass{Trivia, "comment", abstract = true}
    Token = LexemeClass{Lexeme, "token", abstract = true}
    Literal = LexemeClass{Token, "literal", abstract = true}
    NumericLiteral = LexemeClass{Literal, "numeric literal", abstract = true}
    Punctuation = LexemeClass{Token, "punctuation", abstract = true}

    EndOfFile = LexemeClass{Token, "end of file", fixedSpelling = ""}
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

    Semicolon = LexemeClass{Punctuation, fixedSpelling = ";"}
    Comma = LexemeClass{Punctuation, fixedSpelling = ","}
    Dot = LexemeClass{Punctuation, fixedSpelling = "."}
    DotDot = LexemeClass{Punctuation, fixedSpelling = ".."}
    Ellipsis = LexemeClass{Punctuation, fixedSpelling = "..."}
    LBrace = LexemeClass{Punctuation, fixedSpelling = "{"}
    RBrace = LexemeClass{Punctuation, fixedSpelling = "}"}
    LBracket = LexemeClass{Punctuation, fixedSpelling = "["}
    RBracket = LexemeClass{Punctuation, fixedSpelling = "]"}
    LParent = LexemeClass{Punctuation, fixedSpelling = "("}
    RParent = LexemeClass{Punctuation, fixedSpelling = ")"}
    OpAssign = LexemeClass{Punctuation, fixedSpelling = "="}
    OpAdd = LexemeClass{Punctuation, fixedSpelling = "+"}
    OpSub = LexemeClass{Punctuation, fixedSpelling = "-"}
    OpMul = LexemeClass{Punctuation, fixedSpelling = "*"}
    OpDiv = LexemeClass{Punctuation, fixedSpelling = "/"}
    OpMod = LexemeClass{Punctuation, fixedSpelling = "%"}
    OpNot = LexemeClass{Punctuation, fixedSpelling = "!"}
    OpBitNot = LexemeClass{Punctuation, fixedSpelling = "~"}
    OpLsh = LexemeClass{Punctuation, fixedSpelling = "<<"}
    OpRsh = LexemeClass{Punctuation, fixedSpelling = ">>"}
    OpEql = LexemeClass{Punctuation, fixedSpelling = "=="}
    OpNeq = LexemeClass{Punctuation, fixedSpelling = "!="}
    OpGreater = LexemeClass{Punctuation, fixedSpelling = ">"}
    OpLess = LexemeClass{Punctuation, fixedSpelling = "<"}
    OpGeq = LexemeClass{Punctuation, fixedSpelling = ">="}
    OpLeq = LexemeClass{Punctuation, fixedSpelling = "<="}
    OpAnd = LexemeClass{Punctuation, fixedSpelling = "&&"}
    OpOr = LexemeClass{Punctuation, fixedSpelling = "||"}
    OpBitAnd = LexemeClass{Punctuation, fixedSpelling = "&"}
    OpBitOr = LexemeClass{Punctuation, fixedSpelling = "|"}
    OpBitXor = LexemeClass{Punctuation, fixedSpelling = "^"}
    OpInc = LexemeClass{Punctuation, fixedSpelling = "++"}
    OpDec = LexemeClass{Punctuation, fixedSpelling = "--"}
    OpAddAssign = LexemeClass{Punctuation, fixedSpelling = "+="}
    OpSubAssign = LexemeClass{Punctuation, fixedSpelling = "-="}
    OpMulAssign = LexemeClass{Punctuation, fixedSpelling = "*="}
    OpDivAssign = LexemeClass{Punctuation, fixedSpelling = "/="}
    OpModAssign = LexemeClass{Punctuation, fixedSpelling = "%="}
    OpShlAssign = LexemeClass{Punctuation, fixedSpelling = "<<="}
    OpShrAssign = LexemeClass{Punctuation, fixedSpelling = ">>="}
    OpAndAssign = LexemeClass{Punctuation, fixedSpelling = "&="}
    OpOrAssign = LexemeClass{Punctuation, fixedSpelling = "|="}
    OpXorAssign = LexemeClass{Punctuation, fixedSpelling = "^="}
    QuestionMark = LexemeClass{Punctuation, fixedSpelling = "?"}
    Colon = LexemeClass{Punctuation, fixedSpelling = ":"}
    RightArrow = LexemeClass{Punctuation, fixedSpelling = "->"}
    DoubleRightArrow = LexemeClass{Punctuation, fixedSpelling = "=>"}
    At = LexemeClass{Punctuation, fixedSpelling = "@"}
    Dollar = LexemeClass{Punctuation, fixedSpelling = "$"}
    DollarDollar = LexemeClass{Punctuation, fixedSpelling = "$$"}
    Pound = LexemeClass{Punctuation, fixedSpelling = "#"}
    PoundPound = LexemeClass{Punctuation, fixedSpelling = "##"}
    Scope = LexemeClass{Punctuation, fixedSpelling = "::"}
    CompletionRequest = LexemeClass{Punctuation, fixedSpelling = "#?"}

    EscapedNewLine = LexemeClass{Trivia, "escaped end of line"}
    AngleBracketHeaderName = LexemeClass{Token, "angle-bracket-enclosed header name"}
    EndOfDirective = LexemeClass{Token, "end of preprocessor directive", fixedSpelling = ""}
end


-- A reference can precede its definition. The environment returns the same class
-- object for both, so children keep a valid parent reference when it is defined later.
local LexemeClass = {}
LexemeClass.__index = LexemeClass
local classesByName = {}
local specificationKeys = {[1] = true, [2] = true, abstract = true, fixedSpelling = true}

-- Returns whether this class is `base` or derives from it.
function LexemeClass:isSubclassOf(base)
    local current = self
    while current do
        if current == base then return true end
        current = current.base
    end
    return false
end

-- Creates an unnamed lexeme class from its schema specification.
--
-- The schema environment supplies the name when it registers the definition.
local function createLexemeClass(specification)
    local base = specification[1]
    local description = specification[2]
    if type(base) == "string" then
        assert(description == nil, "a root class has only a description")
        description = base
        base = nil
    else
        assert(getmetatable(base) == LexemeClass, "a parent must be a lexeme class")
    end
    for key in pairs(specification) do
        assert(specificationKeys[key], "unknown lexeme class option: " .. tostring(key))
    end
    -- A fixed spelling can supply the punctuation's diagnostic description as well.
    -- An explicit empty spelling describes a zero-width token; nil means variable spelling.
    local fixedSpelling = specification.fixedSpelling
    if fixedSpelling ~= nil then
        assert(type(fixedSpelling) == "string", "fixedSpelling must be a string")
        if description == nil then description = "'" .. fixedSpelling .. "'" end
    end
    assert(type(description) == "string", "a lexeme class needs a diagnostic description")
    local isAbstract = specification.abstract
    if isAbstract == nil then isAbstract = false end
    assert(type(isAbstract) == "boolean", "abstract must be a boolean")
    if fixedSpelling ~= nil then
        assert(not isAbstract, "an abstract class cannot have a fixed spelling")
    end
    return setmetatable({
        base = base,
        description = description,
        isAbstract = isAbstract,
        fixedSpelling = fixedSpelling,
    }, LexemeClass)
end

-- Returns the cached class for `name`, creating an undefined placeholder if needed.
--
-- `registerLexemeClass` fills that same object, preserving earlier parent references.
local function getClass(name)
    local class = classesByName[name]
    if not class then
        class = setmetatable({name = name, isDefined = false}, LexemeClass)
        classesByName[name] = class
    end
    return class
end

-- Registers `definition` under `name` by filling its cached forward reference.
--
-- Reusing a named class as a definition would give it two identities, so aliases
-- and duplicate names are errors.
local function registerLexemeClass(_, name, definition)
    assert(getmetatable(definition) == LexemeClass, "a definition must use LexemeClass")
    assert(not definition.name, "a class cannot be defined under another class's name")
    assert(name ~= "Count", "Count is reserved for the tag count")
    assert(name ~= "LexemeClass", "LexemeClass is reserved for the constructor")
    local class = getClass(name)
    assert(not class.isDefined, "duplicate lexeme class: " .. name)
    class.base = definition.base
    class.description = definition.description
    class.isAbstract = definition.isAbstract
    class.fixedSpelling = definition.fixedSpelling
    class.isDefined = true
end

-- Returns the constructor for `LexemeClass`, or the cached class for another name.
--
-- Class lookup may create an undefined placeholder through `getClass`.
local function findLexemeDefinition(_, name)
    if name == "LexemeClass" then return createLexemeClass end
    return getClass(name)
end

defineLexemeClasses(setmetatable({}, {
    __index = findLexemeDefinition,
    __newindex = registerLexemeClass,
}))

-- Names sort siblings independently of declaration order. We validate references
-- before constructing child lists so an unresolved name cannot disappear from the tree.
local declaredClasses = {}
for name, class in pairs(classesByName) do
    assert(class.isDefined, "undefined lexeme class: " .. name)
    class.children = {}
    table.insert(declaredClasses, class)
end
table.sort(declaredClasses, function(a, b) return a.name < b.name end)
local root = assert(classesByName.Lexeme, "Lexeme must be declared")
assert(root.base == nil, "Lexeme must be the root class")
assert(root.isAbstract, "Lexeme must be abstract")
for _, class in ipairs(declaredClasses) do
    if class ~= root then
        local base = assert(class.base, "only Lexeme can be a root class: " .. class.name)
        assert(base.name and classesByName[base.name] == base,
            "a parent must be a named lexeme class: " .. class.name)
        assert(base.isAbstract, "a concrete class cannot have subclasses: " .. base.name)
        table.insert(base.children, class)
    end
end

-- Visiting a class records its first leaf before its children and its end after
-- them. A concrete class contributes itself; an abstract class concatenates the
-- leaves of its children. No class may describe an empty set of concrete lexemes.
local classes = {}
local concreteClasses = {}
local function visit(class)
    assert(class.visitState == nil, "cycle in lexeme hierarchy: " .. class.name)
    class.visitState = "visiting"
    class.firstTag = #concreteClasses
    class.leafClasses = {}
    table.insert(classes, class)
    if class.isAbstract then
        for _, child in ipairs(class.children) do
            visit(child)
            for _, leaf in ipairs(child.leafClasses) do
                table.insert(class.leafClasses, leaf)
            end
        end
        assert(#class.leafClasses > 0, "abstract class has no concrete descendants: " .. class.name)
    else
        assert(#class.children == 0, "a concrete class cannot have subclasses: " .. class.name)
        class.tag = #concreteClasses
        table.insert(concreteClasses, class)
        table.insert(class.leafClasses, class)
    end
    class.endTag = #concreteClasses
    class.visitState = "visited"
end
visit(root)

-- With one root and one parent per class, an unvisited component must contain a
-- cycle. Visiting it reports the cycle rather than silently dropping its classes.
for _, class in ipairs(declaredClasses) do
    if class.visitState ~= "visited" then visit(class) end
end

local trivia = assert(classesByName.Trivia, "Trivia must be declared")
local token = assert(classesByName.Token, "Token must be declared")
assert(trivia.base == root, "Trivia must derive directly from Lexeme")
assert(token.base == root, "Token must derive directly from Lexeme")
for _, class in ipairs(classes) do
    if class ~= root then
        assert(class:isSubclassOf(trivia) or class:isSubclassOf(token),
            "a lexeme class must derive from Trivia or Token: " .. class.name)
    end
end

return {
    classes = classes,
    concreteClasses = concreteClasses,
    classesByName = classesByName,
    root = root,
    trivia = trivia,
    token = token,
}
