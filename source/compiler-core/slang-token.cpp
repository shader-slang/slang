// slang-token.cpp
#include "slang-token.h"

namespace Slang
{

// Kind relationships come from the same table as the wrapper declarations.
// We follow conceptual parents because abstract groups need not occupy adjacent
// numeric ranges in the unified kind space.
bool isLexemeKind(LexemeKind kind, LexemeKind base)
{
    if (kind == LexemeKind::Unknown)
        return false;
    if (kind >= LexemeKind::Count)
        return false;
    for (;;)
    {
        if (kind == base)
            return true;
        if (kind == LexemeKind::Lexeme)
            return false;
        switch (kind)
        {
#define SLANG_LEXEME_KIND(NAME, BASE, CONCRETE, DESCRIPTION) \
    case LexemeKind::NAME:                                   \
        kind = LexemeKind::BASE;                             \
        break;
#include "slang-lexeme-defs.h"
#undef SLANG_LEXEME_KIND
        default:
            SLANG_UNREACHABLE("Invalid lexeme kind.");
        }
    }
}

bool isConcreteLexemeKind(LexemeKind kind)
{
    switch (kind)
    {
#define SLANG_LEXEME_KIND(NAME, BASE, CONCRETE, DESCRIPTION) \
    case LexemeKind::NAME:                                   \
        return CONCRETE;
#include "slang-lexeme-defs.h"
#undef SLANG_LEXEME_KIND
    default:
        return false;
    }
}

char const* TokenTypeToString(TokenType type)
{
    switch (type)
    {
    case TokenType::Unknown:
        return "<unknown>";
#define SLANG_LEXEME_KIND(NAME, BASE, CONCRETE, DESCRIPTION) \
    case TokenType::NAME:                                    \
        return DESCRIPTION;
#include "slang-lexeme-defs.h"
#undef SLANG_LEXEME_KIND
    default:
        SLANG_UNREACHABLE("Invalid lexeme kind.");
    }
}

// Storage for a pool's immutable objects, stable lookup keys, and retained names.
struct TokenPool::Impl
{
    // Lookup keys borrow their text and arrays. We replace those borrowed views
    // with arena-backed views before inserting a new entry into a dictionary.
    /// The location-independent lexical state used to intern an atomic object.
    struct LexemeKey
    {
        LexemeKind kind;
        UnownedStringSlice spelling;
        UnownedStringSlice content;
        Name* name;

        HashCode getHashCode() const
        {
            auto hash = combineHash(Slang::getHashCode(kind), spelling.getHashCode());
            hash = combineHash(hash, content.getHashCode());
            return combineHash(hash, Slang::getHashCode(name));
        }
        bool operator==(const LexemeKey& other) const
        {
            if (kind != other.kind)
                return false;
            if (spelling != other.spelling)
                return false;
            if (content != other.content)
                return false;
            return name == other.name;
        }
    };

    /// The raw object and ordered trivia lists used to intern a cluster.
    struct TokenKey
    {
        const LexemeObj* rawToken;
        ConstArrayView<const LexemeObj*> leading;
        ConstArrayView<const LexemeObj*> trailing;

        HashCode getHashCode() const
        {
            auto hash = Slang::getHashCode(rawToken);
            hash = combineHash(hash, Slang::getHashCode(leading.getCount()));
            for (auto object : leading)
                hash = combineHash(hash, Slang::getHashCode(object));
            hash = combineHash(hash, Slang::getHashCode(trailing.getCount()));
            for (auto object : trailing)
                hash = combineHash(hash, Slang::getHashCode(object));
            return hash;
        }
        bool operator==(const TokenKey& other) const
        {
            if (rawToken != other.rawToken)
                return false;
            if (leading != other.leading)
                return false;
            return trailing == other.trailing;
        }
    };

