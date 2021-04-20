#include "unit-test.h"


#include "../../source/compiler-core/slang-name-convention-util.h"

#include "../../source/core/slang-io.h"

#include "../../source/compiler-core/slang-source-loc.h"
#include "../../source/compiler-core/slang-lexer.h"

#include "identifier-lookup.h"
#include "node-tree.h"
#include "parser.h"
#include "options.h"

namespace CppExtract {
using namespace Slang;

static const char enumSource[] =
"enum SomeEnum\n"
"{\n"
"    Value,\n"
"    Another = 10,\n"
"};\n";

struct TestState
{
    TestState():
        m_slicePool(StringSlicePool::Style::Default)
    {
        m_identifierLookup.initDefault(UnownedStringSlice::fromLiteral("SLANG_"));

        m_sourceManager.initialize(nullptr, nullptr);

        m_sink.init(&m_sourceManager, Lexer::sourceLocationLexer);

        m_namePool.setRootNamePool(&m_rootNamePool);
    }

    RootNamePool m_rootNamePool;
    Options m_options;
    SourceManager m_sourceManager;
    DiagnosticSink m_sink;
    NamePool m_namePool;
    StringSlicePool m_slicePool;
    IdentifierLookup m_identifierLookup;
};

/* static */SlangResult UnitTestUtil::run()
{
    {
        TestState state;

        NodeTree tree(&state.m_slicePool, &state.m_namePool, &state.m_identifierLookup);
        
        UnownedStringSlice contents = UnownedStringSlice::fromLiteral(enumSource);
        PathInfo pathInfo = PathInfo::makeFromString("enum.h");

        SourceManager* sourceManager = &state.m_sourceManager;

        SourceFile* sourceFile = sourceManager->createSourceFileWithString(pathInfo, contents);
        SourceOrigin* sourceOrigin = tree.addSourceOrigin(sourceFile, state.m_options);

        Parser parser(&tree, &state.m_sink);
        SLANG_RETURN_ON_FAIL(parser.parse(sourceOrigin, &state.m_options));
    }

    return SLANG_OK;
}

} // namespace CppExtract
