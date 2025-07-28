# Slang IR Instruction Management and Versioning

This document explains how Slang's intermediate representation (IR) instructions are defined, generated, and versioned. It covers the workflow for adding or modifying instructions and the mechanisms that ensure backwards compatibility for serialized IR modules.

## High-Level Concepts

The Slang IR uses a code generation approach where instruction definitions are centralized in a Lua file (`slang-ir-insts.lua`), and various C++ headers and source files are generated from this single source of truth. This ensures consistency across the codebase and enables sophisticated features like backwards compatibility through stable instruction naming.

### Key Components

- **Instruction Definitions** (`slang-ir-insts.lua`): The canonical source for all IR instruction definitions
- **Stable Names** (`slang-ir-insts-stable-names.lua`): Maps instruction names to permanent integer IDs for backwards compatibility
- **Code Generation** (via Fiddle): Generates C++ enums, structs, and tables from the Lua definitions
- **Module Versioning**: Tracks compatibility ranges for serialized IR modules

## The Instruction Definition System

### Source of Truth: `slang-ir-insts.lua`

All IR instructions are defined in `source/slang/slang-ir-insts.lua`. This file contains a hierarchical table structure that defines:

- Instruction names and their organization into categories
- Struct names for the C++ representation (if different from the default)
- Flags like `hoistable`, `parent`, `global`, etc.
- (Optionally) Minimum operand counts
- (Optionally) The operands themselves
- Parent-child relationships in the instruction hierarchy

Here's a simplified example of how instructions are defined:

```lua
local insts = {
    { nop = {} },
    {
        Type = {
            {
                BasicType = {
                    hoistable = true,
                    { Void = { struct_name = "VoidType" } },
                    { Bool = { struct_name = "BoolType" } },
                    { Int = { struct_name = "IntType" } },
                    -- ... more basic types
                },
            },
            -- ... more type categories
        },
    },
    -- ... more instruction categories
}
```

The hierarchy is important: instructions inherit properties from their parent categories. For example, all `BasicType` instructions inherit the `hoistable = true` flag.

### Code Generation Flow

The Fiddle tool processes `slang-ir-insts.lua` and generates several outputs:

1. **Enum Definitions** (`slang-ir-insts-enum.h`):

   - `IROp` enum with values like `kIROp_Void`, `kIROp_Bool`, etc.
   - Range markers like `kIROp_FirstBasicType` and `kIROp_LastBasicType`

2. **Struct Definitions** (`slang-ir-insts.h`):

   - C++ struct definitions for instruction types not manually defined
   - `leafInst()` and `baseInst()` macros for RTTI support
   - If operands of an IR are specified in `slang-ir-insts.lua` in the format `{ { "operand1_name", "operand1_type" }, {"operand2_name"} }` and so on,
     Fiddle will generate getters for each of the operands as part of the IR's struct. Note that the order in which the operands are listed matters and
     specification of the type of the operand is optional; defaulting to "IRInst" when the type is not specified.

3. **Instruction Info Table** (`slang-ir-insts-info.cpp`):

   - Maps opcodes to their string names, operand counts, and flags
   - Used for debugging, printing, and validation

4. **Stable Name Mappings** (`slang-ir-insts-stable-names.cpp`):
   - Bidirectional mapping between opcodes and stable IDs
   - Critical for backwards compatibility

## Adding or Modifying Instructions

### Adding a New Instruction

To add a new IR instruction:

1. **Edit `slang-ir-insts.lua`**: Add your instruction in the appropriate category:

   ```lua
   { MyNewInst = { min_operands = 2 } },
   ```

2. **Run the build**: The build system will automatically regenerate the C++ files.

3. **Update the stable names**: Either

   - Run the validation script:

     **Note**: Skip make command if lua is already built.
     ```bash
     make -C external/lua MYCFLAGS="-DLUA_USE_POSIX" MYLIBS=""
     ./external/lua/lua extras/check-ir-stable-names.lua update
     ```

   - Or add a new ID to the mapping in `source/slang/slang-ir-insts-stable-names.lua`, this is checked for consistency in CI so it's safe to add manually.

   This assigns a permanent ID to your new instruction.

4. **Implement the instruction logic**: Add handling in relevant files like:

   - `slang-ir-insts.h` (if you need a custom struct definition)
   - `slang-emit-*.cpp` files for code generation
   - `slang-ir-lower-*.cpp` files for transformations

5. **Update the module version**: In `slang-ir.h`, increment `k_maxSupportedModuleVersion`:
   ```cpp
   const static UInt k_maxSupportedModuleVersion = 1; // was 0
   ```

### Modifying an Existing Instruction

Modifications require more care:

- **Adding operands or changing semantics**: This is a breaking change. You must:

  1. Increment both `k_minSupportedModuleVersion` and `k_maxSupportedModuleVersion`
  2. Document the change in the version history

