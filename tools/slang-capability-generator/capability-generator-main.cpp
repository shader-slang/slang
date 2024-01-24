// capabilities-generator-main.cpp

#include <stdio.h>
#include "../../source/compiler-core/slang-lexer.h"
#include "../../source/compiler-core/slang-perfect-hash-codegen.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-secure-crt.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-file-system.h"

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

struct CapabilityDef : public RefObject
{
    Index enumValue;
    String name;
    CapabilityDisjunctionExpr expr;
    CapabilityFlavor flavor;
    int rank;
    List<List<CapabilityDef*>> canonicalRepresentation;
    SerializedArrayView serializedCanonicalRepresentation;

    CapabilityDef* getAbstractBase()
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
};

struct CapabilityDefParser
{
    CapabilityDefParser(Lexer* lexer, DiagnosticSink* sink)
        : m_lexer(lexer)
        , m_sink(sink)
    {
    }
    
    Lexer* m_lexer;
    DiagnosticSink* m_sink;

    Dictionary<String, CapabilityDef*> m_mapNameToCapability;
    List<RefPtr<CapabilityDef>> m_defs;

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
            if (!(advanceIf(TokenType::OpAnd) || advanceIf(TokenType::OpAdd)))
                break;
        }
        return SLANG_OK;
    }

    SlangResult parseExpr(CapabilityDisjunctionExpr& expr)
    {
        for (;;)
        {
            CapabilityConjunctionExpr conjunction;
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
            if (!m_mapNameToCapability.addIfNotExists(def->name, def))
            {
                m_sink->diagnose(nextToken.loc, Diagnostics::redefinition, def->name);
                return SLANG_FAIL;
            }
        }
        return SLANG_OK;
    }
};

struct CapabilityConjunction
{
    HashSet<CapabilityDef*> atoms;
    bool implies(const CapabilityConjunction& c) const
    {
        for (auto& atom : c.atoms)
        {
            if (!atoms.contains(atom))
                return false;
        }
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

    void addConjunction(const CapabilityConjunction& c)
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
            if (conjunctions[i].implies(c))
            {
                conjunctions.fastRemoveAt(i);
            }
            else
            {
                i++;
            }
        }
        conjunctions.add(_Move(c));
    }

    CapabilityDisjunction joinWith(const CapabilityDisjunction& other)
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
                result.addConjunction(_Move(newC));
            }
        }
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

CapabilityDisjunction evaluateConjunction(const List<CapabilityDef*>& atoms)
{
    CapabilityDisjunction result;
    for (auto& def : atoms)
    {
        CapabilityDisjunction defCanonical = getCanonicalRepresentation(def);
        result = result.joinWith(defCanonical);
    }
    return result;
}

void calcCanonicalRepresentation(CapabilityDef* def, const List<CapabilityDef*>& mapEnumValueToDef)
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
        CapabilityDisjunction evalD = evaluateConjunction(c.atoms);
        for (auto& cc : evalD.conjunctions)
            exprVal.addConjunction(cc);
    }
    disjunction = disjunction.joinWith(exprVal);
    def->canonicalRepresentation = disjunction.canonicalize();
}

void calcCanonicalRepresentations(const List<RefPtr<CapabilityDef>>& defs, const List<CapabilityDef*>& mapEnumValueToDef)
{
    for (auto def : defs)
        calcCanonicalRepresentation(def, mapEnumValueToDef);
}

SlangResult generateDefinitions(const List<RefPtr<CapabilityDef>>& defs, StringBuilder& sbHeader, StringBuilder& sbCpp)
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

    calcCanonicalRepresentations(defs, mapEnumValueToDef);

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
    for (auto& def : defs)
    {
        List<SerializedArrayView> conjunctions;
        for (auto& c : def->canonicalRepresentation)
            conjunctions.add(serializeConjunction(c));
        def->serializedCanonicalRepresentation = serializeDisjunction(conjunctions);
    }

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
    for (auto def : mapEnumValueToDef)
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


SlangResult parseDefFile(DiagnosticSink* sink, String inputPath, List<RefPtr<CapabilityDef>>& outDefs)
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
   
    CapabilityDefParser parser(&lexer, sink);

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
    if (SLANG_FAILED(parseDefFile(&sink, inPath, defs)))
    {
        printDiagnostics(&sink);
        return 1;
    }

    StringBuilder sbHeader, sbCpp;
    if (SLANG_FAILED(generateDefinitions(defs, sbHeader, sbCpp)))
    {
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
    return 0;
}
