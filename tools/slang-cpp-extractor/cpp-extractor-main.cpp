// main.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../source/core/slang-secure-crt.h"

#include "../../slang-com-helper.h"

#include "../../source/core/slang-list.h"
#include "../../source/core/slang-string.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-string-slice-pool.h"
#include "../../source/core/slang-writer.h"
#include "../../source/core/slang-file-system.h"

#include "../../source/compiler-core/slang-source-loc.h"
#include "../../source/compiler-core/slang-lexer.h"
#include "../../source/compiler-core/slang-diagnostic-sink.h"
#include "../../source/compiler-core/slang-name.h"
#include "../../source/compiler-core/slang-name-convention-util.h"

#include "node.h"
#include "diagnostics.h"
#include "options.h"
#include "parser.h"
#include "macro-writer.h"
#include "file-util.h"
#include "unit-test.h"

/*
Some command lines:

-d source/slang slang-ast-support-types.h slang-ast-base.h slang-ast-decl.h slang-ast-expr.h slang-ast-modifier.h slang-ast-stmt.h slang-ast-type.h slang-ast-val.h -strip-prefix slang- -o slang-generated -output-fields -mark-suffix _CLASS
*/

namespace CppExtract
{

using namespace Slang;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! App !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class App
{
public:
    
    SlangResult execute(const Options& options);

        /// Execute
    SlangResult executeWithArgs(int argc, const char*const* argv);

    const Options& getOptions() const { return m_options; }

    App(DiagnosticSink* sink, SourceManager* sourceManager, RootNamePool* rootNamePool):
        m_sink(sink),
        m_sourceManager(sourceManager),
        m_slicePool(StringSlicePool::Style::Default)
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

    // Okay let's check out the typeSets
    {
        for (TypeSet* typeSet : tree.getTypeSets())
        {
            // The macro name is in upper snake, so split it 
            List<UnownedStringSlice> slices;
            NameConventionUtil::split(typeSet->m_macroName, slices);

            if (typeSet->m_fileMark.getLength() == 0)
            {
                StringBuilder buf;
                // Let's guess a 'fileMark' (it becomes part of the filename) based on the macro name. Use lower kabab.
                NameConventionUtil::join(slices.getBuffer(), slices.getCount(), CharCase::Lower, NameConvention::Kabab, buf);
                typeSet->m_fileMark = buf.ProduceString();
            }

            if (typeSet->m_typeName.getLength() == 0)
            {
                // Let's guess a typename if not set -> go with upper camel
                StringBuilder buf;
                NameConventionUtil::join(slices.getBuffer(), slices.getCount(), CharCase::Upper, NameConvention::Camel, buf);
                typeSet->m_typeName = buf.ProduceString();
            }
        }
    }

    // Dump out the tree
    if (options.m_dump)
    {
        {
            StringBuilder buf;
            tree.getRootNode()->dump(0, buf);
            m_sink->writer->write(buf.getBuffer(), buf.getLength());
        }

        for (TypeSet* typeSet : tree.getTypeSets())
        {
            const List<ClassLikeNode*>& baseTypes = typeSet->m_baseTypes;

            for (ClassLikeNode* baseType : baseTypes)
            {
                StringBuilder buf;
                baseType->dumpDerived(0, buf);
                m_sink->writer->write(buf.getBuffer(), buf.getLength());
            }
        }
    }

    if (options.m_defs)
    {
        MacroWriter macroWriter(m_sink, &m_options);
        SLANG_RETURN_ON_FAIL(macroWriter.writeDefs(&tree));
    }

    if (options.m_outputPath.getLength())
    {
        MacroWriter macroWriter(m_sink, &m_options);
        SLANG_RETURN_ON_FAIL(macroWriter.writeOutput(&tree));
    }

    return SLANG_OK;
}

/// Execute
SlangResult App::executeWithArgs(int argc, const char*const* argv)
{
    Options options;
    OptionsParser optionsParser;
    SLANG_RETURN_ON_FAIL(optionsParser.parse(argc, argv, m_sink, options));
    SLANG_RETURN_ON_FAIL(execute(options));
    return SLANG_OK;
}

} // namespace CppExtract

int main(int argc, const char*const* argv)
{
    using namespace CppExtract;
    using namespace Slang;

    {
        ComPtr<ISlangWriter> writer(new FileWriter(stderr, WriterFlag::AutoFlush));

        RootNamePool rootNamePool;

        SourceManager sourceManager;
        sourceManager.initialize(nullptr, nullptr);

        DiagnosticSink sink(&sourceManager, Lexer::sourceLocationLexer);
        sink.writer = writer;

        // Set to true to see command line that initiated C++ extractor. Helpful when finding issues from solution building failing, and then so
        // being able to repeat the issue
        bool dumpCommandLine = false;

        if (dumpCommandLine)
        {
            StringBuilder builder;

            for (Index i = 1; i < argc; ++i)
            {
                builder << argv[i] << " ";
            }

            sink.diagnose(SourceLoc(), CPPDiagnostics::commandLine, builder);
        }

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
    }
    return 0;
}

