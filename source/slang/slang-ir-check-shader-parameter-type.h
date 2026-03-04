#pragma once

namespace Slang
{
struct IRModule;
class DiagnosticSink;
class TargetRequest;

void checkForInvalidShaderParameterType(
    IRModule* module,
    TargetRequest* targetReq,
    DiagnosticSink* sink);
} // namespace Slang
