// slang-ir-glsl-legalize.h
#pragma once
#include"../core/slang-list.h"
#include "slang-compiler.h"

namespace Slang
{

class DiagnosticSink;
class Session;

class GLSLExtensionTracker;

struct IRFunc;
struct IRModule;

void legalizeEntryPointsForGLSL(
    Session*                session,
    IRModule*               module,
    const List<IRFunc*>&    func,
    CodeGenContext*         context,
    GLSLExtensionTracker*   glslExtensionTracker);

void legalizeImageSubscriptForGLSL(IRModule* module);

void legalizeConstantBufferLoadForGLSL(IRModule* module);

void legalizeDispatchMeshPayloadForGLSL(IRModule* module);

}
