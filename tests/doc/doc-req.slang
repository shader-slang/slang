//TEST:DOC:-module-name module -doc -no-codegen

// To Test out documentation around intrinsics

__target_intrinsic(hlsl, "someHLSLThing()")
[__requiresNVAPI]
__cuda_sm_version(2.0)
__target_intrinsic(cuda, "someCUDAThing()")
__target_intrinsic(glsl, "someGLSLThing()")
__glsl_extension(GL_EXT_NiftyExtension)
__spirv_version(1.3)
__target_intrinsic(cpp, "someCPPThing()")
void doThing()
{
}
