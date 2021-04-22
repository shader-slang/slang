#ifndef CPP_EXTRACT_MACRO_WRITER_H
#define CPP_EXTRACT_MACRO_WRITER_H

#include "diagnostics.h"

#include "options.h"
#include "node-tree.h"

#include "../../source/compiler-core/slang-diagnostic-sink.h"

namespace CppExtract {
using namespace Slang;

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
    MacroWriter(DiagnosticSink* sink, const Options* options): 
        m_sink(sink),
        m_options(options)
    {
    }

protected:

    const Options* m_options = nullptr;
    DiagnosticSink* m_sink;
};

} // CppExtract

#endif
