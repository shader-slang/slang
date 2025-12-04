#!/usr/bin/env python3
"""
Script to add -capability spirv_nv to SPIRV tests in shader-execution-reordering
"""

import os
import re

test_files = [
    'hit-object-get-object-ray.slang',
    'hit-object-make-miss.slang',
    'hit-object-make-nop.slang',
    'hit-object-make-hit.slang',
    'hit-object-output.slang',
    'hit-object-reorder-thread.slang',
    'hit-object-trace-ray.slang',
    'hit-object-trace-motion-ray.slang',
]

test_dir = 'tests/hlsl-intrinsic/shader-execution-reordering'

for test_file in test_files:
    filepath = os.path.join(test_dir, test_file)

    if not os.path.exists(filepath):
        print(f"Warning: {filepath} not found")
        continue

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Add -capability spirv_nv to lines that have:
    # -target spirv ... -O0 (but don't already have -capability)
    def add_capability(match):
        line = match.group(0)
        if '-capability' in line:
            return line  # Already has capability
        # Add capability before -O0
        return line.replace(' -O0', ' -O0 -capability spirv_nv')

    # Pattern: lines with -target spirv and -O0
    pattern = r'//TEST:SIMPLE\(filecheck=SPIRV\):.*-target spirv.*-O0[^\n]*'
    content = re.sub(pattern, add_capability, content)

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)

    print(f"Updated: {test_file}")

print("\nDone! All tests updated.")
