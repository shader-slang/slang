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
struct IRFunc;

class NativeCallMarshallingContext
{
public:
    DiagnosticSink* diagnosticSink = nullptr;

public:
    // Get a native type for `type` that can be used directly in a native function signature.
    IRType* getNativeType(IRBuilder& builder, IRType* type);

    // Get a native function type of `func`.
    IRFuncType* getNativeFuncType(IRBuilder& builder, IRFuncType* declaredFuncType);

    /// Diagnoses every parameter and result of `func` whose type cannot cross a native
    /// `[DllImport]` or `[DllExport]` boundary, and returns whether the signature is supported.
    ///
    /// Only types without managed elements have a lifetime that native code can honor, so a
    /// signature may use plain-old-data types (scalars, vectors, matrices, `NativeString`, raw
    /// pointers to such types, and structs and fixed-size arrays of them) and `[COM]` interfaces,
    /// whose ownership follows `AddRef`/`Release`. A `String` is additionally accepted as a
    /// by-value input parameter, because the call only borrows its bytes (an import passes
    /// `const char*`) or copies them (an export builds a new `String`). Any other `String`, and
    /// any class, resource, or aggregate containing one, is rejected.
    bool checkNativeSignature(IRFunc* func);

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

    void marshalRefManagedValueToNativeValue(
        IRBuilder& builder,
        IRInst* originalArg,
        List<IRInst*>& args);

    // Marshal a managed value to a native value for input into a native functions.
    void marshalManagedValueToNativeValue(
        IRBuilder& builder,
        IRType* originalParamType,
        IRInst* originalArg,
        List<IRInst*>& args);

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
