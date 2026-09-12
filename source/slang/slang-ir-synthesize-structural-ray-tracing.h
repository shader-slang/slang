#pragma once

#include "core/slang-dictionary.h"
#include "core/slang-list.h"

namespace Slang
{

struct IRModule;
struct IRFunc;
struct IRInst;
class DiagnosticSink;
class TargetRequest;

/// Validate the payload and record ABI types of a selected standalone structural stage.
bool validateStructuralRayTracingEntryPoint(IRFunc* entryPoint, DiagnosticSink* sink);

/// Validate the target-independent relationships encoded by one structural trace or callable
/// operation after linking and specialization have made its complete closed schema available.
bool validateStructuralRayTracingSchemaOperation(IRInst* operation, DiagnosticSink* sink);

/// Replace selected logical stage methods with zero-parameter native entry-point adapters before
/// generic entry-point legalization examines their signatures.
void preparePortableStructuralRayTracingEntryPoints(IRModule* module, List<IRFunc*>& ioEntryPoints);

/// Generate native D3D/Vulkan entry-point adapters for stages selected by structural layouts.
void synthesizePortableStructuralRayTracingEntryPoints(
    IRModule* module,
    List<IRFunc*>& ioEntryPoints,
    DiagnosticSink* sink);

/// Lower structural stage-input operations through their portable standard-module bodies.
void lowerPortableStructuralRayTracingStageInputOperations(IRModule* module);

/// Thread compiler-provided payload parameters through generated Metal visible-stage adapters.
void lowerMetalStructuralRayTracingStageInputOperations(
    IRModule* module,
    const Dictionary<IRFunc*, IRInst*>& entryPointPayloadValues);

/// Lower structural trace and callable-dispatch operations through their portable
/// standard-module bodies.
void lowerPortableStructuralRayTracingOperations(IRModule* module, TargetRequest* targetRequest);

} // namespace Slang
