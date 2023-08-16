// slang-capability-defs.h

// This file uses macros to define the capability "atoms" that
// are used by the `CapabilitySet` implementation.
//
// Any file that `#include`s this file is required to set
// the `SLANG_CAPABILITY_ATOM` macro before including it.
//
#ifndef SLANG_CAPABILITY_ATOM
#error Must define SLANG_CAPABILITY_ATOM before including.
#endif
//
// It is not necessary to `#undef` the macro in the client
// file, because this file will `#undef` it at the end.

// Our representation allows each capability atom to define
// a number of other base atoms that it "inherits" from.
//
// Different atoms will need different numbers of bases,
// so we will define a few different macros that wrap
// `SLANG_CAPABILITY_ATOM` and let us handle the cases
// more conveniently.
//
// TODO: There is probably a way to handle this with
// variadic macros.
//
#define SLANG_CAPABILITY_ATOM4(ENUMERATOR, NAME, KIND, CONFLICTS, RANK, BASE0, BASE1, BASE2, BASE3) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, KIND, CONFLICTS, RANK, BASE0, BASE1, BASE2, BASE3)

#define SLANG_CAPABILITY_ATOM3(ENUMERATOR, NAME, KIND, CONFLICTS, RANK, BASE0, BASE1, BASE2) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, KIND, CONFLICTS, RANK, BASE0, BASE1, BASE2, Invalid)

#define SLANG_CAPABILITY_ATOM2(ENUMERATOR, NAME, KIND, CONFLICTS, RANK, BASE0, BASE1) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, KIND, CONFLICTS, RANK, BASE0, BASE1, Invalid, Invalid)

#define SLANG_CAPABILITY_ATOM1(ENUMERATOR, NAME, KIND, CONFLICTS, RANK, BASE0) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, KIND, CONFLICTS, RANK, BASE0, Invalid, Invalid, Invalid)

#define SLANG_CAPABILITY_ATOM0(ENUMERATOR, NAME, KIND, CONFLICTS, RANK) \
    SLANG_CAPABILITY_ATOM(ENUMERATOR, NAME, KIND, CONFLICTS, RANK, Invalid, Invalid, Invalid, Invalid)

// Several capabilities represent the target formats in which we generate code.
// Because we can only generate code in one format at a time, all of these are
// marked as conflicting with one another along the `TargetFormat` axis.
//
// Note: We are only including here the source code formats we initially generate
// code in and not the formats that code might be translated into "downstream."
// Trying to figure out how to integrate both kinds of formats into our capability
// system will be an interesting challenge (e.g., can we compile code for `hlsl+spirv`
// and for `glsl+spirv` or even just for `spirv`, and how should all of those impact
// overloading).
//
SLANG_CAPABILITY_ATOM0(TEXTUAL_SOURCE, textual_source, Abstract,None,0)
SLANG_CAPABILITY_ATOM1(HLSL,     hlsl,      Concrete,TargetFormat,0,TEXTUAL_SOURCE)
SLANG_CAPABILITY_ATOM1(GLSL,     glsl,      Concrete,TargetFormat,0,TEXTUAL_SOURCE)
SLANG_CAPABILITY_ATOM1(C,        c,         Concrete,TargetFormat,0,TEXTUAL_SOURCE)
SLANG_CAPABILITY_ATOM1(CPP,      cpp,       Concrete,TargetFormat,0,TEXTUAL_SOURCE)
SLANG_CAPABILITY_ATOM1(CUDA,     cuda,      Concrete,TargetFormat,0,TEXTUAL_SOURCE)

SLANG_CAPABILITY_ATOM0(SPIRV_DIRECT, spirv, Concrete, TargetFormat, 0)

// We have multiple capabilities for the various SPIR-V versions,
// arranged so that they inherit from one another to represent which versions
// provide a super-set of the features of earlier ones (e.g., SPIR-V 1.4 is
// expressed as inheriting from SPIR-V 1.3).
//
SLANG_CAPABILITY_ATOM1(SPIRV,       __spirv,    Abstract,None,0,   GLSL)
SLANG_CAPABILITY_ATOM1(SPIRV_1_0,   spirv_1_0,  Concrete,None,0,   SPIRV)
SLANG_CAPABILITY_ATOM1(SPIRV_1_1,   spirv_1_1,  Concrete,None,0,   SPIRV_1_0)
SLANG_CAPABILITY_ATOM1(SPIRV_1_2,   spirv_1_2,  Concrete,None,0,   SPIRV_1_1)
SLANG_CAPABILITY_ATOM1(SPIRV_1_3,   spirv_1_3,  Concrete,None,0,   SPIRV_1_2)
SLANG_CAPABILITY_ATOM1(SPIRV_1_4,   spirv_1_4,  Concrete,None,0,   SPIRV_1_3)
SLANG_CAPABILITY_ATOM1(SPIRV_1_5,   spirv_1_5,  Concrete,None,0,   SPIRV_1_4)

// The following capabilities all pertain to how ray tracing shaders are translated
// to GLSL, where there are two different extensions that can provide the core
// functionality of `TraceRay` and the related operations.
//
// The two extensions are expressed as distinct capabilities that both are marked
// as conflicting on the `RayTracingExtension` axis, so that a compilation target
// cannot have both enabled at once.
//
// The `GL_EXT_ray_tracing` extension should be favored, so it has a rank of `1`
// instead of `0`, which means that when comparing overloads that require these
// extensions, the `EXT` extension will be favored over the `NV` extension, if
// all other factors are equal.
//
SLANG_CAPABILITY_ATOM1(GLSLRayTracing,      __glslRayTracing,   Abstract,None,0,   GLSL)
SLANG_CAPABILITY_ATOM1(GL_NV_ray_tracing,   GL_NV_ray_tracing,  Concrete,RayTracingExtension,0,   GLSLRayTracing)
SLANG_CAPABILITY_ATOM2(GL_EXT_ray_tracing,  GL_EXT_ray_tracing, Concrete,RayTracingExtension,1,   GLSLRayTracing, SPIRV_1_4)

SLANG_CAPABILITY_ATOM1(GLSLFragmentShaderBarycentric,           __glslFragmentShaderBarycentric,         Abstract, None,                               0, GLSL)
SLANG_CAPABILITY_ATOM1(GL_NV_fragment_shader_barycentric,       GL_NV_fragment_shader_barycentric,       Concrete, FragmentShaderBarycentricExtension, 0, GLSLFragmentShaderBarycentric)
SLANG_CAPABILITY_ATOM1(GL_EXT_fragment_shader_barycentric,      GL_EXT_fragment_shader_barycentric,      Concrete, FragmentShaderBarycentricExtension, 1, GLSLFragmentShaderBarycentric)


#undef SLANG_CAPABILITY_ATOM0
#undef SLANG_CAPABILITY_ATOM1
#undef SLANG_CAPABILITY_ATOM2
#undef SLANG_CAPABILITY_ATOM3
#undef SLANG_CAPABILITY_ATOM4

#undef SLANG_CAPABILITY_ATOM
