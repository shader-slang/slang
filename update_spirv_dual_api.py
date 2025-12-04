#!/usr/bin/env python3
"""
Script to add dual SPIRV NV/EXT support to HitObject methods.
Transforms single spirv cases into spirv_nv and spirv cases.
"""

import re

# Read the file
with open('source/slang/hlsl.meta.slang', 'r', encoding='utf-8') as f:
    content = f.read()

# Mapping of EXT ops to NV ops
spirv_ops = [
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
    ('OpHitObjectGetShaderBindingTableRecordIndexEXT', 'OpHitObjectGetShaderBindingTableRecordIndexNV'),
    ('OpReorderThreadWithHintEXT', 'OpReorderThreadWithHintNV'),
    ('OpReorderThreadWithHitObjectEXT', 'OpReorderThreadWithHitObjectNV'),
]

# Function to create dual SPIRV case
def create_dual_spirv_case(match):
    """Transform a single spirv case into spirv_nv and spirv cases."""
    indent = match.group(1)
    spirv_content = match.group(2)

    # Find which EXT op is used
    ext_op = None
    nv_op = None
    for e, n in spirv_ops:
        if e in spirv_content:
            ext_op = e
            nv_op = n
            break

    if not ext_op:
        # No EXT op found, skip this block
        return match.group(0)

    # Create NV version
    nv_content = spirv_content.replace(ext_op, nv_op)
    nv_content = nv_content.replace('SPV_EXT_shader_invocation_reorder', 'SPV_NV_shader_invocation_reorder')
    nv_content = nv_content.replace('ShaderInvocationReorderEXT', 'ShaderInvocationReorderNV')

    # Build the dual case output
    result = f'{indent}case spirv_nv:\n{indent}    return spirv_asm\n{indent}    {{\n{nv_content}{indent}    }};\n'
    result += f'{indent}case spirv:\n{indent}    return spirv_asm\n{indent}    {{\n{spirv_content}{indent}    }};'

    return result

# Pattern to match spirv case blocks
# Matches: case spirv: return spirv_asm { ... OpHitObject...EXT ... };
pattern = r'(        )(case spirv:\n            return spirv_asm\n            \{\n)((?:.*?\n)*?)(        \};)'

def replacer(match):
    indent = match.group(1)
    spirv_header = match.group(2)
    spirv_content = match.group(3)
    spirv_footer = match.group(4)

    # Check if this block contains an EXT op we want to dual-ize
    has_ext_op = any(ext_op in spirv_content for ext_op, _ in spirv_ops)

    if not has_ext_op:
        # Skip this block
        return match.group(0)

    # Find the EXT op
    ext_op = None
    nv_op = None
    for e, n in spirv_ops:
        if e in spirv_content:
            ext_op = e
            nv_op = n
            break

    # Create NV version
    nv_content = spirv_content.replace(ext_op, nv_op)
    nv_content = nv_content.replace('SPV_EXT_shader_invocation_reorder', 'SPV_NV_shader_invocation_reorder')
    nv_content = nv_content.replace('ShaderInvocationReorderEXT', 'ShaderInvocationReorderNV')

    # Build dual cases
    result = f'{indent}case spirv_nv:\n{indent}    return spirv_asm\n{indent}    {{\n{nv_content}{indent}    }};\n'
    result += f'{indent}case spirv:\n{indent}    return spirv_asm\n{indent}    {{\n{spirv_content}{indent}    }};'

    return result

# Apply the transformation
content = re.sub(pattern, replacer, content, flags=re.DOTALL)

# Write back
with open('source/slang/hlsl.meta.slang', 'w', encoding='utf-8') as f:
    f.write(content)

print("Updated SPIRV blocks to support dual NV/EXT API")
print("Note: IsMiss() was already updated manually, so it may be duplicated - check the file")
