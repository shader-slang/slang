# Nested Parameter Blocks

## Overview

Slang supports arbitrary nesting of parameter blocks, allowing you to organize shader parameters hierarchically:

```slang
ParameterBlock<float> innerBlock;
ParameterBlock<ParameterBlock<float>> outerBlock;
```

The compiler handles nested parameter blocks differently depending on the target, but the reflection API presents a consistent cross-platform view.

## Basic Usage

### Declaration

```slang
// Simple parameter block
ParameterBlock<float> value;

// Nested parameter blocks  
ParameterBlock<ParameterBlock<float>> nested2;
ParameterBlock<ParameterBlock<ParameterBlock<float>>> nested3;

// Parameter blocks containing structs with nested parameter blocks
struct Material {
    ParameterBlock<Texture2D> albedo;
    float roughness;
};
ParameterBlock<Material> material;
```

### Access

```slang
[shader("compute")]
void main() {
    // Access nested parameter blocks
    float v = nested2;  // Automatically dereferences through all levels
    
    // Access fields in nested structures
    float4 color = material.albedo.Sample(sampler, uv);
}
```

## Target-Specific Behavior

### Metal

On Metal, nested parameter blocks are implemented using **argument buffers with pointer chains**:

```metal
// Generated Metal code for ParameterBlock<ParameterBlock<float>>
struct wrapper_ParameterBlock_float {
    float constant* inner;
};

[[kernel]] void main(
    wrapper_ParameterBlock_float constant* outerBlock [[buffer(0)]]
) {
    float value = *outerBlock->inner;  // Dereference through wrapper
}
```

**Key Points:**
- Each nesting level is wrapped in a struct
- Metal uses `constant*` pointers for indirection
- Supports up to 10 levels of nesting (configurable)

### HLSL / D3D12

On HLSL, nested parameter blocks are **flattened** to the innermost element type:

```hlsl
// Generated HLSL code for ParameterBlock<ParameterBlock<float>>
cbuffer outerBlock {
    float value;  // Flattened to innermost type
}

void main() {
    float v = value;  // Direct access
}
```

**Key Points:**
- Nested parameter blocks unwrap to innermost type
- Uses standard `cbuffer` declarations
- All nesting information is resolved at compile time

### SPIRV / Vulkan

On SPIRV, nested parameter blocks are **flattened** similar to HLSL:

```spirv
OpDecorate %value Binding 0
OpDecorate %value DescriptorSet 0

%value = OpVariable %_ptr_Uniform_float Uniform
```

**Key Points:**
- Flattened to innermost type
- Uses separate descriptor sets for each level
- Standard Vulkan descriptor binding model

## Reflection API

Regardless of target, the reflection API presents a **unified logical structure**:

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

This allows cross-platform RHI code to work identically across all targets.

## RHI Usage

### Setting Nested Parameter Blocks

```cpp
// Create inner parameter block
ComPtr<IShaderObject> innerBlock;
device->createShaderObject(..., innerBlock.writeRef());
ShaderCursor(innerBlock).setData(1.0f);

// Create outer parameter block
ComPtr<IShaderObject> outerBlock;
device->createShaderObject(..., outerBlock.writeRef());

// Set the nested parameter block
ShaderCursor(outerBlock).getDereferenced().setObject(innerBlock);

// Bind to root
rootObject["myNestedBlock"].setObject(outerBlock);
```

### Sharing Parameter Blocks

You can set the same parameter block to multiple locations:

```cpp
ComPtr<IShaderObject> sharedMaterial;
device->createShaderObject(..., sharedMaterial.writeRef());

// All scenes share the same material
cursor["scene1"]["material"].setObject(sharedMaterial);
cursor["scene2"]["material"].setObject(sharedMaterial);
cursor["scene3"]["material"].setObject(sharedMaterial);
```

The RHI stores reference-counted pointers, so modifying the shared object affects all references.

## Best Practices

### 1. Use Nested Parameter Blocks for Logical Organization

```slang
struct SceneParams {
    ParameterBlock<Camera> camera;
    ParameterBlock<Lighting> lighting;
};

ParameterBlock<SceneParams> scene;
```

This provides clear separation of concerns and enables selective updates.

### 2. Limit Nesting Depth

While the compiler supports up to 10 levels of nesting, excessive nesting can impact performance on some targets. Keep nesting depth ≤ 3 levels for optimal performance.

### 3. Consider Target Differences for Performance

- **Metal**: Each nesting level adds an indirection (pointer dereference)
- **HLSL/SPIRV**: Flattened at compile time, no runtime overhead

If targeting Metal primarily, consider flattening manually for performance-critical code.

### 4. Use Reflection for Cross-Platform Code

Always use reflection APIs to navigate parameter blocks in RHI code:

```cpp
auto paramType = paramLayout->getType();
if (paramType->getKind() == slang::TypeReflection::Kind::ParameterBlock) {
    auto elementType = paramType->getElementType();
    // Handle nested structure using reflection
}
```

## Implementation Details

### Compilation Pipeline

1. **Frontend**: Parses nested parameter block types
2. **Type Layout**: Creates layouts based on source structure
3. **Legalization** (HLSL/SPIRV): Uses `implicitDeref` to recursively unwrap
4. **Wrapping** (Metal): Wraps each level in a struct with pointer field
5. **Emission**: Generates target-specific code

### Reflection Generation

Reflection is generated from type layouts **before** target-specific transformations, ensuring consistent cross-platform structure.

### RHI Binding

The RHI uses reflection to navigate parameter blocks:
- **Metal**: Recursively creates argument buffers, writes GPU pointers
- **HLSL/SPIRV**: Flattened binding directly maps to descriptor sets/cbuffers

## Testing

The following test cases verify nested parameter block support:

- `tests/bindings/nested-parameter-block-tiny.slang`: 2-level nesting
- `tests/bindings/nested-parameter-block-2.slang`: Struct-nested parameter blocks
- `tests/bindings/nested-parameter-block-3.slang`: 3-level nesting
- `tests/bindings/nested-parameter-block-4.slang`: Complex 4-level nesting

All tests compile successfully for Metal, HLSL, and SPIRV targets.

## Troubleshooting

### Metal Compilation Errors

**Error**: `error: cannot declare parameter of type 'float **'`

**Cause**: Very old Metal version or nested parameter blocks not wrapped properly.

**Solution**: Update to latest Slang compiler. Metal wrapping is automatically applied for macOS 10.14+ / iOS 12+.

### SPIRV Validation Errors

**Error**: `OpLoad Pointer <id> ... is not a logical pointer`

**Cause**: Trying to use parameter blocks as direct pointers in SPIRV.

**Solution**: Let the compiler handle dereferencing automatically. Don't manually cast or use pointer operations.

### Performance Issues on Metal

**Symptom**: Slow parameter access with deep nesting.

**Cause**: Each nesting level adds a pointer indirection on Metal.

**Solution**: Flatten deeply nested structures manually or use compile-time specialization.

## See Also

- [Parameter Binding](parameter-binding.md): How parameters are bound to shader resources
- [Reflection API](../reflection-api.md): Using reflection to query shader parameters
- [Metal Argument Buffers](https://developer.apple.com/documentation/metal/argument_buffers): Metal's native feature backing parameter blocks

