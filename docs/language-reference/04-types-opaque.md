Opaque Types
------------

_Opaque_ types are built-in types that (depending on the target platform) may not have a well-defined size or representation in memory.
Similar languages may refer to these as "resource types" or "object types."

The full list of opaque types supported by Slang can be found in the core module reference, but important examples are:

* Texture types such as `Texture2D<T>`, `TextureCubeArray<T>`, and `RWTexture2DMS<T>`
* Sampler state types: `SamplerState` and `SamplerComparisonState`
* Buffer types like `ConstantBuffer<T>` and  `StructuredBuffer<T>`
* Parameter blocks: `ParameterBlock<T>`

Layout for opaque types depends on the target platform, and no specific guarantees can be made about layout rules across platforms.

