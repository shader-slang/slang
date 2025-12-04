#!/usr/bin/env python3
"""
Script to update HitObject methods to support both NV and EXT extensions.
Transforms single glsl/spirv cases into glsl_nv/glsl_ext and spirv_nv/spirv_ext cases.
"""

import re

# Read the file
with open('source/slang/hlsl.meta.slang', 'r', encoding='utf-8') as f:
    content = f.read()

# Pattern 1: Add dual extension decorators before HitObject methods
# Find lines with only __glsl_extension(GL_EXT_shader_invocation_reorder)
# and add __glsl_extension(GL_NV_shader_invocation_reorder) before them
content = re.sub(
    r'(__glsl_extension\(GL_EXT_shader_invocation_reorder\))\n',
    r'__glsl_extension(GL_NV_shader_invocation_reorder)\n\1\n',
    content
)

# Pattern 2: Split "case glsl:" with EXT functions into glsl_nv and glsl_ext
glsl_patterns = [
    ('hitObjectIsMissEXT', 'hitObjectIsMissNV'),
    ('hitObjectIsHitEXT', 'hitObjectIsHitNV'),
    ('hitObjectIsEmptyEXT', 'hitObjectIsEmptyNV'),
    ('hitObjectRecordEmptyEXT', 'hitObjectRecordEmptyNV'),
    ('hitObjectRecordMissEXT', 'hitObjectRecordMissNV'),
    ('hitObjectRecordMissMotionEXT', 'hitObjectRecordMissMotionNV'),
    ('hitObjectTraceRayEXT', 'hitObjectTraceRayNV'),
    ('hitObjectTraceRayMotionEXT', 'hitObjectTraceRayMotionNV'),
    ('hitObjectExecuteShaderEXT', 'hitObjectExecuteShaderNV'),
    ('hitObjectGetInstanceIdEXT', 'hitObjectGetInstanceIdNV'),
    ('hitObjectGetInstanceCustomIndexEXT', 'hitObjectGetInstanceCustomIndexNV'),
    ('hitObjectGetGeometryIndexEXT', 'hitObjectGetGeometryIndexNV'),
    ('hitObjectGetPrimitiveIndexEXT', 'hitObjectGetPrimitiveIndexNV'),
    ('hitObjectGetHitKindEXT', 'hitObjectGetHitKindNV'),
    ('hitObjectGetWorldToObjectEXT', 'hitObjectGetWorldToObjectNV'),
    ('hitObjectGetObjectToWorldEXT', 'hitObjectGetObjectToWorldNV'),
    ('hitObjectGetCurrentTimeEXT', 'hitObjectGetCurrentTimeNV'),
    ('hitObjectGetObjectRayOriginEXT', 'hitObjectGetObjectRayOriginNV'),
    ('hitObjectGetObjectRayDirectionEXT', 'hitObjectGetObjectRayDirectionNV'),
    ('hitObjectGetWorldRayDirectionEXT', 'hitObjectGetWorldRayDirectionNV'),
    ('hitObjectGetWorldRayOriginEXT', 'hitObjectGetWorldRayOriginNV'),
    ('hitObjectGetRayTMaxEXT', 'hitObjectGetRayTMaxNV'),
    ('hitObjectGetRayTMinEXT', 'hitObjectGetRayTMinNV'),
    ('hitObjectGetAttributesEXT', 'hitObjectGetAttributesNV'),
    ('hitObjectGetShaderRecordBufferHandleEXT', 'hitObjectGetShaderRecordBufferHandleNV'),
    ('hitObjectGetShaderBindingTableRecordIndexEXT', 'hitObjectGetShaderBindingTableRecordIndexNV'),
    ('reorderThreadEXT', 'reorderThreadNV'),
]

for ext_func, nv_func in glsl_patterns:
    # Pattern: case glsl: __intrinsic_asm "extFunc(...)"
    pattern = r'(case glsl:)(\s+__intrinsic_asm\s+"' + re.escape(ext_func) + r'[^"]*")'
    replacement = r'case glsl_nv:\2'.replace(ext_func, nv_func) + r'\n        case glsl_ext:\2'
    content = re.sub(pattern, replacement, content)

