# YAML Schema Description for IR Instruction definitions

## Overview

This YAML format represents a hierarchical collection of instructions and instruction ranges, optimized for human readability and editing.

## Root Structure

The root of the YAML file contains a single key:

- **`insts`**: An array of instruction and range entries

## Entry Types

Each entry in the `insts` array is a single-key mapping where the key identifies the entry type:

### 1. Instruction Entry

An instruction entry has the **mnemonic** as its key, with the value being either:

- An empty mapping `{}` for simple instructions
- A mapping containing instruction properties

**Properties** (all optional):
| Field | Type | Description | Default value |
|-------|------|-------------|--------------|
| `comment` | string or literal block | Documentation for the instruction | `""` |
| `type_name` | string | Original name if not derivable from mnemonic | The PascalCase version of the instruction key |
| `operands` | integer | Number of operands | 0 |
| `flags` | array of strings | Instruction flags in CamelCase | `[]` |

**Example:**

```yaml
- nop: {} # Simple instruction with no properties
- int:
    flags: [Hoistable]
- special_op:
    comment: |
      This is a special operation
      with multi-line documentation
    type_name: SpecialOperation
    operands: 1
```

### 2. Range Entry

A range entry has the **range name** as its key, containing grouped instructions and sub-ranges.

**Properties**:
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `comment` | string or literal block | No | Documentation for the range |
| `flags` | array of strings | No | Common flags shared by all instructions in the range |
| `insts` | array | No | Ordered list of instructions and sub-ranges |

**Example:**

```yaml
- Arithmetic:
    comment: Basic arithmetic operations
    flags: [Pure] # Common to all contained instructions
    insts:
      - add:
          operands: 2
      - sub:
          operands: 2
      - Multiplication: # Nested range
          insts:
            - mul:
                operands: 2
            - imul:
                operands: 2
```

## Key Features

### 1. Mnemonic-based Keys

- Instructions are keyed by their mnemonic (display string)
- The compiler internal names are preserved via `type_name` only when necessary

### 2. Name Preservation Rules

The `type_name` field is omitted when the original name can be recovered from the mnemonic by:

- Replacing a `_t` suffix with `_type`
- Splitting the name on `_`
- Uppercasing the first character in each element
- Concatenating the results

### 3. Minimal Representation

- Empty `flags` arrays are omitted
- Zero `operands` values are omitted
- Empty instruction definitions become `{}`
- Comments matching instruction/range names are cleaned

### 4. Flag Hoisting

When all instructions in a range share the same flags:

- Flags can be specified on the parent range
- Individual instruction flags are the union of the instruction flags and all enclosing range flags

### 5. Ordering

- Instructions are placed in sequences to preserve ordering
- Instructions and ranges are interleaved
