// capabilities-generator-main.cpp

#include <stdio.h>
#include "../../source/compiler-core/slang-lexer.h"
#include "../../source/compiler-core/slang-perfect-hash-codegen.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-secure-crt.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-file-system.h"
#include "../../source/core/slang-uint-set.h"

using namespace Slang;

namespace Diagnostics
{
#define DIAGNOSTIC(id, severity, name, messageFormat) const DiagnosticInfo name = { id, Severity::severity, #name, messageFormat };
#include "slang-capability-diagnostic-defs.h"
#undef DIAGNOSTIC
}

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
    SourceLoc            sourceLoc;
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

static void _removeFromOtherAtomsNotInThis(HashSet<const CapabilityDef*> thisSet, HashSet<const CapabilityDef*> otherSet, List<const CapabilityDef*> atomsToRemove)
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
    /// Stores key atoms a CapabilityDef refers to. 
    /// Shared key atoms: key atoms shared between every individual set in a canonicalRepresentation, added together.
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
                        if(!alreadySetTarget)
                            sharedKeyAtomsInCanonicalSet_stage.add(otherkeyAtomsPresent);
                    }
                    // all key atoms associated with atom
                    keyAtomsFound.add(otherkeyAtomsPresent);
                }

                // remove all not shared key atoms 
                if (foundTarget)
                {
                    alreadySetTarget = true;
                    _removeFromOtherAtomsNotInThis(keyAtomsFound, sharedKeyAtomsInCanonicalSet_target, atomsToRemove);
                }
                if (foundStage)
                {
                    alreadySetStage = true;
                    _removeFromOtherAtomsNotInThis(keyAtomsFound, sharedKeyAtomsInCanonicalSet_stage, atomsToRemove);
                }
                keyAtomsFound.clear();
            }
            
            // add all shared key atoms
            for (auto keyAtom : sharedKeyAtomsInCanonicalSet_target)
                this->keyAtomsPresent.add(keyAtom);
            for (auto keyAtom : sharedKeyAtomsInCanonicalSet_stage)
                this->keyAtomsPresent.add(keyAtom);
        }
        if (auto base = this->getAbstractBase())
            keyAtomsPresent.add(this);
    }
};

struct CapabilityDefParser
{
    CapabilityDefParser(
        Lexer* lexer, 
        DiagnosticSink* sink,
        CapabilitySharedContext& sharedContext)
        : m_lexer(lexer)
        , m_sink(sink)
        , m_sharedContext(sharedContext)
    {
    }
    
    Lexer* m_lexer;
    DiagnosticSink* m_sink;

    Dictionary<String, CapabilityDef*> m_mapNameToCapability;
    List<RefPtr<CapabilityDef>> m_defs;
    CapabilitySharedContext& m_sharedContext;

    TokenReader m_tokenReader;

    bool advanceIf(TokenType type)
    {
        if (m_tokenReader.peekTokenType() == type)
        {
            m_tokenReader.advanceToken();
            return true;
        }
        return false;
    }

    SlangResult readToken(TokenType type, Token& nextToken)
    {
        nextToken = m_tokenReader.advanceToken();
        if (nextToken.type != type)
        {
            m_sink->diagnose(nextToken.loc, Diagnostics::unexpectedTokenExpectedTokenType, nextToken, type);
            return SLANG_FAIL;
        }
        return SLANG_OK;
    }

    SlangResult readToken(TokenType type)
    {
        Token nextToken;
        return readToken(type, nextToken);
    }

    SlangResult parseConjunction(CapabilityConjunctionExpr& expr)
    {
        for (;;)
        {
            Token nameToken;
            SLANG_RETURN_ON_FAIL(readToken(TokenType::Identifier, nameToken));
            CapabilityDef* def = nullptr;
            if (m_mapNameToCapability.tryGetValue(nameToken.getContent(), def))
            {
                expr.atoms.add(def);
            }
            else
            {
                m_sink->diagnose(nameToken.loc, Diagnostics::undefinedIdentifier, nameToken);
                return SLANG_FAIL;
            }
            if (!(advanceIf(TokenType::OpAdd)))
                break;
        }
        return SLANG_OK;
    }

