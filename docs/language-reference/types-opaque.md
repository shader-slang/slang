# Opaque Types

Opaque types are built-in types that have target-defined representation in memory. This includes the bit
representation, the underlying type, and characteristics such as size and alignment. Other shader languages
may refer to these as *opaque handle types*, *opaque descriptor types*, *resource object types*, or *resource
data types*.

The full list of opaque types supported by Slang can be found in the core module reference. Some important
examples:

* Texture types such as `Texture2D<T>`, `TextureCubeArray<T>`, `DepthTexture2D<T>`, and `RWTexture2DMS<T>`
* Combined texture-sampler types such as `Sampler2D<T>` and `Sampler2DShadow<T>`
* Sampler state types: `SamplerState` and `SamplerComparisonState`
* Buffer types like `ConstantBuffer<T>` and  `StructuredBuffer<T>`
* Parameter blocks: `ParameterBlock<T>`

Slang makes no guarantees about layout rules or type conversion rules of opaque types.
