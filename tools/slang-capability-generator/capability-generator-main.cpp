// capabilities-generator-main.cpp

#include "compiler-core/slang-lexer.h"
#include "compiler-core/slang-perfect-hash-codegen.h"
#include "core/slang-file-system.h"
#include "core/slang-io.h"
#include "core/slang-secure-crt.h"
#include "core/slang-string-util.h"
#include "core/slang-uint-set.h"

#include <stdio.h>

using namespace Slang;

namespace Diagnostics
{
#define DIAGNOSTIC(id, severity, name, messageFormat) \
    const DiagnosticInfo name = {id, Severity::severity, #name, messageFormat};
#include "slang-capability-diagnostic-defs.h"
#undef DIAGNOSTIC
} // namespace Diagnostics

enum class CapabilityFlavor
{
    Normal,
    Abstract,
    Alias
};

struct CapabilityDef;

struct CapabilityConjunctionExpr
{
    List<CapabilityDef*> atoms;
    SourceLoc sourceLoc;
};

struct CapabilityDisjunctionExpr
{
    List<CapabilityConjunctionExpr> conjunctions;
};

struct SerializedArrayView
{
    Index first;
    Index count;
};

struct CapabilitySharedContext
{
    CapabilityDef* ptrOfTarget = nullptr;
    CapabilityDef* ptrOfStage = nullptr;
};

static void _removeFromOtherAtomsNotInThis(
    HashSet<const CapabilityDef*> thisSet,
    HashSet<const CapabilityDef*> otherSet,
    List<const CapabilityDef*> atomsToRemove)
{
    atomsToRemove.clear();
    atomsToRemove.reserve(otherSet.getCount());
    for (auto keyAtom : otherSet)
    {
        if (thisSet.contains(keyAtom))
            continue;
        atomsToRemove.add(keyAtom);
    }

    for (auto atomToRemove : atomsToRemove)
        otherSet.remove(atomToRemove);
}

enum class AutoDocHeaderGroup : UInt
{
    Targets = 0,
    Stages,
    Versions,
    Extensions,
    Compound,
    Other,
    Count,
    Invalid,
};

UnownedStringSlice getHeaderNameFromAutoDocHeaderGroup(UInt headerGroup)
{
    switch (headerGroup)
    {
    case (UInt)AutoDocHeaderGroup::Targets:
        return UnownedStringSlice("Targets");
    case (UInt)AutoDocHeaderGroup::Stages:
        return UnownedStringSlice("Stages");
    case (UInt)AutoDocHeaderGroup::Extensions:
        return UnownedStringSlice("Extensions");
    case (UInt)AutoDocHeaderGroup::Versions:
        return UnownedStringSlice("Versions");
    case (UInt)AutoDocHeaderGroup::Compound:
        return UnownedStringSlice("Compound Capabilities");
    case (UInt)AutoDocHeaderGroup::Other:
        return UnownedStringSlice("Other");
    default:
        SLANG_ASSERT("Unknown `AutoDocHeaderGroup`");
        return UnownedStringSlice("");
    }
}

UnownedStringSlice getHeaderDescriptionFromAutoDocHeaderGroup(UInt headerGroup)
{
    switch (headerGroup)
    {
    case (UInt)AutoDocHeaderGroup::Targets:
        return UnownedStringSlice(
            "Capabilities to specify code generation targets (`glsl`, `spirv`...)");
    case (UInt)AutoDocHeaderGroup::Stages:
        return UnownedStringSlice(
            "Capabilities to specify code generation stages (`vertex`, `fragment`...)");
    case (UInt)AutoDocHeaderGroup::Extensions:
        return UnownedStringSlice("Capabilities to specify extensions (`GL_EXT`, `SPV_EXT`...)");
    case (UInt)AutoDocHeaderGroup::Versions:
        return UnownedStringSlice("Capabilities to specify versions of a code generation "
                                  "target (`sm_5_0`, `GLSL_400`...)");
    case (UInt)AutoDocHeaderGroup::Compound:
        return UnownedStringSlice("Capabilities to specify capabilities created by other "
                                  "capabilities (`raytracing`, `meshshading`...)");
    case (UInt)AutoDocHeaderGroup::Other:
        return UnownedStringSlice("Capabilities which may be deprecated");
    default:
        SLANG_ASSERT("Unknown `AutoDocHeaderGroup`");
        return UnownedStringSlice("");
    }
}

AutoDocHeaderGroup getAutoDocHeaderGroupFromTag(
    DiagnosticSink* sink,
    UnownedStringSlice headerGroupName,
    SourceLoc loc)
{
    if (headerGroupName.caseInsensitiveEquals(UnownedStringSlice("Other")))
        return AutoDocHeaderGroup::Other;
    else if (headerGroupName.caseInsensitiveEquals(UnownedStringSlice("Target")))
        return AutoDocHeaderGroup::Targets;
    else if (headerGroupName.caseInsensitiveEquals(UnownedStringSlice("Stage")))
        return AutoDocHeaderGroup::Stages;
    else if (headerGroupName.caseInsensitiveEquals(UnownedStringSlice("EXT")))
        return AutoDocHeaderGroup::Extensions;
    else if (headerGroupName.caseInsensitiveEquals(UnownedStringSlice("Version")))
        return AutoDocHeaderGroup::Versions;
    else if (headerGroupName.caseInsensitiveEquals(UnownedStringSlice("Compound")))
        return AutoDocHeaderGroup::Compound;
    else
    {
        sink->diagnose(loc, Diagnostics::invalidDocCommentHeader, headerGroupName);
        return AutoDocHeaderGroup::Invalid;
    }
}

struct AutoDocInfo
{
    String comment;
    AutoDocHeaderGroup headerGroup;

    AutoDocInfo()
    {
        comment = {};
        headerGroup = AutoDocHeaderGroup::Other;
    }
};

struct CapabilityDef : public RefObject
{
public:
    void operator=(const CapabilityDef& other)
    {
        this->name = other.name;
        this->enumValue = other.enumValue;
        this->expr = other.expr;
        this->flavor = other.flavor;
        this->rank = other.rank;
        this->canonicalRepresentation = other.canonicalRepresentation;
        this->serializedCanonicalRepresentation = other.serializedCanonicalRepresentation;
        this->sourceLoc = other.sourceLoc;
        this->keyAtomsPresent = other.keyAtomsPresent;
        this->sharedContext = other.sharedContext;
        this->docComment = other.docComment;
    }

