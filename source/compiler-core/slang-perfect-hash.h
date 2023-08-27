#pragma once

#include "../core/slang-string.h"
#include "../core/slang-list.h"

namespace Slang
{

template<typename T>
struct HashParams
{
    List<UInt32> saltTable;
    List<String> destTable;
    List<T> valueTable;
};

enum class HashFindResult {
    Success,
    NonUniqueKeys,
    UnavoidableHashCollision,
};

// Calculate a minimal perfect hash of a list of input strings
//
// Please note that this doesn't populate hashParams.valueTable, please
// populate that later according to the order in destTable
HashFindResult minimalPerfectHash(const List<String>& ss, HashParams<String>& hashParams);

String perfectHashToEmbeddableCpp(
    const HashParams<String>& hashParams,
    const UnownedStringSlice& valueType);

}
