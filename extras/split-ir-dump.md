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

## Slangc IR Dump Options

The Slang compiler provides several options for dumping IR:

### `-dump-ir`
Dumps IR after every compilation pass. This is the most comprehensive option and generates output for all passes in the pipeline.

```bash
slangc.exe -dump-ir -target spirv-asm -o tmp.spv test.slang | python extras/split-ir-dump.py
```

### `-dump-ir-before <pass-name>`
Dumps IR only before the specified pass. Can be specified multiple times for different passes.

```bash
# Dump IR before the lowerGenerics pass
slangc.exe -dump-ir-before lowerGenerics -target spirv-asm -o tmp.spv test.slang > before.dump

# Dump before multiple passes (useful with split script)
slangc.exe -dump-ir-before lowerGenerics -dump-ir-before eliminateDeadCode -target spirv-asm -o tmp.spv test.slang | python extras/split-ir-dump.py
```

### `-dump-ir-after <pass-name>`
Dumps IR only after the specified pass. Can be specified multiple times for different passes.

```bash
# Dump IR after the lowerGenerics pass
slangc.exe -dump-ir-after lowerGenerics -target spirv-asm -o tmp.spv test.slang > after.dump

# Dump after multiple passes (useful with split script)
slangc.exe -dump-ir-after lowerGenerics -dump-ir-after eliminateDeadCode -target spirv-asm -o tmp.spv test.slang | python extras/split-ir-dump.py
```

### Combining Options
You can combine `-dump-ir-before` and `-dump-ir-after` to dump IR around specific passes:

```bash
# Dump IR before and after lowerGenerics to see exactly what it does
# For just 2 sections, save directly to file instead of using split script
slangc.exe -dump-ir-before lowerGenerics -dump-ir-after lowerGenerics -target spirv-asm -o tmp.spv test.slang > lowerGenerics.dump

# For multiple passes, use split script
slangc.exe -dump-ir-before lowerGenerics -dump-ir-after lowerGenerics -dump-ir-before eliminateDeadCode -dump-ir-after eliminateDeadCode -target spirv-asm -o tmp.spv test.slang | python extras/split-ir-dump.py
```

**Tip**: Use selective dumping with `-dump-ir-before` and `-dump-ir-after` when you already know which pass is problematic. For just 1-2 passes, save directly to a file. For multiple passes, pipe to `split-ir-dump.py`.

## Common Workflows

### Find Which Pass Broke Something

1. Generate and split dump: `slangc.exe -dump-ir -target spirv-asm -o tmp.spv test.slang | python extras/split-ir-dump.py`
2. Use binary search through sections to find where IR becomes invalid
3. Once you identify the problematic pass, use selective dumping to focus on it:
   ```bash
   slangc.exe -dump-ir-before <pass-name> -dump-ir-after <pass-name> -target spirv-asm -o tmp.spv test.slang > pass.dump
   ```
4. Manually examine the dump to see BEFORE and AFTER sections

### Understand a Specific Pass

If you already know which pass to investigate:

1. Use selective dumping to see only that pass:
   ```bash
   slangc.exe -dump-ir-before lowerGenerics -dump-ir-after lowerGenerics -target spirv-asm -o tmp.spv test.slang > lowerGenerics.dump
   ```
2. The dump contains only 2 sections: BEFORE and AFTER the pass
3. Search for `### BEFORE` and `### AFTER` to compare the IR before and after the pass

### Track an IR Value

1. Find where the value is created (search in `001-LOWER-TO-IR.txt`)
2. Follow it through subsequent sections
3. See how it gets transformed, specialized, or optimized

### Focus on Specific Passes (Alternative Approach)

If you've already generated a full dump and want to find specific passes:

```bash
# Search for the pass in the index
grep -i "lowerGenerics" dump-000/000-INDEX.txt

# Read just that section
cat dump-000/032-AFTER-lowerGenerics.txt
```

**Better approach**: Use selective dumping from the start:
```bash
slangc.exe -dump-ir-before lowerGenerics -dump-ir-after lowerGenerics -target spirv-asm -o tmp.spv test.slang > lowerGenerics.dump
```

### Use with InstTrace

When you find a problematic instruction with `_debugUID`, combine with InstTrace:

```bash
# Find the instruction in split dumps
grep "_debugUID=1234" dump-000/*.txt

# Trace where it was created
python3 ./extras/insttrace.py 1234 ./build/Debug/bin/slangc tests/my-test.slang -target spirv
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
Results saved to: dump-000/

# Method 2: From file
$ build/Debug/bin/slangc.exe -dump-ir -target spirv-asm -o tmp.spv test.slang > test.dump
$ python extras/split-ir-dump.py test.dump
Results saved to: dump-001/

# Check the index
$ cat dump-000/000-INDEX.txt
IR Dump Section Index
======================================================================
Source: stdin
Total sections: 72

Sections:
----------------------------------------------------------------------
  1. 001-LOWER-TO-IR.txt                                (   33 lines)
  2. 002-AFTER-fixEntryPointCallsites.txt               (   60 lines)
  ...
 72. 072-AFTER-checkUnsupportedInst.txt                 (   50 lines)

# Read a specific section
$ cat dump-000/032-AFTER-lowerGenerics.txt
```
