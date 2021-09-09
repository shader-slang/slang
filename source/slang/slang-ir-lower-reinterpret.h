// slang-ir-lower-reinterpret.h
#pragma once

// This file defines an IR pass that lowers a reinterept<T>(U) operation, where T and U are any ordinary data types,
// into a packAnyValue<T> followed by a unpackAnyValue<U> operation.

namespace Slang
{

struct IRModule;
class TargetRequest;
class DiagnosticSink;

void lowerReinterpret(TargetRequest* targetReq, IRModule* module, DiagnosticSink* sink);

}
