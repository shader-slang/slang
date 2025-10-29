# Nested Parameter Block Reflection

## Overview

Nested parameter blocks (e.g., `ParameterBlock<ParameterBlock<T>>`) maintain a **unified reflection structure** across all targets, while the physical layout varies per-target.

## Reflection Behavior

### Unified Logical Structure

Both Metal and SPIRV (and all targets) show the same structure in reflection:

```json
{
    "kind": "parameterBlock",
    "elementType": {
        "kind": "parameterBlock",
        "elementType": {
            "kind": "scalar",
            "scalarType": "float32"
        }
    }
}
```

This maintains the original source-level nested structure for cross-platform RHI usage.

### Target-Specific Physical Layout

While reflection is unified, the generated shader code differs:

**Metal:**
```metal
struct wrapper_ParameterBlock { float constant* inner; };
// Parameter: wrapper_ParameterBlock constant* pb [[buffer(0)]]
// Access: *pb->inner
```

**SPIRV/HLSL:**
```hlsl
cbuffer pb { float value; }
// Flattened to innermost element type
// Access: value
```

## Implementation Details

### Order of Operations

1. **Parameter Binding** (`generateParameterBindings`): Creates type layouts based on source structure
2. **Type Layout Creation** (`createParameterGroupTypeLayout`): Records original nested parameter block types
3. **Legalization** (`legalizeResourceTypes`):
   - **Non-Metal**: Uses `implicitDeref` to recursively unwrap nested parameter blocks
   - **Metal**: Preserves structure for later wrapping
4. **Wrapping** (`wrapCBufferElementsForMetal`): Metal-only, wraps each nesting level in a struct

### Key Files

- `source/slang/slang-parameter-binding.cpp`: Generates layouts (line 4206)
- `source/slang/slang-type-layout.cpp`: Creates parameter group layouts (line 4774)
- `source/slang/slang-legalize-types.cpp`: Implements `implicitDeref` unwrapping (line 782)
- `source/slang/slang-ir-wrap-cbuffer-element.cpp`: Wraps for Metal (line 28)
- `source/slang/slang-emit.cpp`: Applies transformations (lines 1360, 1476)

## RHI Integration

The RHI layer **uses reflection** to navigate parameter blocks, not the physical types. This means:

1. RHI queries `getParameterCount()`, `getParameterByIndex()`, `getElementType()` from reflection
2. Reflection returns the logical nested structure
3. RHI navigates using `setObject()` and `getDereferenced()` on the shader cursor
4. The compiler's physical layout (wrapped or flattened) is transparent to the RHI

### Example RHI Usage

```cpp
// Works identically for Metal, SPIRV, HLSL, etc.
ComPtr<IShaderObject> innerBlock;
device->createShaderObject(..., innerBlock.writeRef());
ShaderCursor(innerBlock)["value"].setData(1.0f);

ComPtr<IShaderObject> outerBlock;
device->createShaderObject(..., outerBlock.writeRef());
ShaderCursor(outerBlock).getDereferenced().setObject(innerBlock);

rootObject["pb"].setObject(outerBlock);  // Set nested parameter block
```

## Comparison to Other Resource Types

This follows the same precedent as:

- **Existential types** (`ExistentialBox<T>`): Use `wrappedBuffer` in type legalization
- **Resource types in cbuffers** (`ConstantBuffer<Texture2D>`): Physical reordering is transparent to reflection

In all cases:
- Reflection shows the **logical source structure**
- Physical layout is **target-specific**
- RHI uses **reflection** to navigate

## Testing

Test cases verify compilation for all targets:
- `tests/bindings/nested-parameter-block-tiny.slang`: 2-level nesting
- `tests/bindings/nested-parameter-block-2.slang`: Struct-nested parameter blocks
- `tests/bindings/nested-parameter-block-3.slang`: 3-level nesting  
- `tests/bindings/nested-parameter-block-4.slang`: Complex 4-level nesting

All tests generate correct reflection JSON and compile successfully for Metal, SPIRV, and HLSL.

