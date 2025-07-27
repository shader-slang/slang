# Design Document: Slang IR Module Backwards Compatibility

## Overview

This document describes the design and implementation of backwards compatibility support for serialized Slang IR modules. The feature enables Slang to load IR modules compiled with different versions of the compiler, providing version information and graceful handling of incompatible modules.

## Motivation

As Slang evolves, the intermediate representation (IR) may change with new instructions being added or existing ones being modified. Without backwards compatibility:

- Users cannot load modules compiled with older versions of Slang
- There's no way to detect version mismatches between modules
- Module compatibility issues are opaque to users

This feature addresses these issues by introducing versioning and stable instruction naming.

## User-Facing Changes

### New Command Line Options

1. **`-get-module-info <module-file>`**

   - Prints information about a serialized IR module without loading it
   - Output includes:
     - Module name
     - Module version
     - Compiler version that created the module
   - Example usage: `slangc -get-module-info mymodule.slang-module`

2. **`-get-supported-module-versions`**
   - Prints the range of module versions this compiler supports
   - Output includes minimum and maximum supported versions
   - Example usage: `slangc -get-supported-module-versions`

### API Changes

New method in `ISession` interface:

```cpp
SlangResult loadModuleInfoFromIRBlob(
    slang::IBlob* source,
    SlangInt& outModuleVersion,
    const char*& outModuleCompilerVersion,
    const char*& outModuleName);
```

This allows programmatic inspection of module metadata without full deserialization.

## Technical Design

### Stable Instruction Names

The core mechanism for backwards compatibility is the introduction of stable names for IR instructions:

1. **Stable Name Table** (`slang-ir-insts-stable-names.lua`)

   - Maps instruction names to unique integer IDs
   - IDs are permanent once assigned
   - New instructions get new IDs, never reusing old ones

2. **Runtime Mapping**
   - `getOpcodeStableName(IROp)`: Convert runtime opcode to stable ID
   - `getStableNameOpcode(UInt)`: Convert stable ID back to runtime opcode
   - Unknown stable IDs map to `kIROp_Unrecognized`

### Module Versioning

Two types of versions are tracked:

1. **Module Version** (`IRModule::m_version`)

   - Semantic version of the IR instruction set
   - Range: `k_minSupportedModuleVersion` to `k_maxSupportedModuleVersion`
   - Stored in each serialized module

2. **Serialization Version** (`IRModuleInfo::serializationVersion`)
   - Version of the serialization format itself
   - Currently version 0
   - Allows future changes to serialization structure

### Compiler Version Tracking

Each module stores the exact compiler version (`SLANG_TAG_VERSION`) that created it. This enables version-specific workarounds if needed in the future.

### Validation System

A GitHub Actions workflow (`check-ir-stable-names.yml`) ensures consistency:

1. **Check Mode**: Validates that:

   - All IR instructions have stable names
   - No duplicate stable IDs exist
   - The stable name table is a bijection with current instructions

2. **Update Mode**: Automatically assigns stable IDs to new instructions

The validation is implemented in `check-ir-stable-names.lua` which:

- Loads instruction definitions from `slang-ir-insts.lua`
- Compares against `slang-ir-insts-stable-names.lua`
- Reports missing entries or inconsistencies

## Breaking Changes and Version Management

### When to Update Module Version

The module version must be updated when:

1. **Adding Instructions** (Minor Version Bump)

   - Increment `k_maxSupportedModuleVersion`
   - Older compilers can still load modules that don't use new instructions

2. **Removing Instructions** (Major Version Bump)

   - Increment `k_maxSupportedModuleVersion`
   - Update `k_minSupportedModuleVersion` to exclude versions with removed instructions
   - This breaks compatibility with older modules using removed instructions

3. **Changing Instruction Semantics**
   - Even if the instruction name remains the same
   - Requires version bump to prevent incorrect behavior
   - To avoid bumping the minimum supported version, one may instead introduce
     a new instruction and just bump `k_maxSupportedModuleVersion`

### Serialization Format Changes

Changes to how data is serialized (not what data) require updating `serializationVersion`:

- Changes to the RIFF container structure
- Different encoding for instruction payloads
- Reordering of serialized data

## Implementation Details

### Module Loading Flow

1. **Version Check**

   ```cpp
   if (fossilizedModuleInfo->serializationVersion != IRModuleInfo::kSupportedSerializationVersion)
       return SLANG_FAIL;
   ```

2. **Instruction Deserialization**

   - Stable IDs are converted to runtime opcodes
   - Unknown IDs become `kIROp_Unrecognized`

3. **Validation Pass**
   - After deserialization, check for any `kIROp_Unrecognized` instructions
   - Fail loading if any are found

### Error Handling

- Incompatible serialization versions: Immediate failure
- Unknown instructions: Mark as unrecognized, fail after full deserialization
  (this should be caught by the next check)
- Module version out of range: Fail after deserialization

## Future Considerations

### Potential Enhancements

1. **Graceful Degradation**

   - Skip unrecognized instructions if they're not critical
   - Provide compatibility shims for removed instructions

2. **Module Migration Tools**

   - Utility to upgrade old modules to new formats
   - Batch processing for large codebases

### Maintenance Guidelines

1. **Regular CI Validation**

   - The GitHub Action ensures stable names stay synchronized
   - Catches missing entries before merge

2. **Version Documentation**

   - Maintain changelog of what changed in each module version
   - Document any version-specific workarounds

3. **Testing**
   - Test loading of modules from previous versions
   - Verify error messages for incompatible modules

## Conclusion

This backwards compatibility system provides a robust foundation for Slang IR evolution while maintaining compatibility where possible. The combination of stable instruction naming, comprehensive versioning, and automated validation ensures that:

- Users can reliably use modules across Slang versions
- Developers can evolve the IR with clear compatibility boundaries
- Version mismatches are detected and reported clearly

The system is designed to be maintainable and extensible, with clear guidelines for when and how to make breaking changes.
