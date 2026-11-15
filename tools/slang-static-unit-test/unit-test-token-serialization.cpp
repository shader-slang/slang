// This test serializes actual semantic tokens retained in a checked AST. We compile two
// fields with identical HLSL semantics, then reload the module into a new session. The
// token spelling, trivia, and interning must survive independently of the original AST;
// diagnostic locations are checked both with and without serialized location data.

#include "core/slang-stream.h"
#include "slang-com-ptr.h"
#include "slang/slang-module.h"
#include "slang/slang-serialize-container.h"
#include "slang/slang-session.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

/// Returns the semantic token on the sole field of a named exported struct.
///
/// Requires the named declaration to be a struct with one field carrying a simple
/// HLSL semantic. Failed setup assumptions abort the test before token inspection.
Token getFieldSemanticToken(Module* module, const char* structName)
{
    // Export lookup works for both parsed modules and lazily deserialized modules,
    // so the test does not depend on whether a `FileDecl` encloses a declaration.
    StructDecl* structure = nullptr;
    const auto count = module->getExportedDeclCount();
    for (Index i = 0; i < count; ++i)
    {
        auto declaration = as<StructDecl>(module->getExportedDecl(i));
        if (!declaration)
            continue;
        if (declaration->getName()->text != structName)
            continue;
        structure = declaration;
        break;
    }
    SLANG_CHECK_ABORT(structure);

    auto field = structure->findFirstDirectMemberDeclOfType<VarDecl>();
    SLANG_CHECK_ABORT(field);
    auto semantic = field->findModifier<HLSLSimpleSemantic>();
    SLANG_CHECK_ABORT(semantic);
    return semantic->name;
}

/// Checks the spelling and context shared by both semantic-token occurrences.
void checkSemanticToken(Token token)
{
    // The semantic has one logical identifier, but its physical spelling crosses a
    // newline. The cached name must use the logical spelling after deserialization.
    SLANG_CHECK(token.is<IdentifierToken>());
    SLANG_CHECK(token.getPhysicalSpelling() == UnownedStringSlice("COLO\\\nR"));
    SLANG_CHECK(token.getLogicalSpelling() == UnownedStringSlice("COLOR"));
    SLANG_CHECK_ABORT(token.getName());
    SLANG_CHECK(token.getName()->text == "COLOR");
    SLANG_CHECK((token.getFlags() & TokenFlag::AtStartOfLine) != 0);
    SLANG_CHECK((token.getFlags() & TokenFlag::AfterWhitespace) != 0);

    // The preceding colon owns its first newline. The comment on the following
    // line therefore leads the semantic; its trailing comment precedes the semicolon.
    SLANG_CHECK_ABORT(token.getLeadingTrivia().getCount() == 2);
    SLANG_CHECK(token.getLeadingTrivia()[0].is<LineCommentTrivia>());
    SLANG_CHECK(token.getLeadingTrivia()[0].getPhysicalSpelling() == UnownedStringSlice("// lead"));
    SLANG_CHECK(token.getLeadingTrivia()[1].is<NewLineTrivia>());
    SLANG_CHECK(token.getLeadingTrivia()[1].getPhysicalSpelling() == UnownedStringSlice("\n"));
    SLANG_CHECK_ABORT(token.getTrailingTrivia().getCount() == 2);
    SLANG_CHECK(token.getTrailingTrivia()[0].getPhysicalSpelling() == UnownedStringSlice(" "));
    SLANG_CHECK(token.getTrailingTrivia()[1].is<BlockCommentTrivia>());
    SLANG_CHECK(
        token.getTrailingTrivia()[1].getPhysicalSpelling() == UnownedStringSlice("/*tail*/"));
}

/// Checks that a retained location resolves to the expected physical line and column.
void checkHumaneLocation(SourceManager* manager, SourceLoc loc, Int line, Int column)
{
    SLANG_CHECK(loc.isValid());
    const auto humane = manager->getHumaneLoc(loc, SourceLocType::Actual);
    SLANG_CHECK(humane.line == line);
    SLANG_CHECK(humane.column == column);
}

} // namespace

