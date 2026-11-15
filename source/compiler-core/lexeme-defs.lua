-- Lexeme declarations describe a tree; their textual order does not assign tags.
-- We resolve the declarations first, then visit the tree depth first so every class
-- occupies one contiguous range of concrete leaf tags.

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


-- A reference can precede its definition. The environment returns the same class
-- object for both, so children keep a valid parent reference when it is defined later.
local LexemeClass = {}
LexemeClass.__index = LexemeClass
local classesByName = {}
local specificationKeys = {[1] = true, [2] = true, abstract = true}

function LexemeClass:isSubclassOf(base)
    local current = self
    while current do
        if current == base then return true end
        current = current.base
    end
    return false
end

-- A constructor supplies the class's data; assignment supplies its name.
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

local function getClass(name)
    local class = classesByName[name]
    if not class then
        class = setmetatable({name = name, isDefined = false}, LexemeClass)
        classesByName[name] = class
    end
    return class
end

-- We fill a forward reference rather than replace it. Reusing a named class as a
-- definition would give it two identities, so aliases and duplicate names are errors.
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
    class.isDefined = true
end

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