    String name;
    Index enumValue;
    CapabilityDisjunctionExpr expr;
    CapabilityFlavor flavor;
    /// optional, 0 is default rank.
    int rank = 0;
    List<List<CapabilityDef*>> canonicalRepresentation;
    SerializedArrayView serializedCanonicalRepresentation;
    SourceLoc sourceLoc;
    AutoDocInfo docComment;
    /// Stores key atoms a CapabilityDef refers to.
    /// Shared key atoms: key atoms shared between every individual set in a
    /// canonicalRepresentation, added together.
    HashSet<const CapabilityDef*> keyAtomsPresent;

    CapabilitySharedContext* sharedContext;

    CapabilityDef* getAbstractBase() const
    {
        if (flavor != CapabilityFlavor::Normal)
            return nullptr;
        if (expr.conjunctions.getCount() != 1)
            return nullptr;
        if (expr.conjunctions[0].atoms.getCount() == 0)
            return nullptr;
        if (expr.conjunctions[0].atoms[0]->flavor != CapabilityFlavor::Abstract)
            return nullptr;
        return expr.conjunctions[0].atoms[0];
    }

    void fillKeyAtomsPresentInCannonicalRepresentation()
    {
        HashSet<const CapabilityDef*> sharedKeyAtomsInCanonicalSet_target;
        HashSet<const CapabilityDef*> sharedKeyAtomsInCanonicalSet_stage;
        HashSet<const CapabilityDef*> keyAtomsFound;
        List<const CapabilityDef*> atomsToRemove;
        for (auto& canonicalSet : canonicalRepresentation)
        {
            bool alreadySetTarget = false;
            bool alreadySetStage = false;
            sharedKeyAtomsInCanonicalSet_target.clear();
            sharedKeyAtomsInCanonicalSet_stage.clear();

            // find key atoms all atoms in a canonical set share.
            for (auto& atom : canonicalSet)
            {
                bool foundTarget = false;
                bool foundStage = false;
                for (auto otherkeyAtomsPresent : atom->keyAtomsPresent)
                {
                    auto base = otherkeyAtomsPresent->getAbstractBase();
                    // add all `target` key atoms associated with atom in canonicalSet
                    if (base == sharedContext->ptrOfTarget)
                    {
                        foundTarget = true;
                        if (!alreadySetTarget)
                            sharedKeyAtomsInCanonicalSet_target.add(otherkeyAtomsPresent);
                    }
                    // add all `stage` key atoms associated with atom in canonicalSet
                    else if (base == sharedContext->ptrOfStage)
                    {
                        foundStage = true;
                        if (!alreadySetTarget)
                            sharedKeyAtomsInCanonicalSet_stage.add(otherkeyAtomsPresent);
                    }
                    // all key atoms associated with atom
                    keyAtomsFound.add(otherkeyAtomsPresent);
                }

                // remove all not shared key atoms
                if (foundTarget)
                {
                    alreadySetTarget = true;
                    _removeFromOtherAtomsNotInThis(
                        keyAtomsFound,
                        sharedKeyAtomsInCanonicalSet_target,
                        atomsToRemove);
                }
                if (foundStage)
                {
                    alreadySetStage = true;
                    _removeFromOtherAtomsNotInThis(
                        keyAtomsFound,
                        sharedKeyAtomsInCanonicalSet_stage,
                        atomsToRemove);
                }
                keyAtomsFound.clear();
            }
            // TODO: PR #4237 (ArielG-NV) - alreadySetStage is set but never read.
            // Line 249 uses !alreadySetTarget instead of !alreadySetStage, which is likely a bug.
            SLANG_UNUSED(alreadySetStage);

            // add all shared key atoms
            for (auto keyAtom : sharedKeyAtomsInCanonicalSet_target)
                this->keyAtomsPresent.add(keyAtom);
            for (auto keyAtom : sharedKeyAtomsInCanonicalSet_stage)
                this->keyAtomsPresent.add(keyAtom);
        }
        if (auto base = this->getAbstractBase(); base)
            keyAtomsPresent.add(this);
    }
};

static bool isInternalDef(RefPtr<CapabilityDef> def)
{
    return def->name.startsWith("_");
}

struct CapabilityDefParser
{
    CapabilityDefParser(Lexer* lexer, DiagnosticSink* sink, CapabilitySharedContext& sharedContext)
        : m_lexer(lexer), m_sink(sink), m_sharedContext(sharedContext)
    {
    }

    Lexer* m_lexer;
    DiagnosticSink* m_sink;

    Dictionary<String, CapabilityDef*> m_mapNameToCapability;
    List<RefPtr<CapabilityDef>> m_defs;
    CapabilitySharedContext& m_sharedContext;

    LexemeReader m_lexemeReader;

    /// Skips comments within a definition, leaving the next grammar token unconsumed.
    void skipComments()
    {
        // Comments within a definition do not contribute documentation or grammar symbols.
        // The top-level definition loop reads comments itself to collect documentation.
        while (m_lexemeReader.peekLexeme().is<CommentTrivia>())
            m_lexemeReader.readLexeme();
    }

    /// Skips comments and consumes a token of type `T`, or returns an empty `T`.
    template<typename T>
    T advanceIf()
    {
        skipComments();
        auto token = m_lexemeReader.peekLexeme().as<T>();
        if (token)
            m_lexemeReader.readLexeme();
        return token;
    }

    /// Skips comments, consumes the next lexeme, and diagnoses a mismatch with type `T`.
    ///
    /// On success, `token` receives the checked raw token. On failure, `token` is empty.
    template<typename T>
    SlangResult readToken(T& token)
    {
        // A required read consumes one significant lexeme, then exposes a typed token only
        // when its kind matches. Callers return on failure before using the empty result.
        skipComments();
        auto lexeme = m_lexemeReader.readLexeme();
        token = lexeme.as<T>();
        if (!token)
        {
            m_sink->diagnose(
                lexeme.getLoc(),
                Diagnostics::unexpectedTokenExpectedTokenType,
                lexeme,
                T::kind);
            return SLANG_FAIL;
        }
        return SLANG_OK;
    }

