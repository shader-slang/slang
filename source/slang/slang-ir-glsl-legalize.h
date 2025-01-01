// slang-ir-glsl-legalize.h
#pragma once
#include "../core/slang-list.h"
#include "slang-compiler.h"

namespace Slang
{

class DiagnosticSink;
class Session;

class GLSLExtensionTracker;

struct IRFunc;
struct IRModule;

void legalizeEntryPointsForGLSL(
    Session* session,
    IRModule* module,
    const List<IRFunc*>& func,
    CodeGenContext* context,
    GLSLExtensionTracker* glslExtensionTracker);

void legalizeConstantBufferLoadForGLSL(IRModule* module);

void legalizeDispatchMeshPayloadForGLSL(IRModule* module);

void legalizeDynamicResourcesForGLSL(CodeGenContext* context, IRModule* module);

// Searches through entry point function bodies for instructions that require the
// attribute "[[ ... ]]" syntax or other layout modifiers on the entry point for GLSL.
void legalizeFunctionsForEntryPointAttributesForGLSL(IRModule* module, const List<IRFunc*>& func);

} // namespace Slang
