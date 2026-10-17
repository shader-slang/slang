// Library half of forceinline-module-load.slang. That test precompiles this file to a
// .slang-module and imports it, so the user `[ForceInline]` marker on `serializedHelper` reaches
// the CUDA consumer only through serialized IR.
module forceinline_module_helper;

[ForceInline]
public float serializedHelper(float x)
{
    return x * 2.0f;
}
