#pragma once

namespace Slang {

class DiagnosticSink;
struct IRModule;

// Merge all import symbols with the same mangled name into one single global inst.
// This is necessary to prevent loss of decoration on redundantly defined import symbols during DCE.
//
void mergeImportSymbols(IRModule* inModule);

}
