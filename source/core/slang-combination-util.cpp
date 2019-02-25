#include "slang-combination-util.h"

namespace Slang {

void CombinationUtil::calc(const uint32_t* flags, const SwitchType* types, int numFlags, List<uint32_t>& outCombinations)
{
    uint32_t usedFlags = 0;
    uint32_t invertBits = 0;
    int numChangingBits = 0;

    uint32_t changingBits[32];
    for (int i = 0; i < numFlags; ++i)
    {
        const uint32_t flag = flags[i];
        // The flag/s must be set
        SLANG_ASSERT(flag);
        SLANG_ASSERT((flag & usedFlags) == 0);
        // Mark the flags used
        usedFlags |= flag;

        const auto type = types[i];
        if (type == SwitchType::On || type == SwitchType::OnOff)
        {
            invertBits |= flags[i];
        }
        if (type == SwitchType::OnOff || type == SwitchType::OffOn)
        {
            changingBits[numChangingBits++] = flags[i];
        }
    }

    const int numCombinations = 1 << numChangingBits;
    outCombinations.SetSize(numCombinations);
    uint32_t* dstCombinations = outCombinations.Buffer();

    for (int i = 0; i < numCombinations; ++i)
    {
        uint32_t combination = 0;
        uint32_t bit = 1;
        for (int j = 0; j < numChangingBits; ++j, bit += bit)
        {
            combination |= ((bit & i) ? changingBits[j] : 0);
        }

        dstCombinations[i] = combination ^ invertBits;
    }
}

} // namespace Slang
