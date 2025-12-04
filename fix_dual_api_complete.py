#!/usr/bin/env python3
"""
Complete fix for dual-API support: NV (sub-targets) vs EXT (generic targets)
"""

import re
import sys

def fix_hlsl_meta_nv_targets():
    """
    Step 1: Update hlsl.meta.slang to use spirv_nv for existing NV SPIRV code
    """
    print("Step 1: Updating hlsl.meta.slang - wrapping NV SPIRV in spirv_nv...")

    with open('source/slang/hlsl.meta.slang', 'r', encoding='utf-8') as f:
        content = f.read()

    # Pattern: Find spirv cases with NV operations and change to spirv_nv
    # Look for: case spirv: { spirv_asm { ... OpHitObject*NV ... } }

    # Simple approach: replace "case spirv:" with "case spirv_nv:" only within
    # __target_switch blocks that contain NV operations

    lines = content.split('\n')
    result = []
    i = 0

    while i < len(lines):
        line = lines[i]

        # If we find "case spirv:" check if it's an NV operation
        if re.match(r'\s*case spirv:\s*$', line):
            # Look ahead to see if this block has NV operations
            look_ahead_start = i
            look_ahead_end = min(i + 50, len(lines))
            look_ahead_text = '\n'.join(lines[look_ahead_start:look_ahead_end])

            # Check for NV-specific operations
            has_nv_op = bool(re.search(r'Op(HitObject|ReorderThread)\w+NV', look_ahead_text))

            if has_nv_op:
                # Replace with spirv_nv
                line = line.replace('case spirv:', 'case spirv_nv:')
                print(f"  Line {i+1}: Changed case spirv: -> case spirv_nv:")

        result.append(line)
        i += 1

    content = '\n'.join(result)

    with open('source/slang/hlsl.meta.slang', 'w', encoding='utf-8') as f:
        f.write(content)

    print("  ✓ hlsl.meta.slang updated with spirv_nv targets\n")

def add_ext_variants_from_backup():
    """
    Step 2: Add EXT SPIRV variants using generic spirv target from backup
    """
    print("Step 2: Adding EXT SPIRV variants from backup...")

    # Read the backup which has both NV and EXT implementations
    with open('source/slang/hlsl.meta.slang.backup', 'r', encoding='utf-8') as f:
        backup_content = f.read()

    with open('source/slang/hlsl.meta.slang', 'r', encoding='utf-8') as f:
        current_content = f.read()

    # Extract EXT SPIRV blocks from backup (those using OpHitObject*EXT)
    # and add them to current content

    # Pattern: Find blocks like:
    #   case spirv:
    #   {
    #       spirv_asm
    #       {
    #           OpExtension "SPV_EXT_shader_invocation_reorder";
    #           OpCapability ShaderInvocationReorderEXT;
    #           OpHitObject*EXT ...
    #       };
    #   }

    backup_lines = backup_content.split('\n')
    current_lines = current_content.split('\n')

    # Find method boundaries in current file
    # Look for methods like "static HitObject TraceRay", "static HitObject MakeNop", etc.

    methods_to_update = [
        ('static HitObject TraceRay', 'TraceRay'),
        ('static HitObject TraceMotionRay', 'TraceMotionRay'),
        ('static HitObject MakeMiss', 'MakeMiss'),
        ('static HitObject MakeMotionMiss', 'MakeMotionMiss'),
        ('static HitObject MakeNop', 'MakeNop'),
        ('static void Invoke', 'Invoke'),
        ('bool IsMiss', 'IsMiss'),
        ('bool IsHit', 'IsHit'),
        ('bool IsNop', 'IsNop'),
    ]

    result_lines = list(current_lines)

    for method_signature, method_name in methods_to_update:
        print(f"  Processing {method_name}...")

        # Find the method in backup
        backup_method_start = None
        for i, line in enumerate(backup_lines):
            if method_signature in line:
                backup_method_start = i
                break

        if backup_method_start is None:
            continue

        # Find the EXT spirv case in backup (after the method start)
        ext_block_start = None
        for i in range(backup_method_start, min(backup_method_start + 200, len(backup_lines))):
            if 'case spirv:' in backup_lines[i]:
                # Check if this is an EXT block
                look_ahead = '\n'.join(backup_lines[i:min(i+30, len(backup_lines))])
                if 'SPV_EXT_shader_invocation_reorder' in look_ahead:
                    ext_block_start = i
                    break

        if ext_block_start is None:
            continue

        # Extract the EXT block from backup
        ext_block_lines = []
        brace_count = 0
        in_case = False
        for i in range(ext_block_start, len(backup_lines)):
            line = backup_lines[i]
            if 'case spirv:' in line:
                in_case = True
            if in_case:
                ext_block_lines.append(line)
                brace_count += line.count('{')
                brace_count -= line.count('}')
                if in_case and brace_count == 0 and ('}' in line):
                    break

        # Now find where to insert this in the current file
        # Find the method in current file
        current_method_start = None
        for i, line in enumerate(result_lines):
            if method_signature in line:
                current_method_start = i
                break

        if current_method_start is None:
            continue

        # Find the end of spirv_nv case in current file
        spirv_nv_end = None
        for i in range(current_method_start, min(current_method_start + 200, len(result_lines))):
            if 'case spirv_nv:' in result_lines[i]:
                # Find the closing brace of this case
                brace_count = 0
                for j in range(i, len(result_lines)):
                    brace_count += result_lines[j].count('{')
                    brace_count -= result_lines[j].count('}')
                    if brace_count == 0 and'}' in result_lines[j]:
                        spirv_nv_end = j
                        break
                break

        if spirv_nv_end is None:
            continue

        # Insert the EXT block after spirv_nv case
        result_lines = result_lines[:spirv_nv_end+1] + ext_block_lines + result_lines[spirv_nv_end+1:]
        print(f"    ✓ Added EXT variant for {method_name}")

    content = '\n'.join(result_lines)

    with open('source/slang/hlsl.meta.slang', 'w', encoding='utf-8') as f:
        f.write(content)

    print("  ✓ EXT variants added from backup\n")

def main():
    print("="*60)
    print("Dual-API Fix: NV (sub-targets) + EXT (generic targets)")
    print("="*60)
    print()

    try:
        fix_hlsl_meta_nv_targets()
        add_ext_variants_from_backup()

        print("="*60)
        print("✓ Dual-API implementation complete!")
        print("="*60)
        print()
        print("Next steps:")
        print("1. Build the project: cmake.exe --build --preset debug")
        print("2. Fix any remaining issues")
        print("3. Run tests")

    except Exception as e:
        print(f"\n✗ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()
