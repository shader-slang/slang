// slang-ir-legalize-varying-params.h
#pragma once

namespace Slang
{

class DiagnosticSink;

struct IRFunc;
struct IRModule;

void legalizeEntryPointVaryingParamsForCPU(
    IRModule*               module,
    DiagnosticSink*         sink);

void legalizeEntryPointVaryingParamsForCUDA(
    IRModule*               module,
    DiagnosticSink*         sink);

}
