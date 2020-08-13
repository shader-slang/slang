// slang-ir-lower-generic-type.h
#pragma once

namespace Slang
{
    struct SharedGenericsLoweringContext;

    /// Lower all references to generic types (ThisType, AssociatedType, etc.) into IRAnyValueType.
    void lowerGenericType(
        SharedGenericsLoweringContext* sharedContext);

}