    template<typename T>
    SlangResult readToken()
    {
        T token;
        return readToken(token);
    }

    SlangResult parseConjunction(CapabilityConjunctionExpr& expr)
    {
        // We resolve each identifier in a `+`-separated conjunction as it is read.
        // Definition order therefore determines which capability names are available.
        for (;;)
        {
            RawIdentifier nameToken;
            SLANG_RETURN_ON_FAIL(readToken(nameToken));
            CapabilityDef* def = nullptr;
            if (m_mapNameToCapability.tryGetValue(nameToken.getContent(), def))
            {
                expr.atoms.add(def);
            }
            else
            {
                m_sink->diagnose(nameToken.getLoc(), Diagnostics::undefinedIdentifier, nameToken);
                return SLANG_FAIL;
            }
            if (!advanceIf<RawOpAdd>())
                break;
        }
        return SLANG_OK;
    }

    SlangResult parseExpr(CapabilityDisjunctionExpr& expr)
    {
        // We parse each `|`-separated conjunction independently so `+` binds more tightly.
        for (;;)
        {
            CapabilityConjunctionExpr conjunction;
            conjunction.sourceLoc = m_lexemeReader.peekLoc();
            SLANG_RETURN_ON_FAIL(parseConjunction(conjunction));
            expr.conjunctions.add(conjunction);
            if (!advanceIf<RawOpBitOr>())
                break;
        }
        return SLANG_OK;
    }

    void validateInternalAtomExternalAtomPair()
    {
        // All `_Internal` atoms must have an `External` atom.
        // `External` atoms do not require to have an `_Internal` atom.
        // The following behavior ensures that if we error with 'atom' instead of
        // '_atom' a user may add the 'atom' capability to solve their error. This is
        // important because '_Internal' will only be for 1 target, 'External' will alias
        // to more than 1 target. We need to ensure users avoid 'Internal' when possible.

        Dictionary<String, List<RefPtr<CapabilityDef>>> nameToInternalAndExternalAtom;
        for (auto i : m_defs)
        {
            // 'abstract' atoms are not reported to a user and are ignored
            if (i->flavor == CapabilityFlavor::Abstract)
                continue;

            // Try to pack `_atom` and `atom` into the same per key List
            String name = i->name;
            if (i->name.startsWith("_"))
                name = name.subString(1, name.getLength() - 1);
            nameToInternalAndExternalAtom[name].add(i);
        }
        for (auto i : nameToInternalAndExternalAtom)
        {
            SLANG_ASSERT(i.second.getCount() <= 2);
            if (i.second.getCount() != 2)
            {
                // If we only have a '_Internal' atom inside our name list there is a missing
                // 'External' atom
                if (i.second[0]->name.startsWith("_"))
                    m_sink->diagnose(
                        i.second[0]->sourceLoc,
                        Diagnostics::missingExternalInternalAtomPair,
                        i.second[0]->name);
            }
        }
    }

    bool isLineSuccessive(HumaneSourceLoc above, HumaneSourceLoc below)
    {
        return above.line + 1 == below.line;
    }

