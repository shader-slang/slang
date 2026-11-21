// slang-ir-marshal-native-call.h
#pragma once

#include "core/slang-basic.h"

namespace Slang
{
class DiagnosticSink;
struct IRModule;
struct IRBuilder;
struct IRType;
struct IRFunc;
struct IRFuncType;
struct IRCall;
struct IRInst;
struct IRVar;

class NativeCallMarshallingContext
{
public:
    DiagnosticSink* diagnosticSink = nullptr;

public:
    // Get a native type for `type` that can be used directly in a native function signature.
    IRType* getNativeType(IRBuilder& builder, IRType* type);

    // Get a native function type of `func`.
    IRFuncType* getNativeFuncType(IRBuilder& builder, IRFuncType* declaredFuncType);

    // Insert a call at builder's current position into a native func with original arguments.
    // `originalArgs` will be marshalled to native args before the actual call.
    // returns the managed result value of the call.
    // Note: additional insts maybe inserted after the call inst to marshal the native output values
    // back to non-native arguments/return values.
    IRInst* marshalNativeCall(
        IRBuilder& builder,
        IRFuncType* originalFuncType,
        IRFuncType* nativeFuncType,
        IRInst* nativeFunc,
        Int argCount,
        IRInst* const* originalArgs);

    /// Describes a by-reference argument whose native representation differs from its managed
    /// representation. The call passes the address of `nativeVar`, and `marshalNativeCall`
    /// converts the temporary back into `*managedPtr` after the native call returns.
    struct NativeArgWriteBack
    {
        IRInst* managedPtr = nullptr;
        IRVar* nativeVar = nullptr;
    };

    /// Marshals the pointer argument of an `out`, `inout`, `ref`, or `__constref` parameter.
    ///
    /// When the pointee already has its native representation, the caller's own storage is passed
    /// through. Otherwise the argument becomes a native temporary, initialized from the managed
    /// value unless the parameter is `out`, and recorded in `outWriteBacks` unless the parameter is
    /// read-only, so the callee's result reaches the caller's storage.
    void marshalRefManagedValueToNativeValue(
        IRBuilder& builder,
        IRType* originalParamType,
        IRInst* originalArg,
        List<IRInst*>& args,
        List<NativeArgWriteBack>& outWriteBacks);

    // Marshal a managed value to a native value for input into a native functions.
    void marshalManagedValueToNativeValue(
        IRBuilder& builder,
        IRType* originalParamType,
        IRInst* originalArg,
        List<IRInst*>& args,
        List<NativeArgWriteBack>& outWriteBacks);

    // Marshal a managed value to a native value for the return value of a native function.
    void marshalManagedValueToNativeResultValue(
        IRBuilder& builder,
        IRInst* originalArg,
        List<IRInst*>& args);

    IRInst* marshalNativeValueToManagedValue(IRBuilder& builder, IRInst* nativeValue);

    IRInst* marshalNativeArgToManagedArg(
        IRBuilder& builder,
        const List<IRInst*>& args,
        Index& consumeIndex,
        IRType* expectedManagedType);

    IRFunc* generateDLLExportWrapperFunc(IRBuilder& builder, IRFunc* originalFunc);
};
} // namespace Slang
