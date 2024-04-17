// slang-ir-header-export.h
#pragma once

namespace Slang
{
    struct IRModule;
    class DiagnosticSink;
    // Transitively marks as [HeaderExport].
    void generateTransitiveHeaderExports(IRModule* module, DiagnosticSink* sink);
}
