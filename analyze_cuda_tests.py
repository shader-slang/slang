#!/usr/bin/env python3

import os
import re
from pathlib import Path

def analyze_test_file(file_path):
    """Analyze a single test file for CUDA configuration"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        lines = content.split('\n')
        cuda_tests = []
        disabled_cuda_tests = []
        other_tests = []
        
        # Look for test configurations in the first 20 lines (usually at the top)
        for line in lines[:20]:
            line = line.strip()
            if not line.startswith('//'):
                continue
                
            # Check for CUDA tests
            if '-cuda' in line:
                if 'TEST_DISABLED' in line or 'DISABLE_TEST' in line:
                    disabled_cuda_tests.append(line)
                elif '//TEST(' in line:
                    cuda_tests.append(line)
            elif '//TEST(' in line:
                other_tests.append(line)
        
        # Determine category
        if cuda_tests:
            category = 1  # Has enabled CUDA tests
        elif disabled_cuda_tests:
            category = 2  # Has disabled CUDA tests
        else:
            category = 3  # No CUDA tests found
            
        return {
            'file': file_path.name,
            'category': category,
            'cuda_tests': cuda_tests,
            'disabled_cuda_tests': disabled_cuda_tests,
            'other_tests': other_tests
        }
    except Exception as e:
        print(f"Error analyzing {file_path}: {e}")
        return None

def main():
    compute_dir = Path('/home/runner/work/slang/slang/tests/compute')
    
    # Find all .slang files
    slang_files = list(compute_dir.glob('*.slang'))
    
    categories = {1: [], 2: [], 3: []}
    
    print(f"Found {len(slang_files)} .slang files in tests/compute")
    print("=" * 80)
    
    for file_path in sorted(slang_files):
        result = analyze_test_file(file_path)
        if result:
            categories[result['category']].append(result)
    
    # Print results
    print(f"\n### CATEGORY 1: Tests with CUDA already enabled ({len(categories[1])} tests)")
    print("(These will be SKIPPED as requested)")
    for test in categories[1]:
        print(f"  - {test['file']}")
        for cuda_test in test['cuda_tests']:
            if '-cuda' in cuda_test:
                print(f"    {cuda_test}")
    
    print(f"\n### CATEGORY 2: Tests with CUDA disabled ({len(categories[2])} tests)")
    print("(These will be ENABLED and RUN)")
    for test in categories[2]:
        print(f"  - {test['file']}")
        for disabled_test in test['disabled_cuda_tests']:
            print(f"    {disabled_test}")
    
    print(f"\n### CATEGORY 3: Tests without CUDA ({len(categories[3])} tests)")
    print("(These will have CUDA ADDED and RUN)")
    for test in categories[3]:
        print(f"  - {test['file']}")
        # Show first few test lines to understand what targets they have
        if test['other_tests']:
            print(f"    Current tests: {test['other_tests'][0]}")
    
    print(f"\n### SUMMARY:")
    print(f"Category 1 (Skip): {len(categories[1])} tests")
    print(f"Category 2 (Enable): {len(categories[2])} tests") 
    print(f"Category 3 (Add CUDA): {len(categories[3])} tests")
    print(f"Total tests to run: {len(categories[2]) + len(categories[3])}")
    
    return categories

if __name__ == "__main__":
    categories = main()