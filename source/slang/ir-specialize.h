// ir-specialize.h
#pragma once

namespace Slang
{
struct IRModule;

// Find suitable uses of the `specialize` instruction that
// can be replaced with references to specialized functions.
void specializeGenerics(
    IRModule*   module);

}
