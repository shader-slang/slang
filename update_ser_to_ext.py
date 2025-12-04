#!/usr/bin/env python3
"""
Script to update Slang HitObject methods from NV to EXT extensions.
Updates both GLSL function names and SPIRV ops/extensions.
"""

import re

# Read the file
with open('source/slang/hlsl.meta.slang', 'r', encoding='utf-8') as f:
    content = f.read()

# Define replacements
replacements = [
    # GLSL function names
    ('hitObjectIsMissNV', 'hitObjectIsMissEXT'),
    ('hitObjectIsHitNV', 'hitObjectIsHitEXT'),
    ('hitObjectIsEmptyNV', 'hitObjectIsEmptyEXT'),
    ('hitObjectRecordEmptyNV', 'hitObjectRecordEmptyEXT'),
    ('hitObjectRecordMissNV', 'hitObjectRecordMissEXT'),
    ('hitObjectRecordMissMotionNV', 'hitObjectRecordMissMotionEXT'),
    ('hitObjectRecordHitNV', 'hitObjectRecordHitEXT'),
    ('hitObjectRecordHitMotionNV', 'hitObjectRecordHitMotionEXT'),
    ('hitObjectRecordHitWithIndexNV', 'hitObjectRecordHitWithIndexEXT'),
    ('hitObjectRecordHitWithIndexMotionNV', 'hitObjectRecordHitWithIndexMotionEXT'),
    ('hitObjectTraceRayNV', 'hitObjectTraceRayEXT'),
    ('hitObjectTraceRayMotionNV', 'hitObjectTraceRayMotionEXT'),
    ('hitObjectExecuteShaderNV', 'hitObjectExecuteShaderEXT'),
    ('hitObjectGetShaderBindingTableRecordIndexNV', 'hitObjectGetShaderBindingTableRecordIndexEXT'),
    ('hitObjectGetInstanceIdNV', 'hitObjectGetInstanceIdEXT'),
    ('hitObjectGetInstanceCustomIndexNV', 'hitObjectGetInstanceCustomIndexEXT'),
    ('hitObjectGetGeometryIndexNV', 'hitObjectGetGeometryIndexEXT'),
    ('hitObjectGetPrimitiveIndexNV', 'hitObjectGetPrimitiveIndexEXT'),
    ('hitObjectGetHitKindNV', 'hitObjectGetHitKindEXT'),
    ('hitObjectGetClusterIdNV', 'hitObjectGetClusterIdEXT'),
    ('hitObjectGetWorldToObjectNV', 'hitObjectGetWorldToObjectEXT'),
    ('hitObjectGetObjectToWorldNV', 'hitObjectGetObjectToWorldEXT'),
    ('hitObjectGetCurrentTimeNV', 'hitObjectGetCurrentTimeEXT'),
    ('hitObjectGetObjectRayOriginNV', 'hitObjectGetObjectRayOriginEXT'),
    ('hitObjectGetObjectRayDirectionNV', 'hitObjectGetObjectRayDirectionEXT'),
    ('hitObjectGetWorldRayDirectionNV', 'hitObjectGetWorldRayDirectionEXT'),
    ('hitObjectGetWorldRayOriginNV', 'hitObjectGetWorldRayOriginEXT'),
    ('hitObjectGetRayTMaxNV', 'hitObjectGetRayTMaxEXT'),
    ('hitObjectGetRayTMinNV', 'hitObjectGetRayTMinEXT'),
    ('hitObjectGetAttributesNV', 'hitObjectGetAttributesEXT'),
    ('hitObjectGetShaderRecordBufferHandleNV', 'hitObjectGetShaderRecordBufferHandleEXT'),

    # SPIRV extension and capability
    ('SPV_NV_shader_invocation_reorder', 'SPV_EXT_shader_invocation_reorder'),
    ('ShaderInvocationReorderNV', 'ShaderInvocationReorderEXT'),

    # SPIRV ops
    ('OpHitObjectIsMissNV', 'OpHitObjectIsMissEXT'),
    ('OpHitObjectIsHitNV', 'OpHitObjectIsHitEXT'),
    ('OpHitObjectIsEmptyNV', 'OpHitObjectIsEmptyEXT'),
    ('OpHitObjectRecordEmptyNV', 'OpHitObjectRecordEmptyEXT'),
    ('OpHitObjectRecordMissNV', 'OpHitObjectRecordMissEXT'),
    ('OpHitObjectRecordMissMotionNV', 'OpHitObjectRecordMissMotionEXT'),
    ('OpHitObjectRecordHitNV', 'OpHitObjectRecordHitEXT'),
    ('OpHitObjectRecordHitMotionNV', 'OpHitObjectRecordHitMotionEXT'),
    ('OpHitObjectTraceRayNV', 'OpHitObjectTraceRayEXT'),
    ('OpHitObjectTraceRayMotionNV', 'OpHitObjectTraceRayMotionEXT'),
    ('OpHitObjectExecuteShaderNV', 'OpHitObjectExecuteShaderEXT'),
    ('OpHitObjectGetShaderBindingTableRecordIndexNV', 'OpHitObjectGetShaderBindingTableRecordIndexEXT'),
    ('OpHitObjectSetShaderBindingTableRecordIndexNV', 'OpHitObjectSetShaderBindingTableRecordIndexEXT'),
    ('OpHitObjectGetInstanceIdNV', 'OpHitObjectGetInstanceIdEXT'),
    ('OpHitObjectGetInstanceCustomIndexNV', 'OpHitObjectGetInstanceCustomIndexEXT'),
    ('OpHitObjectGetGeometryIndexNV', 'OpHitObjectGetGeometryIndexEXT'),
    ('OpHitObjectGetPrimitiveIndexNV', 'OpHitObjectGetPrimitiveIndexEXT'),
    ('OpHitObjectGetHitKindNV', 'OpHitObjectGetHitKindEXT'),
    ('OpHitObjectGetWorldToObjectNV', 'OpHitObjectGetWorldToObjectEXT'),
    ('OpHitObjectGetObjectToWorldNV', 'OpHitObjectGetObjectToWorldEXT'),
    ('OpHitObjectGetCurrentTimeNV', 'OpHitObjectGetCurrentTimeEXT'),
    ('OpHitObjectGetObjectRayOriginNV', 'OpHitObjectGetObjectRayOriginEXT'),
    ('OpHitObjectGetObjectRayDirectionNV', 'OpHitObjectGetObjectRayDirectionEXT'),
    ('OpHitObjectGetWorldRayDirectionNV', 'OpHitObjectGetWorldRayDirectionEXT'),
    ('OpHitObjectGetWorldRayOriginNV', 'OpHitObjectGetWorldRayOriginEXT'),
    ('OpHitObjectGetRayTMaxNV', 'OpHitObjectGetRayTMaxEXT'),
    ('OpHitObjectGetRayTMinNV', 'OpHitObjectGetRayTMinEXT'),
    ('OpHitObjectGetAttributesNV', 'OpHitObjectGetAttributesEXT'),
    ('OpHitObjectGetShaderRecordBufferHandleNV', 'OpHitObjectGetShaderRecordBufferHandleEXT'),

    # GLSL extension decorator
    ('__glsl_extension(GL_NV_shader_invocation_reorder)', '__glsl_extension(GL_EXT_shader_invocation_reorder)'),
]

# Apply replacements
for old, new in replacements:
    content = content.replace(old, new)

# Write back
with open('source/slang/hlsl.meta.slang', 'w', encoding='utf-8') as f:
    f.write(content)

print(f"Updated {len(replacements)} patterns in hlsl.meta.slang")
print("Done!")
