//TEST_IGNORE_FILE:

// Functions without derivatives, for tests that give them derivatives in a module that imports
// this one with `__exported import`.

module "cross-module-exported-primal";

public float exportedSplitForwardPrimal(no_diff float a, float x) { return a * x * x; }

public float exportedSplitBackwardPrimal(no_diff float a, float x) { return a * x * x; }

public float exportedSplitForwardExtended(no_diff float a, float x) { return a * x * x; }

public float exportedSplitBackwardExtended(no_diff float a, float x) { return a * x * x; }

public float exportedSplitApplyExtended(no_diff float a, float x) { return a * x * x; }

public float exportedMainForwardPrimal(no_diff float a, float x) { return a * x * x; }

public float exportedMainBackwardPrimal(no_diff float a, float x) { return a * x * x; }

public float exportedMainForwardExtended(no_diff float a, float x) { return a * x * x; }

public float exportedMainBackwardExtended(no_diff float a, float x) { return a * x * x; }

public float exportedMainApplyExtended(no_diff float a, float x) { return a * x * x; }