    SlangResult parseDefs()
    {
        // Documentation comments participate in the top-level scan, but whitespace and
        // newline trivia do not. We retain comments and raw tokens in the reader's list;
        // adjacency between documentation lines is checked with the source file's line map.
        LexemeList lexemes;
        for (;;)
        {
            auto lexeme = m_lexer->readLexeme();
            if (lexeme.is<RawToken>())
                lexemes.add(lexeme);
            else if (lexeme.is<CommentTrivia>())
                lexemes.add(lexeme);
            if (lexeme.is<RawEndOfFile>())
                break;
        }
        m_lexemeReader = LexemeReader(lexemes);
        AutoDocInfo successiveComments = AutoDocInfo();
        HumaneSourceLoc successiveCommentLine = {};

        for (;;)
        {
            auto nextLexeme = m_lexemeReader.readLexeme();

            if (!isLineSuccessive(
                    successiveCommentLine,
                    m_lexer->m_sourceView->getHumaneLoc(nextLexeme.getLoc())))
                successiveComments = AutoDocInfo();

            RefPtr<CapabilityDef> def = new CapabilityDef();
            def->sharedContext = &m_sharedContext;
            def->flavor = CapabilityFlavor::Normal;
            if (nextLexeme.getContent() == "alias")
            {
                def->flavor = CapabilityFlavor::Alias;
            }
            else if (nextLexeme.getContent() == "abstract")
            {
                def->flavor = CapabilityFlavor::Abstract;
            }
            else if (nextLexeme.getContent() == "def")
            {
                def->flavor = CapabilityFlavor::Normal;
            }
            else if (nextLexeme.is<BlockCommentTrivia>())
            {
                // Do not auto-document
                continue;
            }
            else if (nextLexeme.is<LineCommentTrivia>())
            {
                // Auto-document if the preceeding token to an identifier is '///'
                // complete rules described in `source\slang\slang-capabilities.capdef`
                auto commentContent = nextLexeme.getContent();

                // remove "//"
                commentContent = commentContent.subString(2, commentContent.getLength() - 2);
                if (commentContent.startsWith("/"))
                {
                    auto commentLine = m_lexer->m_sourceView->getHumaneLoc(nextLexeme.getLoc());

                    // Reset the `successiveCommentLine` to our newest commentLine
                    successiveCommentLine = commentLine;

                    // remove "/" from "///"
                    commentContent =
                        commentContent.subString(1, commentContent.getLength() - 1).trim();

                    // Check if we have a `[header]`
                    if (commentContent.startsWith("["))
                    {
                        // Make a substring of `header]`
                        auto consumedLeftBracketOfHeader =
                            commentContent.subString(1, commentContent.getLength() - 1);
                        // Find a `]` of `header]` if it exists
                        auto indexOfHeaderEnd = consumedLeftBracketOfHeader.indexOf(']');
                        if (indexOfHeaderEnd != -1)
                        {
                            // We found our `header`
                            auto headerName =
                                consumedLeftBracketOfHeader.subString(0, indexOfHeaderEnd);
                            successiveComments.headerGroup = getAutoDocHeaderGroupFromTag(
                                m_sink,
                                headerName,
                                nextLexeme.getLoc());
                            continue;
                        }
                        // If we did not find a header this is a regular comment
                    }
                    successiveComments.comment.append("> ");
                    successiveComments.comment.append(commentContent);
                    successiveComments.comment.append("\n");
                }
                continue;
            }
            else if (nextLexeme.is<RawEndOfFile>())
            {
                break;
            }
            else
            {
                m_sink->diagnose(nextLexeme.getLoc(), Diagnostics::unexpectedToken, nextLexeme);
                return SLANG_FAIL;
            }

            RawIdentifier nameToken;
            SLANG_RETURN_ON_FAIL(readToken(nameToken));
            def->name = nameToken.getContent();

            if (def->flavor == CapabilityFlavor::Normal)
            {
                if (advanceIf<RawColon>())
                {
                    SLANG_RETURN_ON_FAIL(parseExpr(def->expr));
                }
                if (advanceIf<RawOpAssign>())
                {
                    RawIntegerLiteral rankToken;
                    SLANG_RETURN_ON_FAIL(readToken(rankToken));
                    def->rank = stringToInt(rankToken.getContent());
                }
                def->docComment = successiveComments;
                if (def->docComment.comment.getLength() == 0 && !isInternalDef(def))
                    m_sink->diagnose(
                        nextLexeme.getLoc(),
                        Diagnostics::requiresDocComment,
                        def->name);
            }
            else if (def->flavor == CapabilityFlavor::Alias)
            {
                SLANG_RETURN_ON_FAIL(readToken<RawOpAssign>());
                SLANG_RETURN_ON_FAIL(parseExpr(def->expr));
                def->docComment = successiveComments;
                if (def->docComment.comment.getLength() == 0 && !isInternalDef(def))
                    m_sink->diagnose(
                        nextLexeme.getLoc(),
                        Diagnostics::requiresDocComment,
                        def->name);
            }
            else if (def->flavor == CapabilityFlavor::Abstract)
            {
                if (advanceIf<RawColon>())
                {
                    SLANG_RETURN_ON_FAIL(parseExpr(def->expr));
                }
            }
            SLANG_RETURN_ON_FAIL(readToken<RawSemicolon>());
            m_defs.add(def);
            if (!m_mapNameToCapability.addIfNotExists(def->name, m_defs.getLast()))
            {
                m_sink->diagnose(nextLexeme.getLoc(), Diagnostics::redefinition, def->name);
                return SLANG_FAIL;
            }

            // set abstract atom identifiers
            if (!m_sharedContext.ptrOfTarget && def->name.equals("target"))
                m_sharedContext.ptrOfTarget = m_defs.getLast();
            else if (!m_sharedContext.ptrOfStage && def->name.equals("stage"))
                m_sharedContext.ptrOfStage = m_defs.getLast();

            def->sourceLoc = nameToken.getLoc();
        }
        validateInternalAtomExternalAtomPair();
        if (m_sink->getErrorCount())
            return SLANG_FAIL;
        return SLANG_OK;
    }
};

struct CapabilityConjunction
{
    HashSet<CapabilityDef*> atoms;

    String toString() const
    {
        bool first = true;
        String result = "[";
        for (auto atom : atoms)
        {
            if (!first)
            {
                result.append(" + ");
            }
            first = false;
            result.append(atom->name);
        }
        result.appendChar(']');
        return result;
    }

    bool implies(const CapabilityConjunction& c) const
    {
        for (auto& atom : c.atoms)
        {
            if (!atoms.contains(atom))
                return false;
        }
        return true;
    }

    const CapabilityDef* getAbstractAtom(CapabilityDef* defToFilterFor) const
    {
        for (auto* atom : this->atoms)
        {
            for (auto present : atom->keyAtomsPresent)
            {
                auto base = present->getAbstractBase();
                if (base != defToFilterFor)
                    continue;
                return present;
            }
        }
        return nullptr;
    }

    bool shareTargetAndStageAtom(
        const CapabilityConjunction& other,
        CapabilitySharedContext& context)
    {
        // shared target means thisTarget==otherTarget
        // shared stage means either `nostage + ...` or `stage == stage`

        const CapabilityDef* thisTarget = this->getAbstractAtom(context.ptrOfTarget);
        const CapabilityDef* otherTarget = other.getAbstractAtom(context.ptrOfTarget);

        if (thisTarget != otherTarget && thisTarget && otherTarget)
            return false;

        const CapabilityDef* thisStage = this->getAbstractAtom(context.ptrOfStage);
        const CapabilityDef* otherStage = other.getAbstractAtom(context.ptrOfStage);

        if (thisStage != otherStage && thisStage && otherStage)
            return false;

        return true;
    }

    bool isImpossible() const
    {
        // Keep a map from an abstract base to the concrete atom defined in this conjunction that
        // implements the base.
        Dictionary<CapabilityDef*, CapabilityDef*> abstractKV;

        for (auto& atom : atoms)
        {
            auto abstractBase = atom->getAbstractBase();
            if (!abstractBase)
                continue;

            // Have we already seen another concrete atom that implements the same abstract base of
            // the current atom? If so, we have a conflict and the conjunction is impossible.
            //
            CapabilityDef* value = nullptr;
            if (abstractKV.tryGetValue(abstractBase, value))
            {
                if (value != atom)
                    return true;
            }
            else
            {
                abstractKV[abstractBase] = atom;
            }
        }
        return false;
    }
};

struct CapabilityDisjunction
{
    List<CapabilityConjunction> conjunctions;

