-- This hierarchy is the source of truth for lexeme kinds and their C++ value wrappers.
-- Abstract cases classify values but are never allocated as objects.
-- The checked-in fiddle output allows the lexer to bootstrap fiddle itself.
local cases = {
    { name = "Lexeme", abstract = true, description = "lexeme" },
    { name = "Trivia", base = "Lexeme", abstract = true, description = "trivia" },
    { name = "Comment", base = "Trivia", abstract = true, description = "comment" },
    { name = "Token", base = "Lexeme", abstract = true, description = "token" },
    { name = "Literal", base = "Token", abstract = true, description = "literal" },
    { name = "NumericLiteral", base = "Literal", abstract = true, description = "numeric literal" },
    { name = "Punctuation", base = "Token", abstract = true, description = "punctuation" },
        { name = "EndOfFile", base = "Token", description = "end of file" },
        { name = "Invalid", base = "Token", description = "invalid character" },
        { name = "Identifier", base = "Token", description = "identifier" },
        { name = "IntegerLiteral", base = "NumericLiteral", description = "integer literal" },
        { name = "FloatingPointLiteral", base = "NumericLiteral", description = "floating-point literal" },
        { name = "StringLiteral", base = "Literal", description = "string literal" },
        { name = "CharLiteral", base = "Literal", description = "character literal" },
        { name = "WhiteSpace", base = "Trivia", description = "whitespace" },
        { name = "NewLine", base = "Trivia", description = "end of line" },
        { name = "LineComment", base = "Comment", description = "line comment" },
        { name = "BlockComment", base = "Comment", description = "block comment" },
        { name = "Semicolon", base = "Punctuation", description = "';'" },
        { name = "Comma", base = "Punctuation", description = "','" },
        { name = "Dot", base = "Punctuation", description = "'.'" },
        { name = "DotDot", base = "Punctuation", description = "'..'" },
        { name = "Ellipsis", base = "Punctuation", description = "'...'" },
        { name = "LBrace", base = "Punctuation", description = "'{'" },
        { name = "RBrace", base = "Punctuation", description = "'}'" },
        { name = "LBracket", base = "Punctuation", description = "'['" },
        { name = "RBracket", base = "Punctuation", description = "']'" },
        { name = "LParent", base = "Punctuation", description = "'('" },
        { name = "RParent", base = "Punctuation", description = "')'" },
        { name = "OpAssign", base = "Punctuation", description = "'='" },
        { name = "OpAdd", base = "Punctuation", description = "'+'" },
        { name = "OpSub", base = "Punctuation", description = "'-'" },
        { name = "OpMul", base = "Punctuation", description = "'*'" },
        { name = "OpDiv", base = "Punctuation", description = "'/'" },
        { name = "OpMod", base = "Punctuation", description = "'%'" },
        { name = "OpNot", base = "Punctuation", description = "'!'" },
        { name = "OpBitNot", base = "Punctuation", description = "'~'" },
        { name = "OpLsh", base = "Punctuation", description = "'<<'" },
        { name = "OpRsh", base = "Punctuation", description = "'>>'" },
        { name = "OpEql", base = "Punctuation", description = "'=='" },
        { name = "OpNeq", base = "Punctuation", description = "'!='" },
        { name = "OpGreater", base = "Punctuation", description = "'>'" },
        { name = "OpLess", base = "Punctuation", description = "'<'" },
        { name = "OpGeq", base = "Punctuation", description = "'>='" },
        { name = "OpLeq", base = "Punctuation", description = "'<='" },
        { name = "OpAnd", base = "Punctuation", description = "'&&'" },
        { name = "OpOr", base = "Punctuation", description = "'||'" },
        { name = "OpBitAnd", base = "Punctuation", description = "'&'" },
        { name = "OpBitOr", base = "Punctuation", description = "'|'" },
        { name = "OpBitXor", base = "Punctuation", description = "'^'" },
        { name = "OpInc", base = "Punctuation", description = "'++'" },
        { name = "OpDec", base = "Punctuation", description = "'--'" },
        { name = "OpAddAssign", base = "Punctuation", description = "'+='" },
        { name = "OpSubAssign", base = "Punctuation", description = "'-='" },
        { name = "OpMulAssign", base = "Punctuation", description = "'*='" },
        { name = "OpDivAssign", base = "Punctuation", description = "'/='" },
        { name = "OpModAssign", base = "Punctuation", description = "'%='" },
        { name = "OpShlAssign", base = "Punctuation", description = "'<<='" },
        { name = "OpShrAssign", base = "Punctuation", description = "'>>='" },
        { name = "OpAndAssign", base = "Punctuation", description = "'&='" },
        { name = "OpOrAssign", base = "Punctuation", description = "'|='" },
        { name = "OpXorAssign", base = "Punctuation", description = "'^='" },
        { name = "QuestionMark", base = "Punctuation", description = "'?'" },
        { name = "Colon", base = "Punctuation", description = "':'" },
        { name = "RightArrow", base = "Punctuation", description = "'->'" },
        { name = "DoubleRightArrow", base = "Punctuation", description = "'=>'" },
        { name = "At", base = "Punctuation", description = "'@'" },
        { name = "Dollar", base = "Punctuation", description = "'$'" },
        { name = "DollarDollar", base = "Punctuation", description = "'$$'" },
        { name = "Pound", base = "Punctuation", description = "'#'" },
        { name = "PoundPound", base = "Punctuation", description = "'##'" },
        { name = "Scope", base = "Punctuation", description = "'::'" },
        { name = "CompletionRequest", base = "Punctuation", description = "'#?'" },
    { name = "EscapedNewLine", base = "Trivia", description = "escaped end of line" },
    { name = "HeaderName", base = "Token", description = "header name" },
    { name = "EndOfDirective", base = "Token", description = "end of preprocessor directive" },
}