# Pattern 3: Split "case spirv:" with EXT operations into spirv_nv and spirv_ext
spirv_patterns = [
    ('OpHitObjectIsMissEXT', 'OpHitObjectIsMissNV'),
    ('OpHitObjectIsHitEXT', 'OpHitObjectIsHitNV'),
    ('OpHitObjectIsEmptyEXT', 'OpHitObjectIsEmptyNV'),
    ('OpHitObjectRecordEmptyEXT', 'OpHitObjectRecordEmptyNV'),
    ('OpHitObjectRecordMissEXT', 'OpHitObjectRecordMissNV'),
    ('OpHitObjectRecordMissMotionEXT', 'OpHitObjectRecordMissMotionNV'),
    ('OpHitObjectTraceRayEXT', 'OpHitObjectTraceRayNV'),
    ('OpHitObjectTraceRayMotionEXT', 'OpHitObjectTraceRayMotionNV'),
    ('OpHitObjectExecuteShaderEXT', 'OpHitObjectExecuteShaderNV'),
    ('OpHitObjectGetInstanceIdEXT', 'OpHitObjectGetInstanceIdNV'),
    ('OpHitObjectGetInstanceCustomIndexEXT', 'OpHitObjectGetInstanceCustomIndexNV'),
    ('OpHitObjectGetGeometryIndexEXT', 'OpHitObjectGetGeometryIndexNV'),
    ('OpHitObjectGetPrimitiveIndexEXT', 'OpHitObjectGetPrimitiveIndexNV'),
    ('OpHitObjectGetHitKindEXT', 'OpHitObjectGetHitKindNV'),
    ('OpHitObjectGetWorldToObjectEXT', 'OpHitObjectGetWorldToObjectNV'),
    ('OpHitObjectGetObjectToWorldEXT', 'OpHitObjectGetObjectToWorldNV'),
    ('OpHitObjectGetCurrentTimeEXT', 'OpHitObjectGetCurrentTimeNV'),
    ('OpHitObjectGetObjectRayOriginEXT', 'OpHitObjectGetObjectRayOriginNV'),
    ('OpHitObjectGetObjectRayDirectionEXT', 'OpHitObjectGetObjectRayDirectionNV'),
    ('OpHitObjectGetWorldRayDirectionEXT', 'OpHitObjectGetWorldRayDirectionNV'),
    ('OpHitObjectGetWorldRayOriginEXT', 'OpHitObjectGetWorldRayOriginNV'),
    ('OpHitObjectGetRayTMaxEXT', 'OpHitObjectGetRayTMaxNV'),
    ('OpHitObjectGetRayTMinEXT', 'OpHitObjectGetRayTMinNV'),
    ('OpHitObjectGetAttributesEXT', 'OpHitObjectGetAttributesNV'),
    ('OpHitObjectGetShaderRecordBufferHandleEXT', 'OpHitObjectGetShaderRecordBufferHandleNV'),
    ('OpHitObjectGetShaderBindingTableRecordIndexEXT', 'OpHitObjectGetShaderBindingTableRecordIndexEXT'),  # This one doesn't have NV
    ('OpReorderThreadWithHintEXT', 'OpReorderThreadWithHintNV'),
    ('OpReorderThreadWithHitObjectEXT', 'OpReorderThreadWithHitObjectNV'),
]

# For SPIRV, we need to duplicate entire spirv_asm blocks
# This is more complex, so let's do specific replacements

# Helper function to create dual SPIRV cases
def create_dual_spirv(ext_op, nv_op, match_obj):
    """Create both spirv_nv and spirv_ext cases from a spirv_asm block."""
    indent = match_obj.group(1)
    spirv_block = match_obj.group(2)

    # Create NV version
    nv_block = spirv_block.replace('SPV_EXT_shader_invocation_reorder', 'SPV_NV_shader_invocation_reorder')
    nv_block = nv_block.replace('ShaderInvocationReorderEXT', 'ShaderInvocationReorderNV')
    nv_block = nv_block.replace(ext_op, nv_op)

    result = f'{indent}case spirv_nv:\n{indent}    return spirv_asm\n{indent}    {{\n{nv_block}{indent}    }};\n'
    result += f'{indent}case spirv_ext:\n{indent}    return spirv_asm\n{indent}    {{\n{spirv_block}{indent}    }};'

    return result

# Find and replace spirv_asm blocks for each operation
for ext_op, nv_op in spirv_patterns:
    # Match pattern: case spirv: return spirv_asm { ... OpExtXXX ... };
    pattern = r'(        )(case spirv:\s+return spirv_asm\s+\{\s+OpExtension "SPV_EXT_shader_invocation_reorder";\s+OpCapability ShaderInvocationReorderEXT;\s+)(.*?' + re.escape(ext_op) + r'.*?)(        };)'

    def replacer(m):
        indent = m.group(1)
        spirv_content = m.group(3)

        # Create NV version
        nv_content = spirv_content.replace(ext_op, nv_op)

        result = f'{indent}case spirv_nv:\n{indent}    return spirv_asm\n{indent}    {{\n{indent}        OpExtension "SPV_NV_shader_invocation_reorder";\n{indent}        OpCapability ShaderInvocationReorderNV;\n{indent}        {nv_content}{indent}    }};\n'
        result += f'{indent}case spirv_ext:\n{indent}    return spirv_asm\n{indent}    {{\n{indent}        OpExtension "SPV_EXT_shader_invocation_reorder";\n{indent}        OpCapability ShaderInvocationReorderEXT;\n{indent}        {spirv_content}{indent}    }};'

        return result

    content = re.sub(pattern, replacer, content, flags=re.DOTALL)

# Write back
with open('source/slang/hlsl.meta.slang', 'w', encoding='utf-8') as f:
    f.write(content)

print("Updated HitObject methods to support dual NV/EXT API")
