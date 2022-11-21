// slang-ir-autodiff-fwd.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

namespace Slang
{

    template<typename P, typename D>
    struct DiffInstPair
    {
        P primal;
        D differential;
        DiffInstPair() = default;
        DiffInstPair(P primal, D differential) : primal(primal), differential(differential)
        {}
        HashCode getHashCode() const
        {
            Hasher hasher;
            hasher << primal << differential;
            return hasher.getResult();
        }
        bool operator ==(const DiffInstPair& other) const
        {
            return primal == other.primal && differential == other.differential;
        }
    };
    
    typedef DiffInstPair<IRInst*, IRInst*> InstPair;

    struct IRJVPDerivativePassOptions
    {
        // Nothing for now..
    };

    bool processForwardDerivativeCalls(
        IRModule*                           module,
        DiagnosticSink*                     sink,
        IRJVPDerivativePassOptions const&   options = IRJVPDerivativePassOptions());

}
