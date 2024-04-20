// slang-ir-header-export.h
#pragma once

namespace Slang
{
    struct IRModule;
    class DiagnosticSink;
    // Transitively marks as __extern_cpp (ExternCppDecoration).
    void generateTransitiveExternCpp(IRModule* module, DiagnosticSink* sink);
}
