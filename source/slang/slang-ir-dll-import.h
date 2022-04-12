// slang-ir-dll-import.h
#pragma once

namespace Slang
{
    struct IRModule;
    class DiagnosticSink;
        /// Generate implementations for functions marked as [DllImport].
    void generateDllImportFuncs(IRModule* module, DiagnosticSink* sink);

}
