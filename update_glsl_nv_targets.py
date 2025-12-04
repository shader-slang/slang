#!/usr/bin/env python3
"""
Script to update glsl.meta.slang to use glsl_nv/spirv_nv targets for NV functions
"""

import re

def update_glsl_meta():
    with open('source/slang/glsl.meta.slang', 'r', encoding='utf-8') as f:
        content = f.read()

    # Pattern 1: Simple case glsl: -> case glsl_nv:
    content = re.sub(
        r'(\s+__target_switch\s*\{\s*)case glsl:',
        r'\1case glsl_nv:',
        content
    )

    # Pattern 2: case spirv: with OpHitObject*NV -> case spirv_nv: with extensions
    # This is more complex, need to find spirv blocks that use NV operations

    # Find all spirv_asm blocks with OpHitObject*NV or OpReorderThread*NV
    pattern = r'(case spirv:\s*\{\s*spirv_asm\s*\{)(\s*)(Op(?:HitObject|ReorderThread)\w+NV)'

    def add_extensions(match):
        indent = match.group(2)
        # Check if extensions already exist
        full_block = match.group(0)
        if 'OpExtension' in full_block:
            # Just change case spirv: to case spirv_nv:
            return match.group(0).replace('case spirv:', 'case spirv_nv:')
        else:
            # Add extensions and change target
            return (f'case spirv_nv:\n'
                   f'    {{\n'
                   f'        spirv_asm\n'
                   f'        {{\n'
                   f'{indent}OpExtension "SPV_NV_shader_invocation_reorder";\n'
                   f'{indent}OpCapability ShaderInvocationReorderNV;\n'
                   f'{indent}{match.group(3)}')

    # This regex approach is complex, let's do it differently
    # Split by function and process each

    lines = content.split('\n')
    result_lines = []
    i = 0
    in_target_switch = False
    in_spirv_case = False
    in_spirv_asm = False
    needs_nv_extension = False

    while i < len(lines):
        line = lines[i]

        # Detect __target_switch
        if '__target_switch' in line:
            in_target_switch = True

        # Change case spirv: to case spirv_nv: within NV functions
        if in_target_switch and re.match(r'\s*case spirv:\s*$', line):
            in_spirv_case = True
            # Check if next lines have NV operations
            # Look ahead to see if this is an NV operation block
            look_ahead = '\n'.join(lines[i:min(i+30, len(lines))])
            if 'OpHitObject' in look_ahead and 'NV' in look_ahead:
                needs_nv_extension = True
                line = line.replace('case spirv:', 'case spirv_nv:')

        # Detect spirv_asm block
        if in_spirv_case and 'spirv_asm' in line:
            in_spirv_asm = True
            result_lines.append(line)
            i += 1
            # Next line should be opening brace
            if i < len(lines) and '{' in lines[i]:
                result_lines.append(lines[i])
                i += 1
                # Add extensions if needed and not already present
                if needs_nv_extension:
                    # Check if extensions already exist
                    if i < len(lines) and 'OpExtension' not in lines[i]:
                        # Get indentation from next line
                        next_line = lines[i] if i < len(lines) else ''
                        indent_match = re.match(r'(\s*)', next_line)
                        indent = indent_match.group(1) if indent_match else '            '
                        result_lines.append(f'{indent}OpExtension "SPV_NV_shader_invocation_reorder";')
                        result_lines.append(f'{indent}OpCapability ShaderInvocationReorderNV;')
                needs_nv_extension = False
            continue

        # Detect end of target_switch
        if in_target_switch and line.strip() == '}' and not in_spirv_asm:
            # Could be end of target switch
            # Look back to see if we're closing target_switch
            if i > 0 and '}' in lines[i-1]:
                in_target_switch = False
                in_spirv_case = False

        # Detect end of spirv_asm
        if in_spirv_asm and '};' in line:
            in_spirv_asm = False
            in_spirv_case = False

        result_lines.append(line)
        i += 1

    content = '\n'.join(result_lines)

    with open('source/slang/glsl.meta.slang', 'w', encoding='utf-8') as f:
        f.write(content)

    print("Updated glsl.meta.slang with glsl_nv/spirv_nv targets")

if __name__ == '__main__':
    update_glsl_meta()
