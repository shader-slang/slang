// slang-repro-validator.h
#ifndef SLANG_REPRO_VALIDATOR_H_INCLUDED
#define SLANG_REPRO_VALIDATOR_H_INCLUDED

#include "core/slang-list.h"

namespace Slang
{

bool isReproStateValid(const List<uint8_t>& buffer);

} // namespace Slang

#endif
