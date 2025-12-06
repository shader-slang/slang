# Slang IR Dump Processing Tools

Tools to help process and analyze Slang IR dump files for easier understanding by LLMs and humans.

## Problem

When debugging Slang compiler issues, `-dump-ir` output is invaluable but difficult to work with:

1. Dump files are huge (thousands of lines) - LLMs struggle to read the entire file
2. Multiple compilation stages are mixed together (70+ sections)
3. Hard to track how IR transforms through the pipeline
4. Difficult to identify which pass introduced a problem

## Solution

`split-ir-dump.py` splits a monolithic IR dump into separate files, one per compilation stage.

## Quick Start

### 1. Generate and Split IR Dump

```bash
# Method 1: Pipe directly to split script (recommended)
build/Debug/bin/slangc.exe -dump-ir -target spirv-asm -o tmp.spv tests/your-test.slang | python extras/split-ir-dump.py

# Method 2: Save to file first, then split
build/Debug/bin/slangc.exe -dump-ir -target spirv-asm -o tmp.spv tests/your-test.slang > test.dump
python extras/split-ir-dump.py test.dump

# Method 3: Redirect from file
python extras/split-ir-dump.py < test.dump

# For CUDA target
build/Debug/bin/slangc.exe -dump-ir -target cuda -o tmp.cu tests/your-test.slang | python extras/split-ir-dump.py
```

**Important**: When using `-dump-ir` with targets like `spirv-asm`, always use `-o <file>` to redirect the compiled output. Otherwise, the SPIRV/CUDA code will be mixed with the IR dump on stdout.

### 2. Output Structure

This creates a directory `dump-000/` (or `dump-001/`, etc.) containing:

- `000-INDEX.txt` - Overview of all sections with line counts
- `001-LOWER-TO-IR.txt` - Initial IR generation
- `002-AFTER-fixEntryPointCallsites.txt` - After first pass
- `003-AFTER-replaceGlobalConstants.txt` - After second pass
- ... (one file per compilation stage)

### 3. Analyze with LLM

You can now ask an LLM to read specific sections:

```
Read dump-000/032-AFTER-lowerGenerics.txt and explain what changed
```

Or compare sections:

```
Compare dump-000/031-AFTER-checkGetStringHashInsts.txt
with dump-000/032-AFTER-lowerGenerics.txt and show what changed
```

## Output Format

Each section file contains:

```
# Section: AFTER-lowerGenerics:
# Original section number: 32
#======================================================================

[IR content here...]
```

The index file (`000-INDEX.txt`) provides a quick overview:

```
IR Dump Section Index
======================================================================
Source file: test.dump
Total sections: 72

Sections:
----------------------------------------------------------------------
  1. 001-LOWER-TO-IR.txt                                (   33 lines)
  2. 002-AFTER-fixEntryPointCallsites.txt               (   60 lines)
  ...
```

## Directory Naming

The script automatically finds the next available directory:
- First run: `dump-000/`
- Second run: `dump-001/`
- Third run: `dump-002/`
- etc.

This prevents overwriting previous analyses.

## Common Workflows

### Find Which Pass Broke Something

1. Generate and split dump: `slangc.exe -dump-ir -target spirv-asm -o tmp.spv test.slang | python extras/split-ir-dump.py`
2. Use binary search through sections to find where IR becomes invalid
3. Read the section before and after to identify the problematic pass

### Understand a Specific Pass

1. Find the pass in the index: look for `AFTER-<pass-name>`
2. Read the section before the pass
3. Read the section after the pass
4. Compare to see what changed

### Track an IR Value

1. Find where the value is created (search in `001-LOWER-TO-IR.txt`)
2. Follow it through subsequent sections
3. See how it gets transformed, specialized, or optimized

## Tips

### Enable More Detailed Dumps

The Slang compiler has some IR dump code wrapped in `#if 0 / #endif` blocks.
To get more detailed dumps, temporarily enable them:

```cpp
// In source/slang/slang-ir-*.cpp files
#if 0  // Change to #if 1
    dumpIRModule(module, "detailed-state");
#endif
```

### Focus on Specific Passes

If you know which pass is problematic:

```bash
# Search for the pass in the index
grep -i "lowerGenerics" dump-000/000-INDEX.txt

# Read just that section
cat dump-000/032-AFTER-lowerGenerics.txt
```

### Use with InstTrace

When you find a problematic instruction with `_debugUID`, combine with InstTrace:

```bash
# Find the instruction in split dumps
grep "_debugUID=1234" dump-000/*.txt

# Trace where it was created
python3 ./extras/insttrace.py 1234 ./build/Debug/bin/slangc tests/my-test.slang -target spirv
```

## Advanced: Reading Dumps Without GPU

If you need IR dumps but don't have GPU access:

```bash
# CPU compute target
slangc.exe -dump-ir -target cpp -o test.cpp test.slang | python extras/split-ir-dump.py

# Or just stop early (may not show all passes)
slangc.exe -dump-ir test.slang | python extras/split-ir-dump.py
```

## File Naming Convention

Section files are named: `NNN-SECTION-NAME.txt`

- `NNN` = 3-digit sequence number (001, 002, 003, ...)
- `SECTION-NAME` = sanitized section name
  - Spaces → hyphens
  - Special chars → hyphens
  - `AFTER fixEntryPointCallsites:` → `AFTER-fixEntryPointCallsites`

## Requirements

- Python 3.6+
- No external dependencies (uses only standard library)

## Troubleshooting

### "Dump file not found"

Make sure you're passing the correct path to the dump file.

### No sections found

Your dump might be incomplete. Try:
1. Adding `-target spirv-asm` to see full pipeline
2. Checking if compilation succeeded
3. Verifying the dump file isn't empty

### Directory already exists

The script automatically increments: `dump-000`, `dump-001`, etc.
If you want to overwrite, manually delete the old directory first.

## Related Tools

- `extras/insttrace.py` - Trace where an IR instruction was created
- `slangc -dump-ir` - Generate IR dumps (built into compiler)
- `dumpIRToString()` - Dump IR programmatically from C++ code

## Example Session

```bash
# Method 1: Direct pipe (recommended)
$ build/Debug/bin/slangc.exe -dump-ir -target spirv-asm -o tmp.spv tests/render/check-backend-support-on-ci.slang | python extras/split-ir-dump.py
Creating directory: dump-000
Found 72 sections
    1. 001-LOWER-TO-IR.txt                                (  1458 chars)
    2. 002-AFTER-fixEntryPointCallsites.txt               (  2463 chars)
    ...
   72. 072-AFTER-checkUnsupportedInst.txt                 (  2025 chars)

Index created: dump-000\000-INDEX.txt

All sections extracted to: dump-000/

# Method 2: From file
$ build/Debug/bin/slangc.exe -dump-ir -target spirv-asm -o tmp.spv test.slang > test.dump
$ python extras/split-ir-dump.py test.dump
Creating directory: dump-001
...

# Check the index
$ cat dump-000/000-INDEX.txt
IR Dump Section Index
======================================================================
Source: stdin
Total sections: 72

# Read a specific section
$ cat dump-000/032-AFTER-lowerGenerics.txt
```
