#!/usr/bin/env python3
"""Fix [require(glsl_spirv, ...)] to use glsl_nv/spirv_nv for NV functions"""

import re

def fix_glsl_meta():
    with open('source/slang/glsl.meta.slang', 'r', encoding='utf-8') as f:
        content = f.read()

    # Pattern 1: [require(glsl_spirv, ser_raygen_closesthit_miss)]
    content = re.sub(
        r'\[require\(glsl_spirv, ser_raygen_closesthit_miss\)\]',
        '[require(glsl_nv, ser_raygen_closesthit_miss)]\n[require(spirv_nv, ser_raygen_closesthit_miss)]',
        content
    )

    # Pattern 2: [require(glsl_spirv, ser_motion_raygen_closesthit_miss)]
    content = re.sub(
        r'\[require\(glsl_spirv, ser_motion_raygen_closesthit_miss\)\]',
        '[require(glsl_nv, ser_motion_raygen_closesthit_miss)]\n[require(spirv_nv, ser_motion_raygen_closesthit_miss)]',
        content
    )

    # Pattern 3: [require(glsl_spirv, ser_raygen)]
    content = re.sub(
        r'\[require\(glsl_spirv, ser_raygen\)\]',
        '[require(glsl_nv, ser_raygen)]\n[require(spirv_nv, ser_raygen)]',
        content
    )

    with open('source/slang/glsl.meta.slang', 'w', encoding='utf-8') as f:
        f.write(content)

    print("✓ Updated require clauses in glsl.meta.slang")

if __name__ == '__main__':
    fix_glsl_meta()