    SlangResult parseExpr(CapabilityDisjunctionExpr& expr)
    {
        for (;;)
        {
            CapabilityConjunctionExpr conjunction;
            conjunction.sourceLoc = this->m_tokenReader.m_cursor->getLoc();
            SLANG_RETURN_ON_FAIL(parseConjunction(conjunction));
            expr.conjunctions.add(conjunction);
            if (!advanceIf(TokenType::OpBitOr))
                break;
        }
        return SLANG_OK;
    }

    SlangResult parseDefs()
    {
        auto tokens = m_lexer->lexAllSemanticTokens();
        m_tokenReader = TokenReader(tokens);
        for (;;)
        {
            RefPtr<CapabilityDef> def = new CapabilityDef();
            def->sharedContext = &m_sharedContext;
            def->flavor = CapabilityFlavor::Normal;
            auto nextToken = m_tokenReader.advanceToken();
            if (nextToken.getContent() == "alias")
            {
                def->flavor = CapabilityFlavor::Alias;
            }
            else if (nextToken.getContent() == "abstract")
            {
                def->flavor = CapabilityFlavor::Abstract;
            }
            else if (nextToken.getContent() == "def")
            {
                def->flavor = CapabilityFlavor::Normal;
            }
            else if (nextToken.type == TokenType::EndOfFile)
            {
                break;
            }
            else
            {
                m_sink->diagnose(nextToken.loc, Diagnostics::unexpectedToken, nextToken);
                return SLANG_FAIL;
            }

            Token nameToken;
            SLANG_RETURN_ON_FAIL(readToken(TokenType::Identifier, nameToken));
            def->name = nameToken.getContent();

            if (def->flavor == CapabilityFlavor::Normal)
            {
                if (advanceIf(TokenType::Colon))
                {
                    SLANG_RETURN_ON_FAIL(parseExpr(def->expr));
                }
                if (advanceIf(TokenType::OpAssign))
                {
                    Token rankToken;
                    SLANG_RETURN_ON_FAIL(readToken(TokenType::IntegerLiteral, rankToken));
                    def->rank = stringToInt(rankToken.getContent());
                }
            }
            else if (def->flavor == CapabilityFlavor::Alias)
            {
                SLANG_RETURN_ON_FAIL(readToken(TokenType::OpAssign));
                SLANG_RETURN_ON_FAIL(parseExpr(def->expr));
            }
            else if (def->flavor == CapabilityFlavor::Abstract)
            {
                if (advanceIf(TokenType::Colon))
                {
                    SLANG_RETURN_ON_FAIL(parseExpr(def->expr));
                }
            }
            SLANG_RETURN_ON_FAIL(readToken(TokenType::Semicolon));
            m_defs.add(def);
            if (!m_mapNameToCapability.addIfNotExists(def->name, m_defs.getLast()))
            {
                m_sink->diagnose(nextToken.loc, Diagnostics::redefinition, def->name);
                return SLANG_FAIL;
            }

            //set abstract atom identifiers
            if (!m_sharedContext.ptrOfTarget
                && def->name.equals("target"))
                m_sharedContext.ptrOfTarget = m_defs.getLast();
            else if (!m_sharedContext.ptrOfStage
                && def->name.equals("stage"))
                m_sharedContext.ptrOfStage = m_defs.getLast();

            def->sourceLoc = nameToken.loc;
        }
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

    bool shareTargetAndStageAtom(const CapabilityConjunction& other, CapabilitySharedContext& context)
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
        // Keep a map from an abstract base to the concrete atom defined in this conjunction that implements the base.
        Dictionary<CapabilityDef*, CapabilityDef*> abstractKV;

        for (auto& atom : atoms)
        {
            auto abstractBase = atom->getAbstractBase();
            if (!abstractBase)
                continue;

            // Have we already seen another concrete atom that implements the same abstract base of the current atom?
            // If so, we have a conflict and the conjunction is impossible.
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

    void addConjunction(DiagnosticSink* sink, SourceLoc sourceLoc, CapabilitySharedContext& context, CapabilityConjunction& c)
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
                        sink->diagnose(sourceLoc, Diagnostics::unionWithSameKeyAtomButNotSubset, conjunctions[i].toString(), c.toString());
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

                if(i < ii)
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

    void inclusiveJoinConjunction(CapabilitySharedContext& context, CapabilityConjunction& c, List<CapabilityConjunction>& toAddAfter)
    {
        if (c.isImpossible())
            return;
        for (auto& conjunction : conjunctions)
        {
            if (c.implies(conjunction))
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

    CapabilityDisjunction joinWith(DiagnosticSink* sink, SourceLoc sourceLoc, CapabilitySharedContext& context, const CapabilityDisjunction& other)
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
            atoms.sort([](CapabilityDef* c1, CapabilityDef* c2) {return c1->enumValue < c2->enumValue; });
            result.add(_Move(atoms));
        }
        result.sort([](const List<CapabilityDef*>& c1, const List<CapabilityDef*>& c2)
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

CapabilityDisjunction evaluateConjunction(DiagnosticSink* sink, SourceLoc sourceLoc, CapabilitySharedContext& context, const List<CapabilityDef*>& atoms)
{
    CapabilityDisjunction result;
    for (auto* def : atoms)
    {
        CapabilityDisjunction defCanonical = getCanonicalRepresentation(def);
        result = result.joinWith(sink, sourceLoc, context, defCanonical);
    }
    return result;
}

void calcCanonicalRepresentation(DiagnosticSink* sink, CapabilityDef* def, const List<CapabilityDef*>& mapEnumValueToDef)
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
        CapabilityDisjunction evalD = evaluateConjunction(sink, c.sourceLoc, *def->sharedContext, c.atoms);
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

void calcCanonicalRepresentations(DiagnosticSink* sink, List<RefPtr<CapabilityDef>>& defs, const List<CapabilityDef*>& mapEnumValueToDef)
{
    for (auto def : defs)
        calcCanonicalRepresentation(sink, def, mapEnumValueToDef);
}

void outputUIntSetAsBufferValues(const String& nameOfBuffer, StringBuilder& resultBuilder, UIntSet& set)
{
    // store UIntSet::Element as uint8_t to stay sizeof(UIntSet::Element) independent.
    // underlying type may change, bits stay the same.
    resultBuilder << "inline static CapabilityAtomSet generate_" << nameOfBuffer << "()\n";
    resultBuilder << "{\n";
    resultBuilder << "    CapabilityAtomSet generatedSet;\n";

    for (Index i = 0; i < set.getBuffer().getCount(); i++)
    {
        resultBuilder << "    generatedSet.addRawElement(UIntSet::Element(" << set.getBuffer()[i] << "UL), " << i << ");\n";
    }
    resultBuilder << "    return generatedSet;\n";
    resultBuilder << "}\n";

    resultBuilder << "const static CapabilityAtomSet " << nameOfBuffer << " = generate_" << nameOfBuffer << "();\n";
}

SlangResult generateDefinitions(DiagnosticSink* sink, List<RefPtr<CapabilityDef>>& defs, StringBuilder& sbHeader, StringBuilder& sbCpp)
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
    outputUIntSetAsBufferValues("kAnyTargetUIntSetBuffer", anyTargetUIntSetHash, anyTargetAtomSet);
    outputUIntSetAsBufferValues("kAnyStageUIntSetBuffer", anyStageUIntSetHash, anyStageAtomSet);
    
    sbHeader << "\nenum {\n";
    sbHeader << "    kCapabilityTargetCount = " << targetCount << ",\n";
    sbHeader << "    kCapabilityStageCount = " << stageCount << ",\n";
    sbHeader << "};\n\n";

    calcCanonicalRepresentations(sink, defs, mapEnumValueToDef);
    List<String> capabiltiyNameArray;
    List<SerializedArrayView> serializedCapabilityArrays;

    List<SerializedArrayView> serializedAtomDisjunctions;
    auto serializeConjunction = [&](const List<CapabilityDef*>& capabilities) -> SerializedArrayView
        {
            // Do we already have a serialized capability array that is the same the one we are trying to serialize?
            for (auto existingArray : serializedCapabilityArrays)
            {
                if (existingArray.count == capabilities.getCount())
                {
                    bool match = true;
                    for (Index i = 0; i < capabilities.getCount(); i++)
                    {
                        if (capabiltiyNameArray[existingArray.first+i] != capabilities[i]->name)
                        {
                            match = false;
                            break;
                        }
                    }
                    if (match)
                        return existingArray;
                }
            }
            SerializedArrayView result;
            result.first = capabiltiyNameArray.getCount();
            for (auto capability : capabilities)
            {
                capabiltiyNameArray.add(capability->name);
            }
            result.count = capabilities.getCount();
            serializedCapabilityArrays.add(result);
            return result;
        };
    auto serializeDisjunction = [&](const List<SerializedArrayView>& conjunctions) -> SerializedArrayView
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
        List<SerializedArrayView> conjunctions;
        for (auto& c : def->canonicalRepresentation)
            conjunctions.add(serializeConjunction(c));
        def->serializedCanonicalRepresentation = serializeDisjunction(conjunctions);
    }
    
    sbCpp << anyTargetUIntSetHash;
    sbCpp << anyStageUIntSetHash;

    sbCpp << "static CapabilityName kCapabilityArray[] = {\n";
    Index arrayIndex = 0;
    sbCpp << "    /* [0] @0: */ ";
    for (Index i = 0; i < capabiltiyNameArray.getCount(); ++i)
    {
        sbCpp << " CapabilityName::" << capabiltiyNameArray[i] << ",";
        if (i + 1 == serializedCapabilityArrays[arrayIndex].first + serializedCapabilityArrays[arrayIndex].count)
        {
            arrayIndex++;
            if (arrayIndex == serializedCapabilityArrays.getCount())
                sbCpp << "\n";
            else
                sbCpp << "\n    /* [" << arrayIndex << "] @" << serializedCapabilityArrays[arrayIndex].first <<": */ ";
        }
    }
    sbCpp << "};\n";
    sbCpp << "static ArrayView<CapabilityName> kCapabilityConjunctions[] = {\n";
    for (auto c : serializedAtomDisjunctions)
    {
        sbCpp << "    { kCapabilityArray + " << c.first << ", " << c.count << " },\n";
    }
    sbCpp << "};\n";

    sbCpp << "static const CapabilityAtomInfo kCapabilityNameInfos[int(CapabilityName::Count)] = {\n";
    for (auto* def : mapEnumValueToDef)
    {
        if (!def)
        {
            sbCpp << R"(    { "Invalid", CapabilityNameFlavor::Concrete, CapabilityName::Invalid, 0, {nullptr, 0} },)" << "\n";
            continue;
        }

        // name.
        sbCpp << "    { \"" << def->name << "\", ";

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

        // canonnical representation.
        sbCpp << def->rank << ", { kCapabilityConjunctions + " << def->serializedCanonicalRepresentation.first << ", " << def->serializedCanonicalRepresentation.count << "} },\n";
    }
    
    sbCpp << "};\n";
    return SLANG_OK;
}


SlangResult parseDefFile(DiagnosticSink* sink, String inputPath, List<RefPtr<CapabilityDef>>& outDefs, CapabilitySharedContext& capabilitySharedContext)
{
    auto sourceManager = sink->getSourceManager();

    String contents;
    SLANG_RETURN_ON_FAIL(File::readAllText(inputPath, contents));
    PathInfo    pathInfo = PathInfo::makeFromString(inputPath);
    SourceFile* sourceFile = sourceManager->createSourceFileWithString(pathInfo, contents);
    SourceView* sourceView = sourceManager->createSourceView(sourceFile, nullptr, SourceLoc());
    Lexer   lexer;
    NamePool namePool;
    RootNamePool rootPool;
    namePool.setRootNamePool(&rootPool);
    lexer.initialize(sourceView, sink, &namePool, sourceManager->getMemoryArena());
   
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
        fprintf(
            stderr,
            "Usage: %s\n",
            argc >= 1 ? argv[0] : "slang-capabilities-generator");
        return 1;
    }
    String targetDir;
    for (int i = 0; i < argc - 1; i++)
    {
        if (strcmp(argv[i], "--target-directory") == 0)
            targetDir = argv[i + 1];
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

    writeIfChanged(outHeaderPath, sbHeader.produceString());
    writeIfChanged(outCppPath, sbCpp.produceString());

    List<String> opnames;
    for (auto def : defs)
    {
        opnames.add(def->name);
    }

    if (SLANG_FAILED(writePerfectHashLookupCppFile(outLookupPath, opnames, "CapabilityName", "CapabilityName::", "slang-capability.h", &sink)))
    {
        printDiagnostics(&sink);
        return 1;
    }
    printDiagnostics(&sink);
    return 0;
}
