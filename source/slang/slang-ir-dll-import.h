// slang-ir-dll-import.h
#pragma once

namespace Slang
{
    struct IRModule;
    class DiagnosticSink;
    class TargetRequest;
        /// Generate implementations for functions marked as [DllImport].
    void generateDllImportFuncs(TargetRequest* targetReq, IRModule* module, DiagnosticSink* sink);
}