- **Renaming**: Don't rename instructions directly. Instead:

  1. Add the new instruction
  2. Mark the old one as deprecated
  3. Eventually remove it in a major version bump

## The Stable Name System

### Purpose

When Slang serializes IR modules, it needs to handle the case where the compiler version that reads a module is different from the one that wrote it. Instructions might have been added, removed, or reordered in the `IROp` enum.

The stable name system solves this by assigning permanent integer IDs to each instruction. These IDs never change once assigned.

### How It Works

1. **Assignment**: When a new instruction is added, the `check-ir-stable-names.lua` script assigns it the next available ID.

2. **Serialization**: When writing a module, opcodes are converted to stable IDs:

   ```cpp
   auto stableName = getOpcodeStableName(value);
   ```

3. **Deserialization**: When reading, stable IDs are converted back:

   ```cpp
   value = getStableNameOpcode(stableName);
   ```

4. **Validation**: The CI system ensures the stable name table stays synchronized with the instruction definitions.

### Maintenance

The stable name table is validated in CI:

```bash
./extras/check-ir-stable-names-gh-actions.sh
```

This script:

- Verifies all instructions have stable names
- Checks for duplicate IDs
- Ensures the mapping is bijective
- Can automatically fix missing entries

## Module Versioning

### Version Types

Slang tracks two version numbers:

1. **Module Version** (`IRModule::m_version`): The semantic version of the IR instruction set

   - Range: `k_minSupportedModuleVersion` to `k_maxSupportedModuleVersion`
   - Stored in each serialized module

2. **Serialization Version** (`IRModuleInfo::serializationVersion`): The format version
   - Allows changes to how data is encoded

### When to Update Versions

**Minor Version Bump** (increment `k_maxSupportedModuleVersion` only):

- Adding new instructions
- Adding new instruction flags that don't affect existing code
- Adding new optional operands

**Major Version Bump** (increment both min and max):

- Removing instructions
- Changing instruction semantics
- Modifying minimum operand counts or types
- Any change that breaks compatibility

### Version Checking

During deserialization:

```cpp
if (fossilizedModuleInfo->serializationVersion != IRModuleInfo::kSupportedSerializationVersion)
    return SLANG_FAIL;

// Later, after loading instructions:
if (hasUnrecognizedInsts)
    return SLANG_FAIL;
```

## Serialization Details

### The Flat Representation

For efficiency, IR modules are serialized as a "flat" representation:

```cpp
struct FlatInstTable
{
    List<InstAllocInfo> instAllocInfo;  // Op + operand count
    List<Int64> childCounts;            // Children per instruction
    List<SourceLoc> sourceLocs;         // Source locations
    List<Int64> operandIndices;         // Flattened operand references
    List<Int64> stringLengths;          // For string/blob constants
    List<uint8_t> stringChars;          // Concatenated string data
    List<UInt64> literals;              // Integer/float constant values
};
```

This representation:

- Minimizes pointer chasing during deserialization
- Groups similar data together for better cache performance
- Enables efficient bulk operations

### Traversal Order

Instructions are serialized in a specific order for performance:

```cpp
traverseInstsInSerializationOrder(moduleInst, [&](IRInst* inst) {
    // Process instruction
});
```

The traversal:

1. Visits instructions in preorder (parent before children)
2. Optionally reorders module-level instructions to group constants together
3. Maintains deterministic ordering for reproducible builds

## Debugging and Validation

### Available Tools

1. **Module Info Inspection**:

   ```bash
   slangc -get-module-info module.slang-module
   ```

   Shows module name, version, and compiler version.

2. **Version Query**:

   ```bash
   slangc -get-supported-module-versions
   ```

   Reports the supported version range.

3. **IR Dumping**:
   ```bash
   slangc -dump-ir module.slang
   ```
   Shows the IR in human-readable form.

### Common Issues

**"Unrecognized instruction" errors**: The module contains instructions unknown to this compiler version. Update Slang or recompile the module.

**Stable name validation failures**: Run the update script and commit the changes:

**Note**: Skip make command if lua is already built.
```bash
make -C external/lua MYCFLAGS="-DLUA_USE_POSIX" MYLIBS=""
./external/lua/lua extras/check-ir-stable-names.lua update
```

**Version mismatch**: The module was compiled with an incompatible Slang version. Check the version ranges and recompile if necessary.

## Best Practices

1. **Always update stable names**: After adding instructions, run the validation script before committing.

2. **Document version changes**: When bumping module versions, add a comment explaining what changed.

3. **Prefer addition over modification**: When possible, add new instructions rather than changing existing ones.

4. **Group related changes**: If making multiple breaking changes, do them together in a single version bump.
