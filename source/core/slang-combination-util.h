#ifndef SLANG_COMBINATION_UTIL_H
#define SLANG_COMBINATION_UTIL_H

#include "list.h"

namespace Slang {

/* Controls how a switch is switched */
enum class SwitchType
{
    On,                 ///< Always on
    Off,                ///< Always off
    OnOff,              ///< Initially on then off
    OffOn,              ///< Initially off then on
};

/* A utility for calculating combinations of flags */
struct CombinationUtil
{
    /* Calculates all the combinations of flags specified by their order, and the switch types.

    Note that the order that flags are specified is the order that they will change, with the first specified being changed (if changing)
    changing first.

    @param flags - Defines the flags that will be changed. At least one bit must bet set, but multiple bits can be set. Bits must be unique
    @param switchTypes - How each flag will be handled
    @param numFlags - The total amount of flags. There must be an identical amount of switchTypes
    @param outCombinations - All of the combinations in order
    */
    static void calc(const uint32_t* flags, const SwitchType* switchTypes, int numFlags, List<uint32_t>& outCombinations);
};

} // namespace Slang

#endif // SLANG_COMBINATION_UTIL_H
