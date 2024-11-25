#pragma once

#include "../../source/compiler-core/slang-diagnostic-sink.h"
#include "../tools/slang-cpp-parser/diagnostics.h"
#include "../tools/slang-cpp-parser/node-tree.h"
#include "../tools/slang-cpp-parser/options.h"

namespace CppExtract
{
using namespace Slang;
using namespace CppParse;

/* A class that writes out macros that define type hierarchies, as well as fields of types */
class MacroWriter
{
public:
    /// Write output
    SlangResult writeOutput(NodeTree* tree);

    /// Write def files
    SlangResult writeDefs(NodeTree* tree);

    /// Calculate the header
    SlangResult calcTypeHeader(NodeTree* tree, TypeSet* typeSet, StringBuilder& out);
    SlangResult calcChildrenHeader(NodeTree* tree, TypeSet* typeSet, StringBuilder& out);
    SlangResult calcOriginHeader(NodeTree* tree, StringBuilder& out);

    SlangResult calcDef(NodeTree* tree, SourceOrigin* origin, StringBuilder& out);

    /// Ctor.
    MacroWriter(DiagnosticSink* sink, const Options* options)
        : m_sink(sink), m_options(options)
    {
    }

protected:
    const Options* m_options = nullptr;
    DiagnosticSink* m_sink;
};

} // namespace CppExtract
