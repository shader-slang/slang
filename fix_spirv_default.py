#!/usr/bin/env python3
"""
Script to fix SPIRV dual API: make NV the default for backward compatibility.
Removes spirv_nv cases and keeps only spirv case with NV operations.
"""

import re

# Read the file
with open('source/slang/hlsl.meta.slang', 'r', encoding='utf-8') as f:
    content = f.read()

# Pattern to match spirv_nv and spirv cases together
# We want to remove spirv_nv and keep spirv, but change spirv to use NV operations
pattern = r'(        )case spirv_nv:\n((?:.*?\n)*?)(        case spirv:\n(?:.*?\n)*?            \};)'

def replacer(match):
    indent = match.group(1)
    spirv_nv_content = match.group(2)
    spirv_ext_case = match.group(3)

    # Extract just the spirv_asm block from spirv_nv
    # The spirv_nv content should be the NV operations we want to keep
    # We'll replace the entire spirv case with just the NV operations

    # Build new spirv case with NV operations
    result = f'{indent}case spirv:\n{spirv_nv_content}'

    return result

# Apply the transformation
content = re.sub(pattern, replacer, content, flags=re.DOTALL)

# Write back
with open('source/slang/hlsl.meta.slang', 'w', encoding='utf-8') as f:
    f.write(content)

print("Fixed SPIRV default to use NV operations")
print("Removed spirv_nv cases, kept spirv with NV operations")