SLANG_UNIT_TEST(tokenASTSerializationPreservesLexicalValues)
{
    // The two fields below use the semantic COLOR, with an escaped newline
    // between its final two letters. The parser retains that identifier token
    // in `HLSLSimpleSemantic::name`, a serialized AST field. Both occurrences
    // must share their loaded `TokenObj` despite their different locations.
    const char* source = "module token_serialization;\n"
                         "public struct First\n"
                         "{\n"
                         "float4 value :\n"
                         "// lead\n"
                         "COLO\\\n"
                         "R /*tail*/;\n"
                         "};\n"
                         "public struct Second\n"
                         "{\n"
                         "float4 value :\n"
                         "// lead\n"
                         "COLO\\\n"
                         "R /*tail*/;\n"
                         "};\n";
    // The static harness shares its initialized global session across tests.
    // Each producing and receiving session below still has independent module state.
    auto globalSession = unitTestContext->slangGlobalSession;
    SLANG_CHECK_ABORT(globalSession);

    for (bool includeLocations : {false, true})
    {
        // We destroy the producing session before loading its bytes. Only the
        // serialized stream crosses that lifetime boundary.
        OwnedMemoryStream serialized(FileAccess::Write);
        TokenFlags expectedFlags = 0;
        {
            slang::SessionDesc desc = {};
            ComPtr<slang::ISession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(globalSession->createSession(desc, session.writeRef())));
            ComPtr<ISlangBlob> diagnostics;
            auto publicModule = session->loadModuleFromSourceString(
                "token_serialization",
                "token_serialization.slang",
                source,
                diagnostics.writeRef());
            SLANG_CHECK_ABORT(publicModule);
            auto module = static_cast<Module*>(publicModule);
            SLANG_AST_BUILDER_RAII(module->getASTBuilder());

            auto first = getFieldSemanticToken(module, "First");
            auto second = getFieldSemanticToken(module, "Second");
            checkSemanticToken(first);
            SLANG_CHECK(first.getPool() == module->getASTBuilder()->getTokenPool());
            SLANG_CHECK(second.getPool() == first.getPool());
            SLANG_CHECK(first.getObject() == second.getObject());
            SLANG_CHECK(first.getLoc() != second.getLoc());
            expectedFlags = first.getFlags();

            // The container API makes omission of location data explicit. The
            // token serializer must preserve lexical state under either policy.
            SerialContainerUtil::WriteOptions options;
            if (includeLocations)
                options.sourceManagerToUseWhenSerializingSourceLocs =
                    module->getLinkage()->getSourceManager();
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(SerialContainerUtil::write(module, options, &serialized)));
        }

        TokenRef<IdentifierToken> retainedToken;
        {
            slang::SessionDesc desc = {};
            ComPtr<slang::ISession> session;
            SLANG_CHECK_ABORT(
                SLANG_SUCCEEDED(globalSession->createSession(desc, session.writeRef())));
            ComPtr<ISlangBlob> diagnostics;
            const auto data = serialized.getContents();
            auto publicModule = slang_loadModuleFromIRBlob(
                session,
                "token_serialization",
                "token_serialization.slang-module",
                data.getBuffer(),
                data.getCount(),
                diagnostics.writeRef());
            SLANG_CHECK_ABORT(publicModule);
            auto module = static_cast<Module*>(publicModule);
            SLANG_AST_BUILDER_RAII(module->getASTBuilder());

            auto first = getFieldSemanticToken(module, "First");
            auto second = getFieldSemanticToken(module, "Second");
            checkSemanticToken(first);
            SLANG_CHECK(first.getPool() == module->getASTBuilder()->getTokenPool());
            SLANG_CHECK(second.getPool() == first.getPool());
            SLANG_CHECK(first.getObject() == second.getObject());
            SLANG_CHECK(first.getFlags() == expectedFlags);
            retainedToken = TokenRef(first.as<IdentifierToken>());

            if (includeLocations)
            {
                // Registering only the identifier's location would lose the
                // leading comment's line metadata and the line after the splice.
                auto manager = module->getLinkage()->getSourceManager();
                checkHumaneLocation(manager, first.getLoc(), 6, 1);
                checkHumaneLocation(manager, first.getLeadingTrivia()[0].getLoc(), 5, 1);
                checkHumaneLocation(manager, first.getLeadingTrivia()[1].getLoc(), 5, 8);
                checkHumaneLocation(manager, first.getTrailingTrivia()[0].getLoc(), 7, 2);
                checkHumaneLocation(manager, first.getTrailingTrivia()[1].getLoc(), 7, 3);
                checkHumaneLocation(manager, second.getLoc(), 13, 1);
            }
            else
            {
                // Unknown locations must remain unknown through both indexed and
                // sequential trivia access, rather than acquiring byte offsets.
                SLANG_CHECK(!first.getLoc().isValid());
                SLANG_CHECK(!first.getLeadingTrivia()[0].getLoc().isValid());
                SLANG_CHECK(!first.getTrailingTrivia()[1].getLoc().isValid());
                for (auto trivia : first.getLeadingTrivia())
                    SLANG_CHECK(!trivia.getLoc().isValid());
                for (auto trivia : first.getTrailingTrivia())
                    SLANG_CHECK(!trivia.getLoc().isValid());
            }
        }

        // Retaining the pool keeps spelling, trivia, and the cached name usable
        // after the receiving AST and source manager have been released.
        checkSemanticToken(retainedToken.get());
    }
}
