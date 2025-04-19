#include "compiler-core/slang-diagnostic-sink.h"
#include "compiler-core/slang-lexer.h"
#include "compiler-core/slang-name.h"
#include "compiler-core/slang-source-loc.h"
#include "core/slang-file-system.h"
#include "core/slang-io.h"
#include "core/slang-list.h"
#include "core/slang-string-slice-pool.h"
#include "core/slang-string-util.h"
#include "core/slang-string.h"
#include "core/slang-writer.h"
#include "slang-com-helper.h"
#include "slang-cpp-parser/diagnostics.h"
#include "slang-cpp-parser/file-util.h"
#include "slang-cpp-parser/node-tree.h"
#include "slang-cpp-parser/node.h"
#include "slang-cpp-parser/options.h"
#include "slang-cpp-parser/parser.h"
#include "slang-cpp-parser/unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace BindingGenerator
{

using namespace Slang;
using namespace CppParse;

class App
{
public:
    SlangResult execute(const Options& options);

    SlangResult executeWithArgs(int argc, const char* const* argv);

    const Options& getOptions() const { return m_options; }

    App(DiagnosticSink* sink, SourceManager* sourceManager, RootNamePool* rootNamePool)
        : m_sink(sink), m_sourceManager(sourceManager), m_slicePool(StringSlicePool::Style::Default)
    {
        m_namePool.setRootNamePool(rootNamePool);
    }

protected:
    NamePool m_namePool;

    Options m_options;
    DiagnosticSink* m_sink;
    SourceManager* m_sourceManager;

    StringSlicePool m_slicePool;
};

SlangResult App::execute(const Options& options)
{
    m_options = options;

    if (options.m_runUnitTests)
    {
        SLANG_RETURN_ON_FAIL(UnitTestUtil::run());
    }

    IdentifierLookup identifierLookup;
    identifierLookup.initDefault(options.m_markPrefix.getUnownedSlice());

    NodeTree tree(&m_slicePool, &m_namePool, &identifierLookup);

    // Read in each of the input files
    for (Index i = 0; i < m_options.m_inputPaths.getCount(); ++i)
    {
        String inputPath;

        if (m_options.m_inputDirectory.getLength())
        {
            inputPath = Path::combine(m_options.m_inputDirectory, m_options.m_inputPaths[i]);
        }
        else
        {
            inputPath = m_options.m_inputPaths[i];
        }

        // Read the input file
        String contents;
        SLANG_RETURN_ON_FAIL(FileUtil::readAllText(inputPath, m_sink, contents));

        PathInfo pathInfo = PathInfo::makeFromString(inputPath);

        SourceFile* sourceFile = m_sourceManager->createSourceFileWithString(pathInfo, contents);

        SourceOrigin* sourceOrigin = tree.addSourceOrigin(sourceFile, options);

        Parser parser(&tree, m_sink);
        SLANG_RETURN_ON_FAIL(parser.parse(sourceOrigin, &m_options));
    }

    SLANG_RETURN_ON_FAIL(tree.calcDerivedTypes(m_sink));

    // Dump out the tree
    if (options.m_dump)
    {
        {
            StringBuilder buf;
            tree.getRootNode()->dump(0, buf);
            m_sink->writer->write(buf.getBuffer(), buf.getLength());
        }
    }

    return SLANG_OK;
}

SlangResult App::executeWithArgs(int argc, const char* const* argv)
{
    Options options;
    OptionsParser optionsParser;
    SLANG_RETURN_ON_FAIL(optionsParser.parse(argc, argv, m_sink, options));
    SLANG_RETURN_ON_FAIL(execute(options));
    return SLANG_OK;
}

} // namespace BindingGenerator

int main(int argc, const char* const* argv)
{
    using namespace Slang;
    using namespace BindingGenerator;

    ComPtr<ISlangWriter> writer(new FileWriter(stderr, WriterFlag::AutoFlush));

    RootNamePool rootNamePool;

    SourceManager sourceManager;
    sourceManager.initialize(nullptr, nullptr);

    DiagnosticSink sink(&sourceManager, Lexer::sourceLocationLexer);
    sink.writer = writer;

    App app(&sink, &sourceManager, &rootNamePool);

    try
    {
        if (SLANG_FAILED(app.executeWithArgs(argc - 1, argv + 1)))
        {
            sink.diagnose(SourceLoc(), CPPDiagnostics::extractorFailed);
            return 1;
        }
        if (sink.getErrorCount())
        {
            sink.diagnose(SourceLoc(), CPPDiagnostics::extractorFailed);
            return 1;
        }
    }
    catch (...)
    {
        sink.diagnose(SourceLoc(), CPPDiagnostics::internalError);
        return 1;
    }

    return 0;
}