    void addConjunction(
        DiagnosticSink* sink,
        SourceLoc sourceLoc,
        CapabilitySharedContext& context,
        CapabilityConjunction& c)
    {
        if (c.isImpossible())
            return;
        bool cImpliesThis = false;
        for (Index i = 0; i < conjunctions.getCount();)
        {
            // implied sets will be replaced
            if (c.implies(conjunctions[i]))
            {
                cImpliesThis = true;
                conjunctions.fastRemoveAt(i);
            }
            else
                i++;
        }
        if (cImpliesThis)
        {
            conjunctions.add(_Move(c));
            return;
        }

        for (Index i = 0; i < conjunctions.getCount();)
        {
            if (conjunctions[i].implies(c))
            {
                // subset is implied, we do not need to add it.
                return;
            }
            else
            {
                // validate we are not creating a disjunction of same targets
                if (conjunctions[i].shareTargetAndStageAtom(c, context))
                {
                    if (sink)
                    {
                        sink->diagnose(
                            sourceLoc,
                            Diagnostics::unionWithSameKeyAtomButNotSubset,
                            conjunctions[i].toString(),
                            c.toString());
                        sink = nullptr;
                    }
                }
                i++;
            }
        }
        conjunctions.add(_Move(c));
    }
    void removeImplied()
    {
        for (Index i = 0; i < conjunctions.getCount(); i++)
        {
            for (Index ii = 0; ii < conjunctions.getCount(); ii++)
            {
                if (ii == i)
                    continue;

                if (!conjunctions[i].implies(conjunctions[ii]))
                    continue;

                if (i < ii)
                {
                    conjunctions.fastRemoveAt(ii);
                }
                else
                {
                    conjunctions.removeAt(ii);
                    i--;
                }
                ii--;
            }
        }
    }

    void inclusiveJoinConjunction(
        CapabilitySharedContext& context,
        CapabilityConjunction& c,
        List<CapabilityConjunction>& toAddAfter)
    {
        if (c.isImpossible())
            return;
        for (auto& conjunction : conjunctions)
        {
            if (conjunction.implies(c))
                return;
        }
        for (Index i = 0; i < conjunctions.getCount();)
        {
            if (conjunctions[i].shareTargetAndStageAtom(c, context))
            {
                CapabilityConjunction toAddAfterSet;
                for (auto atom : conjunctions[i].atoms)
                    toAddAfterSet.atoms.add(atom);
                for (auto atom : c.atoms)
                    toAddAfterSet.atoms.add(atom);
                toAddAfter.add(toAddAfterSet);
                return;
            }
            else
            {
                i++;
            }
        }
        conjunctions.add(_Move(c));
    }

    CapabilityDisjunction joinWith(
        DiagnosticSink* sink,
        SourceLoc sourceLoc,
        CapabilitySharedContext& context,
        const CapabilityDisjunction& other)
    {
        if (conjunctions.getCount() == 0)
        {
            return other;
        }
        if (other.conjunctions.getCount() == 0)
        {
            return *this;
        }

        CapabilityDisjunction result;

        for (auto& thisC : conjunctions)
        {
            for (auto& thatC : other.conjunctions)
            {
                CapabilityConjunction newC;
                for (auto atom : thisC.atoms)
                    newC.atoms.add(atom);
                for (auto atom : thatC.atoms)
                    newC.atoms.add(atom);
                result.addConjunction(sink, sourceLoc, context, newC);
            }
        }

        // incompatible abstract atoms
        if (result.conjunctions.getCount() == 0)
            sink->diagnose(sourceLoc, Diagnostics::invalidJoinInGenerator);

        return result;
    }

