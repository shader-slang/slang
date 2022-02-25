// slang-ir-glsl-legalize.h
#pragma once
#include"../core/slang-list.h"

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
    DiagnosticSink*         sink,
    GLSLExtensionTracker*   glslExtensionTracker);

void legalizeImageSubscriptForGLSL(IRModule* module);

}
