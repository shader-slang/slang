// slang-ir-legalize-varying-params.h
#pragma once

namespace Slang
{

class DiagnosticSink;

struct IRModule;
struct IRInst;
struct IRFunc;
struct IRVectorType;
struct IRBuilder;
struct IREntryPointDecoration;

void legalizeEntryPointVaryingParamsForCPU(
    IRModule*               module,
    DiagnosticSink*         sink);

void legalizeEntryPointVaryingParamsForCUDA(
    IRModule*               module,
    DiagnosticSink*         sink);

void legalizeEntryPointVaryingParamsForMetal(
    IRModule*               module,
    DiagnosticSink*         sink);

struct EntryPointInfo
{
    IRFunc* entryPointFunc;
    IREntryPointDecoration* entryPointDecor;
};

//TODO: remove these below
IRInst* emitCalcGroupThreadIndex(
    IRBuilder& builder,
    IRInst* groupThreadID,
    IRInst* groupExtents);

IRInst* emitCalcGroupExtents(
    IRBuilder& builder,
    IRFunc* entryPoint,
    IRVectorType* type);
}