    MemoryArena arena{16 * 1024};
    Dictionary<LexemeKey, const LexemeObj*> lexemes;
    Dictionary<TokenKey, const TokenObj*> tokens;
    List<RefPtr<Name>> names;
};

TokenPool::TokenPool()
    : m_impl(new Impl())
{
}

TokenPool::~TokenPool()
{
    delete m_impl;
}

/// Tests whether a token denotes only a position and has no spelling.
static bool isZeroWidthTokenKind(TokenType kind)
{
    switch (kind)
    {
    case TokenType::EndOfFile:
    case TokenType::EndOfDirective:
        return true;
    default:
        return false;
    }
}

Lexeme TokenPool::createLexeme(
    LexemeKind kind,
    UnownedStringSlice spelling,
    UnownedStringSlice content,
    SourceLoc loc,
    TokenFlags flags,
    Name* name)
{
    // Abstract kinds and the empty-value kind have no object representation.
    // Identity depends on spelling and an optional interned name, while location
    // and preprocessing context belong to the occurrence returned below.
    SLANG_RELEASE_ASSERT(isConcreteLexemeKind(kind));
    // EOF and directive delimiters denote positions rather than source text.
    // Enforcing their empty spelling here also covers replacement and import.
    if (isZeroWidthTokenKind(kind))
    {
        SLANG_RELEASE_ASSERT(spelling.getLength() == 0);
        SLANG_RELEASE_ASSERT(content.getLength() == 0);
    }
    if (name)
        SLANG_RELEASE_ASSERT(name->text.getUnownedSlice() == content);
    Impl::LexemeKey key{kind, spelling, content, name};
    if (auto found = m_impl->lexemes.tryGetValue(key))
        return Lexeme(*found, loc, flags);

    // We share the two text views when no lexical scrubbing was necessary.
    // This avoids two copies for the common case without borrowing input storage.
    const auto spellingCopy = UnownedStringSlice(
        m_impl->arena.allocateString(spelling.begin(), spelling.getLength()),
        spelling.getLength());
    auto contentCopy = spellingCopy;
    if (content != spelling)
    {
        contentCopy = UnownedStringSlice(
            m_impl->arena.allocateString(content.begin(), content.getLength()),
            content.getLength());
    }

    auto storage = m_impl->arena.allocateAligned(sizeof(LexemeObj), alignof(LexemeObj));
    auto object = new (storage) LexemeObj(this, kind, spellingCopy, contentCopy, name);
    m_impl->lexemes.add(Impl::LexemeKey{kind, spellingCopy, contentCopy, name}, object);
    if (name)
        m_impl->names.add(name);
    return Lexeme(object, loc, flags);
}

Token TokenPool::createToken(
    RawToken rawToken,
    ConstArrayView<const LexemeObj*> leadingTrivia,
    ConstArrayView<const LexemeObj*> trailingTrivia)
{
    // A cluster stores only object pointers. We import foreign objects before
    // interning so its lifetime depends on exactly one pool.
    SLANG_RELEASE_ASSERT(rawToken);
    SLANG_RELEASE_ASSERT(isLexemeKind(rawToken.getType(), LexemeKind::Token));
    SLANG_RELEASE_ASSERT(isConcreteLexemeKind(rawToken.getType()));
    const auto rawObject = _importLexeme(rawToken.getObject());
    List<const LexemeObj*> importedLeading;
    List<const LexemeObj*> importedTrailing;
    const auto leading = _importTrivia(leadingTrivia, importedLeading);
    const auto trailing = _importTrivia(trailingTrivia, importedTrailing);
    Index leadingLength = 0;
    for (auto object : leading)
        leadingLength += object->getSpelling().getLength();

    Impl::TokenKey key{rawObject, leading, trailing};
    if (auto found = m_impl->tokens.tryGetValue(key))
        return Token(*found, rawToken.getLoc(), rawToken.getFlags());

    // The dictionary and object share stable copies of the ordered trivia arrays.
    const auto leadingCopy = makeConstArrayView(
        m_impl->arena.allocateAndCopyArray(leading.getBuffer(), leading.getCount()),
        leading.getCount());
    const auto trailingCopy = makeConstArrayView(
        m_impl->arena.allocateAndCopyArray(trailing.getBuffer(), trailing.getCount()),
        trailing.getCount());
    auto storage = m_impl->arena.allocateAligned(sizeof(TokenObj), alignof(TokenObj));
    auto object = new (storage) TokenObj(rawObject, leadingCopy, trailingCopy, leadingLength);
    m_impl->tokens.add(Impl::TokenKey{rawObject, leadingCopy, trailingCopy}, object);
    return Token(object, rawToken.getLoc(), rawToken.getFlags());
}

const LexemeObj* TokenPool::_importLexeme(const LexemeObj* object)
{
    // A foreign pool may be released independently. We reuse same-pool objects
    // and copy foreign lexical state before retaining any of its pointers.
    SLANG_RELEASE_ASSERT(object);
    if (object->getPool() == this)
        return object;
    auto imported = createLexeme(
        object->getType(),
        object->getSpelling(),
        object->getContent(),
        SourceLoc(),
        0,
        object->getName());
    return imported.getObject();
}

ConstArrayView<const LexemeObj*> TokenPool::_importTrivia(
    ConstArrayView<const LexemeObj*> objects,
    List<const LexemeObj*>& imported)
{
    // Most clusters already belong to one pool and can reuse their views without
    // allocating temporary arrays. A foreign object requires importing the whole
    // list so the resulting view has one owner and remains in its original order.
    bool needsImport = false;
    for (auto object : objects)
    {
        SLANG_RELEASE_ASSERT(object);
        SLANG_RELEASE_ASSERT(isLexemeKind(object->getType(), LexemeKind::Trivia));
        if (object->getPool() != this)
            needsImport = true;
    }
    if (!needsImport)
        return objects;
    for (auto object : objects)
        imported.add(_importLexeme(object));
    return imported.getArrayView();
}

Token TokenPool::createToken(
    TokenType kind,
    UnownedStringSlice content,
    SourceLoc loc,
    TokenFlags flags)
{
    SLANG_RELEASE_ASSERT(isLexemeKind(kind, LexemeKind::Token));
    return createToken(createLexeme(kind, content, content, loc, flags).as<RawToken>());
}

Token TokenPool::createToken(TokenType kind, Name* name, SourceLoc loc, TokenFlags flags)
{
    SLANG_RELEASE_ASSERT(name);
    SLANG_RELEASE_ASSERT(isLexemeKind(kind, LexemeKind::Token));
    const auto content = name->text.getUnownedSlice();
    return createToken(createLexeme(kind, content, content, loc, flags, name).as<RawToken>());
}

Token TokenPool::createEndOfFile(SourceLoc loc, TokenFlags flags)
{
    return createToken(TokenType::EndOfFile, UnownedStringSlice(), loc, flags);
}

RawOpGreater TokenPool::createSecondGreaterToken(RawToken rightShift)
{
    // For `G<G<int>> value;`, the inner argument consumes one closer and leaves the
    // second as lookahead. The lexer ends the `OpRsh` spelling at that second `>`,
    // including any escaped newlines between the two characters. Its final byte
    // therefore locates the second closer without assuming that the bytes are adjacent.
    SLANG_RELEASE_ASSERT(rightShift.getType() == TokenType::OpRsh);
    SLANG_RELEASE_ASSERT(rightShift.getContent() == toSlice(">>"));
    const auto spelling = rightShift.getSpelling();
    SLANG_RELEASE_ASSERT(spelling.getLength() >= 2);
    SLANG_RELEASE_ASSERT(spelling[spelling.getLength() - 1] == '>');

    auto secondLoc = rightShift.getLoc();
    if (secondLoc.isValid())
        secondLoc = secondLoc + (spelling.getLength() - 1);
    const auto greater = toSlice(">");
    return createLexeme(TokenType::OpGreater, greater, greater, secondLoc).as<RawOpGreater>();
}

Token TokenPool::importToken(Token token)
{
    if (!token)
        return token;
    return createToken(
        token.getRawToken(),
        token.getLeadingTrivia().getObjects(),
        token.getTrailingTrivia().getObjects());
}

TriviaList Token::getLeadingTrivia() const
{
    // Leading spelling precedes the raw token in a physical cluster. An absent
    // debug location must stay unknown instead of wrapping around on subtraction.
    if (!m_object)
        return TriviaList();
    auto loc = m_loc;
    if (loc.isValid())
    {
        SLANG_RELEASE_ASSERT(loc.getRaw() >= m_object->getLeadingTriviaLength());
        loc = loc + -m_object->getLeadingTriviaLength();
    }
    return TriviaList(m_object->getLeadingTrivia(), loc);
}

TriviaList Token::getTrailingTrivia() const
{
    // We offset a known raw location by its spelling length. Synthetic or stripped
    // locations remain unknown even when the lexical data has non-empty spelling.
    if (!m_object)
        return TriviaList();
    auto loc = m_loc;
    if (loc.isValid())
        loc = loc + getSpellingLength();
    return TriviaList(m_object->getTrailingTrivia(), loc);
}

Trivia TriviaList::operator[](Index index) const
{
    // Trivia objects have no individual offsets, so indexed access uses the same
    // spelling-length accumulation as sequential iteration.
    SLANG_RELEASE_ASSERT(index >= 0);
    SLANG_RELEASE_ASSERT(index < m_objects.getCount());
    auto iterator = begin();
    for (Index i = 0; i < index; ++i)
        ++iterator;
    return *iterator;
}

void Token::setType(TokenType type)
{
    // Retagging is used for parser-created interpretations and EOF boundaries.
    // EOF is zero-width, so its replacement does not inherit the original text.
    SLANG_RELEASE_ASSERT(m_object);
    if (type == TokenType::Unknown)
    {
        *this = Token();
        return;
    }
    SLANG_RELEASE_ASSERT(isLexemeKind(type, LexemeKind::Token));
    SLANG_RELEASE_ASSERT(isConcreteLexemeKind(type));
    if (isZeroWidthTokenKind(type))
    {
        *this = getPool()->createToken(type, UnownedStringSlice(), m_loc, m_flags);
        return;
    }
    auto raw = getRawToken();
    auto replacement = getPool()->createLexeme(
        type,
        raw.getSpelling(),
        raw.getContent(),
        m_loc,
        m_flags,
        raw.getName());
    *this = getPool()->createToken(
        replacement.as<RawToken>(),
        getLeadingTrivia().getObjects(),
        getTrailingTrivia().getObjects());
}

void Token::setContent(UnownedStringSlice content)
{
    // Changed spelling is synthetic. Existing adjacent trivia cannot retain
    // physical locations after a length change, so this creates a bare token.
    SLANG_RELEASE_ASSERT(m_object);
    *this = getPool()->createToken(getType(), content, m_loc, m_flags);
}

void Token::setName(Name* name)
{
    SLANG_RELEASE_ASSERT(m_object);
    *this = getPool()->createToken(getType(), name, m_loc, m_flags);
}

void Token::setLoc(SourceLoc loc)
{
    // Legacy consumers relocate macro tokens independently. Keeping old trivia
    // would incorrectly imply adjacent bytes at the new location.
    if (m_object)
    {
        if (loc != m_loc)
            *this = getPool()->createToken(getRawToken());
    }
    m_loc = loc;
}

} // namespace Slang
