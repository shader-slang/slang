// slang-ir-any-value-marshalling.h
#pragma once

namespace Slang
{
    struct SharedGenericsLoweringContext;

    /// Generates functions that pack and unpack `AnyValue`s, and replaces
    /// all `IRPackAnyValue` and `IRUnpackAnyValue` instructions with calls
    /// to these packing/unpacking functions.
    /// This is a sub-pass of lower-generics.
    void generateAnyValueMarshallingFunctions(
        SharedGenericsLoweringContext* sharedContext);
}
