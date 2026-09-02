/**
Differentiable refinements of the experimental numeric interfaces.

@remarks Importing this annex re-exports `slang.numerics`. Its interfaces re-declare only the
operations for which differentiability is part of the contract.
*/
[ExperimentalModule]
module differentiable;

__exported import slang.numerics;
import slang.numerics.__builtin_differentiable;

__include "differentiable-interfaces.slang";
__include "differentiable-exp10.slang";
__include "differentiable-conformances.slang";
__include "differentiable-wrappers.slang";