-- We compute wrapper names from the conceptual hierarchy so raw tokens and
-- tokens with trivia share one kind space without sharing their C++ bases.
local function generate()
    local byName = {}
    for _, case in ipairs(cases) do
        assert(not byName[case.name], "duplicate lexeme kind: " .. case.name)
        byName[case.name] = case
    end

    -- A concrete case must be a leaf, and every ancestry chain must terminate
    -- at the abstract root. We reject invalid data before generating C++.
    for _, case in ipairs(cases) do
        local ancestors = {}
        local current = case
        while current.base do
            assert(not ancestors[current.name], "cyclic lexeme ancestry: " .. case.name)
            ancestors[current.name] = true
            local parent = byName[current.base]
            assert(parent, "unknown lexeme base: " .. current.base)
            assert(parent.abstract, "concrete lexeme has a subtype: " .. parent.name)
            current = parent
        end
        assert(current.name == "Lexeme", "lexeme has a different root: " .. case.name)
        assert(current.abstract, "the lexeme root must be abstract")
    end

    local function isTrivia(case)
        if case.name == "Trivia" then return true end
        if not case.base then return false end
        return isTrivia(byName[case.base])
    end
    local function rawName(case)
        if case.name == "Lexeme" or case.name == "Trivia" then return case.name end
        if isTrivia(case) then return case.name .. "Trivia" end
        return "Raw" .. case.name
    end
    local function tokenName(case)
        if case.name == "Token" then return "Token" end
        return case.name .. "Token"
    end
    local lines = {
        "// Generated from slang-lexeme-defs.lua. Regenerate with regenerate-lexeme-definitions.",
        "#ifdef SLANG_LEXEME_KIND",
    }
    for _, case in ipairs(cases) do
        table.insert(lines, string.format('SLANG_LEXEME_KIND(%s, %s, %s, %q)',
            case.name, case.base or case.name, case.abstract and "false" or "true",
            case.description))
    end
    table.insert(lines, "#endif")
    table.insert(lines, "#ifdef SLANG_RAW_LEXEME")
    for _, case in ipairs(cases) do
        if case.name ~= "Lexeme" then
            table.insert(lines, string.format('SLANG_RAW_LEXEME(%s, %s, %s)',
                case.name, rawName(case), rawName(byName[case.base])))
        end
    end
    table.insert(lines, "#endif")
    table.insert(lines, "#ifdef SLANG_TOKEN_WRAPPER")
    for _, case in ipairs(cases) do
        if case.name ~= "Lexeme" and case.name ~= "Token" and not isTrivia(case) then
            table.insert(lines, string.format('SLANG_TOKEN_WRAPPER(%s, %s, %s)',
                case.name, tokenName(case), tokenName(byName[case.base])))
        end
    end
    table.insert(lines, "#endif")
    return table.concat(lines, "\n") .. "\n"
end

return { cases = cases, generate = generate }
