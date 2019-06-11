// slang-ir-glsl-legalize.h
#pragma once

namespace Slang
{

class DiagnosticSink;
class Session;

class GLSLExtensionTracker;

struct IRFunc;
struct IRModule;

void legalizeEntryPointForGLSL(
    Session*                session,
    IRModule*               module,
    IRFunc*                 func,
    DiagnosticSink*         sink,
    GLSLExtensionTracker*   glslExtensionTracker);

}
