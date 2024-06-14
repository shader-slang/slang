// slang-ir-legalize-varying-params.h
#pragma once

namespace Slang
{

class DiagnosticSink;

struct IRFunc;
struct IRModule;
struct IRInst;
struct IRFunc;
struct IRVectorType;
struct IRBuilder;

void legalizeEntryPointVaryingParamsForCPU(
    IRModule*               module,
    DiagnosticSink*         sink);

void legalizeEntryPointVaryingParamsForCUDA(
    IRModule*               module,
    DiagnosticSink*         sink);

IRInst* emitCalcGroupThreadIndex(
    IRBuilder& builder,
    IRInst* groupThreadID,
    IRInst* groupExtents);

IRInst* emitCalcGroupExtents(
    IRBuilder& builder,
    IRFunc* entryPoint,
    IRVectorType* type);
}