    List<List<CapabilityDef*>> canonicalize()
    {
        List<List<CapabilityDef*>> result;
        for (auto& c : conjunctions)
        {
            List<CapabilityDef*> atoms;
            for (auto& atom : c.atoms)
                atoms.add(atom);
            atoms.sort([](CapabilityDef* c1, CapabilityDef* c2)
                       { return c1->enumValue < c2->enumValue; });
            result.add(_Move(atoms));
        }
        result.sort(
            [](const List<CapabilityDef*>& c1, const List<CapabilityDef*>& c2)
            {
                for (Index i = 0; i < Math::Min(c1.getCount(), c2.getCount()); i++)
                {
                    if (c1[i]->enumValue < c2[i]->enumValue)
                        return true;
                    else if (c1[i]->enumValue > c2[i]->enumValue)
                        return false;
                }
                return c1.getCount() < c2.getCount();
            });
        return result;
    }
};

CapabilityDisjunction getCanonicalRepresentation(CapabilityDef* def)
{
    CapabilityDisjunction result;
    for (auto& c : def->canonicalRepresentation)
    {
        CapabilityConjunction conj;
        for (auto& atom : c)
            conj.atoms.add(atom);
        result.conjunctions.add(conj);
    }
    return result;
}

CapabilityDisjunction evaluateConjunction(
    DiagnosticSink* sink,
    SourceLoc sourceLoc,
    CapabilitySharedContext& context,
    const List<CapabilityDef*>& atoms)
{
    CapabilityDisjunction result;
    for (auto* def : atoms)
    {
        CapabilityDisjunction defCanonical = getCanonicalRepresentation(def);
        result = result.joinWith(sink, sourceLoc, context, defCanonical);
    }
    return result;
}

void calcCanonicalRepresentation(
    DiagnosticSink* sink,
    CapabilityDef* def,
    const List<CapabilityDef*>& mapEnumValueToDef)
{
    CapabilityDisjunction disjunction;
    if (def->flavor == CapabilityFlavor::Normal)
    {
        CapabilityConjunction c;
        c.atoms.add(def);
        disjunction.conjunctions.add(c);
    }
    CapabilityDisjunction exprVal;
    for (auto& c : def->expr.conjunctions)
    {
        CapabilityDisjunction evalD =
            evaluateConjunction(sink, c.sourceLoc, *def->sharedContext, c.atoms);
        List<CapabilityConjunction> toAddAfter;
        for (auto& cc : evalD.conjunctions)
        {
            exprVal.inclusiveJoinConjunction(*def->sharedContext, cc, toAddAfter);
        }
        for (auto& i : toAddAfter)
            exprVal.conjunctions.add(i);
        if (toAddAfter.getCount() > 0)
            exprVal.removeImplied();
    }
    disjunction = disjunction.joinWith(sink, def->sourceLoc, *def->sharedContext, exprVal);
    def->canonicalRepresentation = disjunction.canonicalize();
    def->fillKeyAtomsPresentInCannonicalRepresentation();
}

void calcCanonicalRepresentations(
    DiagnosticSink* sink,
    List<RefPtr<CapabilityDef>>& defs,
    const List<CapabilityDef*>& mapEnumValueToDef)
{
    for (auto def : defs)
        calcCanonicalRepresentation(sink, def, mapEnumValueToDef);
}

// Create a local UIntSet with data
void outputLocalUIntSetBuffer(
    const String& nameOfBuffer,
    StringBuilder& resultBuilder,
    UIntSet& set)
{
    resultBuilder << "    CapabilityAtomSet " << nameOfBuffer << ";\n";
    resultBuilder << "    " << nameOfBuffer << ".resizeBackingBufferDirectly("
                  << set.getBuffer().getCount() << ");\n";
    for (Index i = 0; i < set.getBuffer().getCount(); i++)
    {
        resultBuilder << "    " << nameOfBuffer << ".addRawElement(UIntSet::Element("
                      << set.getBuffer()[i] << "UL), " << i << "); \n";
    }
}

// Create function to generate a UIntSet with initial data
void outputUIntSetGenerator(
    const String& nameOfGenerator,
    StringBuilder& resultBuilder,
    UIntSet& set)
{
    resultBuilder << "inline static CapabilityAtomSet " << nameOfGenerator << "()\n";
    resultBuilder << "{\n";
    auto nameOfBackingData = nameOfGenerator + "_data";
    outputLocalUIntSetBuffer(nameOfBackingData, resultBuilder, set);
    resultBuilder << "    return " << nameOfBackingData << ";\n";
    resultBuilder << "}\n";
}


UIntSet atomSetToUIntSet(const List<CapabilityDef*>& atomSet)
{
    UIntSet set{};
    // Last element is generally a larger number. Start from there to minimize reallocations.
    for (Index i = atomSet.getCount() - 1; i >= 0; i--)
        set.add(atomSet[i]->enumValue);
    return set;
}

void printDocForCapabilityDef(
    StringBuilder& sbDoc,
    RefPtr<CapabilityDef> def,
    List<StringBuilder>& sbDocSections)
{
    if (isInternalDef(def) || def->flavor == CapabilityFlavor::Abstract ||
        def->docComment.headerGroup == AutoDocHeaderGroup::Invalid)
        return;

    auto& sbDocSection = sbDocSections[(UInt)def->docComment.headerGroup];
    sbDocSection << "\n"
                 << "`" << def->name << "`\n";
    sbDocSection << def->docComment.comment;
}

List<StringBuilder> setupDocCommentHeaderStringBuilders()
{
    List<StringBuilder> sbDocSections;
    sbDocSections.setCount((UInt)AutoDocHeaderGroup::Count);
    for (UInt i = 0; i < (UInt)AutoDocHeaderGroup::Count; i++)
    {
        sbDocSections[i] << "\n"
                         << getHeaderNameFromAutoDocHeaderGroup(i) << "\n----------------------\n";
        sbDocSections[i] << "*" << getHeaderDescriptionFromAutoDocHeaderGroup(i) << "*\n";
    }
    return sbDocSections;
}

/// "[Link Name](fileName#Link-Name)"
void addHyperLink(StringBuilder& sbDoc, UnownedStringSlice suffix)
{
    String suffixReformatted = "";

    for (auto i : suffix)
    {
        if (i == ' ')
        {
            suffixReformatted.appendChar('-');
            continue;
        }
        suffixReformatted.appendChar(i);
    }
    sbDoc << "[" << suffix << "](#" << suffixReformatted.toLower() << ")";
}

void setupDocumentationHeader(StringBuilder& sbDoc, const String& outPath)
{
    sbDoc << R"(
---
layout: user-guide
---

Capability Atoms
============================

### Sections:

)";

    // Hyper-Links
    for (UInt i = 0; i < (UInt)AutoDocHeaderGroup::Count; i++)
    {
        auto headerName = getHeaderNameFromAutoDocHeaderGroup(i);
        sbDoc << i + 1 << ". "; // "i. "
        addHyperLink(sbDoc, headerName);
        sbDoc << "\n";
    }
}

