// slang-ir-glsl-legalize.h
#pragma once
#include "core/slang-list.h"
#include "slang-compiler.h"

namespace Slang
{

class DiagnosticSink;
class Session;

class ShaderExtensionTracker;

struct IRFunc;
struct IRModule;

void legalizeEntryPointsForGLSL(
    IRModule* module,
    Session* session,
    const List<IRFunc*>& func,
    CodeGenContext* context,
    ShaderExtensionTracker* glslExtensionTracker);

// GLSL and SPIR-V both require an integer `switch` selector, but the front end accepts a
// `switch` on a `bool` (with `case true:`/`case false:`) and lowers it unchanged. Rewrite
// every such switch in `module` into the equivalent integer switch so the Khronos emitters
// never see a boolean selector. Runs for GLSL and SPIR-V; other targets accept a bool switch.
void legalizeBoolSwitchForKhronos(IRModule* module);

void legalizeConstantBufferLoadForGLSL(IRModule* module);

void legalizeDispatchMeshPayloadForGLSL(IRModule* module);

void legalizeDynamicResourcesForGLSL(IRModule* module, CodeGenContext* context);
} // namespace Slang
