//TEST_IGNORE_FILE:

// Functions with reference parameters and no derivatives, for tests that try to give them
// derivatives in a module that imports this one with `__exported import`.

module "cross-module-exported-reference-primal";

public float exportedReferenceForwardPrimal(__ref float a, float x) { return a * x * x; }

public float exportedReferenceBackwardPrimal(__ref float a, float x) { return a * x * x; }

public float exportedGroupSharedForwardPrimal(groupshared float a, float x) { return a * x * x; }

public float exportedGroupSharedBackwardPrimal(groupshared float a, float x) { return a * x * x; }

public float exportedReferenceForwardExtended(__ref float a, float x) { return a * x * x; }

public float exportedReferenceBackwardExtended(__ref float a, float x) { return a * x * x; }

public float exportedReferenceApplyExtended(__ref float a, float x) { return a * x * x; }

public float exportedGroupSharedForwardExtended(groupshared float a, float x) { return a * x * x; }

public float exportedGroupSharedBackwardExtended(groupshared float a, float x) { return a * x * x; }

public float exportedGroupSharedApplyExtended(groupshared float a, float x) { return a * x * x; }