SlangResult generateDocumentation(
    DiagnosticSink* sink,
    List<RefPtr<CapabilityDef>>& defs,
    StringBuilder& sbDoc,
    const String& outPath)
{
    setupDocumentationHeader(sbDoc, outPath);

    List<StringBuilder> sbDocSections = setupDocCommentHeaderStringBuilders();

    // Group capabilities by header group and sort alphabetically within each group
    List<List<RefPtr<CapabilityDef>>> capabilitiesByHeaderGroup;
    capabilitiesByHeaderGroup.setCount((UInt)AutoDocHeaderGroup::Count);

    // Collect capabilities into their respective header groups
    for (auto def : defs)
    {
        if (!isInternalDef(def) && def->flavor != CapabilityFlavor::Abstract &&
            def->docComment.headerGroup != AutoDocHeaderGroup::Invalid)
        {
            capabilitiesByHeaderGroup[(UInt)def->docComment.headerGroup].add(def);
        }
    }

    // Sort capabilities within each header group alphabetically by name
    for (auto& capabilitiesInGroup : capabilitiesByHeaderGroup)
    {
        capabilitiesInGroup.sort([](const RefPtr<CapabilityDef>& a, const RefPtr<CapabilityDef>& b)
                                 { return a->name < b->name; });
    }

    // Add sorted capabilities to documentation sections
    for (UInt headerGroupIndex = 0; headerGroupIndex < (UInt)AutoDocHeaderGroup::Count;
         headerGroupIndex++)
    {
        for (auto def : capabilitiesByHeaderGroup[headerGroupIndex])
        {
            printDocForCapabilityDef(sbDoc, def, sbDocSections);
        }
    }

    for (auto stringBuilder : sbDocSections)
        sbDoc << stringBuilder.toString();
    return 1;
}
SlangResult generateDefinitions(
    DiagnosticSink* sink,
    List<RefPtr<CapabilityDef>>& defs,
    StringBuilder& sbHeader,
    StringBuilder& sbCpp)
{

    sbHeader << "enum class CapabilityAtom\n{\n";
    sbHeader << "    Invalid,\n";
    for (auto def : defs)
    {
        if (def->flavor == CapabilityFlavor::Normal)
        {
            sbHeader << "    " << def->name << ",\n";
        }
    }
    sbHeader << "    Count\n";
    sbHeader << "};\n";

    CapabilityDef* firstAbstractDef = nullptr;
    CapabilityDef* firstAliasDef = nullptr;
    sbHeader << "enum class CapabilityName\n{\n";
    sbHeader << "    Invalid,\n";
    Index enumValueCounter = 1;
    List<CapabilityDef*> mapEnumValueToDef;
    mapEnumValueToDef.add(nullptr); // For Invalid.
    for (auto def : defs)
    {
        if (def->flavor == CapabilityFlavor::Normal)
        {
            def->enumValue = enumValueCounter;
            ++enumValueCounter;
            mapEnumValueToDef.add(def);
            sbHeader << "    " << def->name << " = (int)CapabilityAtom::" << def->name << ",\n";
        }
    }
    for (auto def : defs)
    {
        if (def->flavor == CapabilityFlavor::Abstract)
        {
            if (firstAbstractDef == nullptr)
                firstAbstractDef = def;
            def->enumValue = enumValueCounter;
            ++enumValueCounter;
            mapEnumValueToDef.add(def);
            sbHeader << "    " << def->name << ",\n";
        }
    }
    for (auto def : defs)
    {
        if (def->flavor == CapabilityFlavor::Alias)
        {
            if (firstAliasDef == nullptr)
                firstAliasDef = def;
            def->enumValue = enumValueCounter;
            ++enumValueCounter;
            mapEnumValueToDef.add(def);
            sbHeader << "    " << def->name << ",\n";
        }
    }
    sbHeader << "    Count\n";
    sbHeader << "};\n";

    Index targetCount = 0;
    Index stageCount = 0;

    UIntSet anyTargetAtomSet{};
    UIntSet anyStageAtomSet{};
    StringBuilder anyTargetUIntSetHash;
    StringBuilder anyStageUIntSetHash;

    for (auto def : defs)
    {
        if (def->getAbstractBase() == def->sharedContext->ptrOfTarget)
        {
            targetCount++;
            anyTargetAtomSet.add(def->enumValue);
        }
        else if (def->getAbstractBase() == def->sharedContext->ptrOfStage)
        {
            stageCount++;
            anyStageAtomSet.add(def->enumValue);
        }
    }
    outputUIntSetGenerator(
        "generatorOf_kAnyTargetUIntSetBuffer",
        anyTargetUIntSetHash,
        anyTargetAtomSet);
    anyTargetUIntSetHash << "static CapabilityAtomSet kAnyTargetUIntSetBuffer = "
                            "generatorOf_kAnyTargetUIntSetBuffer();\n";
    sbCpp << anyTargetUIntSetHash;

    outputUIntSetGenerator(
        "generatorOf_kAnyStageUIntSetBuffer",
        anyStageUIntSetHash,
        anyStageAtomSet);
    anyStageUIntSetHash << "static CapabilityAtomSet kAnyStageUIntSetBuffer = "
                           "generatorOf_kAnyStageUIntSetBuffer();\n";
    sbCpp << anyStageUIntSetHash;

    sbHeader << "\nenum {\n";
    sbHeader << "    kCapabilityTargetCount = " << targetCount << ",\n";
    sbHeader << "    kCapabilityStageCount = " << stageCount << ",\n";
    sbHeader << "};\n\n";

    calcCanonicalRepresentations(sink, defs, mapEnumValueToDef);

    struct SerializedConjunction
    {
        SerializedConjunction() {}
        SerializedConjunction(const String& initFunctionName, UIntSet& data)
            : m_initFunctionName(initFunctionName), m_data(data)
        {
        }
        String m_initFunctionName;
        UIntSet m_data;
    };
    List<SerializedConjunction> serializedCapabilitesCache;

    List<Index> serializedAtomDisjunctions;
    auto serializeConjunction = [&](const List<CapabilityDef*>& capabilities,
                                    CapabilityDef* parentDef,
                                    Index conjunctionNumber) -> Index
    {
        auto capabilitiesAsUIntSet = atomSetToUIntSet(capabilities);
        // Do we already have a serialized capability array that is the same the one we are trying
        // to serialize?
        for (Index i = 0; i < serializedCapabilitesCache.getCount(); i++)
        {
            auto& existingSet = serializedCapabilitesCache[i].m_data;
            if (existingSet == capabilitiesAsUIntSet)
            {
                return i;
            }
        }
        auto initName =
            "generatorOf_" + parentDef->name + "_conjunction" + String(conjunctionNumber);
        outputUIntSetGenerator(initName, sbCpp, capabilitiesAsUIntSet);

        auto result = serializedCapabilitesCache.getCount();
        serializedCapabilitesCache.add(
            SerializedConjunction(initName + "()", capabilitiesAsUIntSet));
        return result;
    };
    auto serializeDisjunction = [&](const List<Index>& conjunctions) -> SerializedArrayView
    {
        SerializedArrayView result;
        result.first = serializedAtomDisjunctions.getCount();
        for (auto c : conjunctions)
        {
            serializedAtomDisjunctions.add(c);
        }
        result.count = conjunctions.getCount();
        return result;
    };
    for (auto def : defs)
    {
        List<Index> conjunctions;
        for (auto& c : def->canonicalRepresentation)
            conjunctions.add(serializeConjunction(c, def, conjunctions.getCount()));
        def->serializedCanonicalRepresentation = serializeDisjunction(conjunctions);
    }

    sbCpp << "static CapabilityAtomSet kCapabilityArray[] = {\n";
    Index arrayIndex = 0;
    for (Index i = 0; i < serializedCapabilitesCache.getCount(); ++i)
    {
        sbCpp << "    " << serializedCapabilitesCache[i].m_initFunctionName << ",\n";
    }
    sbCpp << "};\n";
    sbCpp << "static CapabilityAtomSet* kCapabilityConjunctions[] = {\n";
    for (auto c : serializedAtomDisjunctions)
    {
        sbCpp << "    kCapabilityArray + " << c << ", \n";
    }
    sbCpp << "};\n";

    sbCpp
        << "static const CapabilityAtomInfo kCapabilityNameInfos[int(CapabilityName::Count)] = {\n";
    for (auto* def : mapEnumValueToDef)
    {
        if (!def)
        {
            sbCpp
                << R"(    { UnownedStringSlice::fromLiteral("Invalid"), CapabilityNameFlavor::Concrete, CapabilityName::Invalid, 0, {nullptr, 0} },)"
                << "\n";
            continue;
        }

        // name.
        sbCpp << "    { UnownedStringSlice::fromLiteral(\"" << def->name << "\"), ";

        // flavor.
        switch (def->flavor)
        {
        case CapabilityFlavor::Normal:
            sbCpp << "CapabilityNameFlavor::Concrete";
            break;
        case CapabilityFlavor::Abstract:
            sbCpp << "CapabilityNameFlavor::Abstract";
            break;
        case CapabilityFlavor::Alias:
            sbCpp << "CapabilityNameFlavor::Alias";
            break;
        }
        sbCpp << ", ";

        // abstract base.
        auto abstractBase = def->getAbstractBase();
        if (abstractBase)
        {
            sbCpp << "CapabilityName::" << abstractBase->name;
        }
        else
        {
            sbCpp << "CapabilityName::Invalid";
        }
        sbCpp << ", ";

        // rank
        sbCpp << def->rank;
        sbCpp << ", ";

        // canonnical representation.
        sbCpp << "{ kCapabilityConjunctions + " << def->serializedCanonicalRepresentation.first
              << ", " << def->serializedCanonicalRepresentation.count << "} },\n";
    }

    sbCpp << "};\n";

    sbCpp << "void freeCapabilityDefs()\n"
          << "{\n"
          << "    for (auto& cap : kCapabilityArray) { cap = CapabilityAtomSet(); }\n"
          << "    kAnyTargetUIntSetBuffer = CapabilityAtomSet();\n"
          << "    kAnyStageUIntSetBuffer = CapabilityAtomSet();\n"
          << "}\n";
    return SLANG_OK;
}


