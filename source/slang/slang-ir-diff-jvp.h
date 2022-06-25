// slang-ir-diff-jvp.h
#pragma once

namespace Slang
{
    struct IRModule;

    struct IRJVPDerivativePassOptions
    {
        // Nothing for now..
    };

    bool processJVPDerivativeMarkers(
        IRModule*                           module,
        IRJVPDerivativePassOptions const&   options = IRJVPDerivativePassOptions());

}
