// slang-ir-specialize-resources.h
#pragma once

namespace Slang
{
struct CodeGenContext;
struct IRModule;
struct IRType;

/// Specialize calls to functions with resource-type parameters.
///
/// For any function that has resource-type input parameters that
/// would be invalid on the chosen target, this pass will rewrite
/// any call sites that pass suitable arguments (e.g., direct
/// references to global shader parameters) to instead call
/// a specialized variant of the function that does not have
/// those resource parameters (and instead, e.g, refers to the
/// global shader parameters directly).
///
bool specializeResourceParameters(CodeGenContext* codeGenContext, IRModule* module);

/// Specialize resource input and output uses until no transformation can make further progress.
///
/// The operation simplifies the module after each successful specialization or inlining round. If
/// a resource output cannot be reconstructed at its call sites, it tries to inline direct calls to
/// the function and retry specialization. On D3D targets, it also tries to inline a call when a
/// resource input has no IR form that can be emitted directly as HLSL. Returns whether the module
/// changed.
bool specializeResourceUsage(IRModule* irModule, CodeGenContext* codeGenContext);

/// Convert parameter-passing modes for non-copyable types to ones that are valid for GLSL.
///
/// This pass is needed because Slang requires certain non-copyable types like
/// `RayQuery` to be passed via `inout` parameters in order to be mutable, while
/// GLSL does not allow that type to be used for `out` or `inout` parameters.
/// It is semantically okay to change the mode of such parameters over to just
/// use `borrow in` or `in`, because in practice the GLSL type acts more like
/// a handle to a stateful entity rather than a mutable value itself.
///
void legalizeModesOfNonCopyableOpaqueTypedParamsForGLSL(
    IRModule* irModule,
    CodeGenContext* codeGenContext);

bool isIllegalGLSLParameterType(IRType* type);
bool isIllegalSPIRVParameterType(IRType* type, bool isArray);
bool isIllegalWGSLParameterType(IRType* type);

} // namespace Slang