SlangResult parseDefFile(
    DiagnosticSink* sink,
    String inputPath,
    List<RefPtr<CapabilityDef>>& outDefs,
    CapabilitySharedContext& capabilitySharedContext)
{
    auto sourceManager = sink->getSourceManager();

    String contents;
    SLANG_RETURN_ON_FAIL(File::readAllText(inputPath, contents));
    PathInfo pathInfo = PathInfo::makeFromString(inputPath);
    SourceFile* sourceFile = sourceManager->createSourceFileWithString(pathInfo, contents);
    SourceView* sourceView = sourceManager->createSourceView(sourceFile, nullptr, SourceLoc());
    Lexer lexer;
    NamePool namePool;
    lexer.initialize(sourceView, sink, &namePool);

    CapabilityDefParser parser(&lexer, sink, capabilitySharedContext);

    SLANG_RETURN_ON_FAIL(parser.parseDefs());
    outDefs = _Move(parser.m_defs);
    return SLANG_OK;
}

void printDiagnostics(DiagnosticSink* sink)
{
    ComPtr<ISlangBlob> blob;
    sink->getBlobIfNeeded(blob.writeRef());
    if (blob)
    {
        fprintf(stderr, "%s", (const char*)blob->getBufferPointer());
    }
}

void writeIfChanged(String fileName, String content)
{
    if (File::exists(fileName))
    {
        String existingContent;
        File::readAllText(fileName, existingContent);
        if (existingContent.getUnownedSlice().trim() == content.getUnownedSlice().trim())
            return;
    }
    File::writeAllText(fileName, content);
}

int main(int argc, const char* const* argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s\n", argc >= 1 ? argv[0] : "slang-capabilities-generator");
        return 1;
    }
    String targetDir, outDocPath;
    for (int i = 0; i < argc - 1; i++)
    {
        if (strcmp(argv[i], "--target-directory") == 0)
            targetDir = argv[i + 1];
        if (strcmp(argv[i], "--doc") == 0)
            outDocPath = argv[i + 1];
    }

    String inPath = argv[1];
    if (targetDir.getLength() == 0)
        targetDir = Path::getParentDirectory(inPath);

    auto outCppPath = Path::combine(targetDir, "slang-generated-capability-defs-impl.h");
    auto outHeaderPath = Path::combine(targetDir, "slang-generated-capability-defs.h");
    auto outLookupPath = Path::combine(targetDir, "slang-lookup-capability-defs.cpp");
    SourceManager sourceManager;
    sourceManager.initialize(nullptr, OSFileSystem::getExtSingleton());
    DiagnosticSink sink(&sourceManager, nullptr);
    List<RefPtr<CapabilityDef>> defs;
    CapabilitySharedContext capabilitySharedContext;
    if (SLANG_FAILED(parseDefFile(&sink, inPath, defs, capabilitySharedContext)))
    {
        printDiagnostics(&sink);
        return 1;
    }

    StringBuilder sbHeader, sbCpp;
    if (SLANG_FAILED(generateDefinitions(&sink, defs, sbHeader, sbCpp)))
    {
        printDiagnostics(&sink);
        return 1;
    }

    if (!File::exists(outDocPath))
    {
        sink.diagnose(
            SourceLoc(),
            Diagnostics::couldNotFindValidDocumentationOutputPath,
            outDocPath);
    }

    StringBuilder sbDoc;
    if (SLANG_FAILED(generateDocumentation(&sink, defs, sbDoc, outDocPath)))
    {
        printDiagnostics(&sink);
        return 1;
    }

    writeIfChanged(outHeaderPath, sbHeader.produceString());
    writeIfChanged(outCppPath, sbCpp.produceString());
    writeIfChanged(outDocPath, sbDoc.produceString());

    List<String> opnames;
    for (auto def : defs)
    {
        opnames.add(def->name);
    }

    if (SLANG_FAILED(writePerfectHashLookupCppFile(
            outLookupPath,
            opnames,
            "CapabilityName",
            "CapabilityName::",
            "slang-capability.h",
            &sink)))
    {
        printDiagnostics(&sink);
        return 1;
    }
    printDiagnostics(&sink);
    // generateDefinitions can diagnose an error yet still return SLANG_OK, so consult the sink.
    return sink.getErrorCount() ? 1 : 0;
}
