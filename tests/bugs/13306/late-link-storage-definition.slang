//TEST_IGNORE_FILE:

#ifdef UNSIZED_DEFINITION
struct TrailingArray
{
    uint values[];
};
export struct Thing : TrailingArray
{
};
#elif defined(PARAMETER_BLOCK_DEFINITION)
struct Data
{
    uint value;
};
export struct Thing
{
    ParameterBlock<Data> block;
};
#else
export struct Thing
{
    uint value;
};
#endif

// `readThing` reads a `Thing` through a pointer and does not declare a `Thing` variable.
// Linking must validate any mutable `Thing` storage in the importing module.
export uint readThing(Thing* value)
{
#ifdef UNSIZED_DEFINITION
    return value->values[0];
#elif defined(PARAMETER_BLOCK_DEFINITION)
    return value->block.value;
#else
    return value->value;
#endif
}
