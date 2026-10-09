// This template is evaluated by slang-standard-module-generator, not by the Slang compiler.
// Keep the numeric values tied to KnownBuiltinDeclName so adding another compiler builtin cannot
// silently change which role an annotation selects.
namespace rt
{

// @hidden:
// Identifies the ordinary declarations whose roles are consumed by ray-tracing checking.
// For example, a Context requirement keeps its identity after importing a serialized module;
// neither the containing context interface nor its implementations need a special AST type.
public enum __RayTracingBuiltin : int
{
    ShaderList = $((int)KnownBuiltinDeclName::RayTracingShaderList),
    ClosestHitInput = $((int)KnownBuiltinDeclName::RayTracingClosestHitInput),
    AnyHitInput = $((int)KnownBuiltinDeclName::RayTracingAnyHitInput),
    IntersectionInput = $((int)KnownBuiltinDeclName::RayTracingIntersectionInput),
    MissInput = $((int)KnownBuiltinDeclName::RayTracingMissInput),
    CallableInput = $((int)KnownBuiltinDeclName::RayTracingCallableInput),
    TrianglePrimitive = $((int)KnownBuiltinDeclName::RayTracingTrianglePrimitive),
    CurvePrimitive = $((int)KnownBuiltinDeclName::RayTracingCurvePrimitive),
    StageContext = $((int)KnownBuiltinDeclName::RayTracingStageContext),
    StageRecord = $((int)KnownBuiltinDeclName::RayTracingStageRecord),
    PayloadContextPayload = $((int)KnownBuiltinDeclName::RayTracingPayloadContextPayload),
    HitPrimitive = $((int)KnownBuiltinDeclName::RayTracingHitPrimitive),
    PrimitiveAttributes = $((int)KnownBuiltinDeclName::RayTracingPrimitiveAttributes),
    CallableData = $((int)KnownBuiltinDeclName::RayTracingCallableData),
    ProgramHitGroups = $((int)KnownBuiltinDeclName::RayTracingProgramHitGroups),
    ProgramMissShaders = $((int)KnownBuiltinDeclName::RayTracingProgramMissShaders),
    ProgramCallableShaders = $((int)KnownBuiltinDeclName::RayTracingProgramCallableShaders),
};

} // namespace rt
