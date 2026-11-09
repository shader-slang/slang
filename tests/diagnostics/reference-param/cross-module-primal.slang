//TEST_IGNORE_FILE:

// Functions with reference parameters and no derivatives, for tests that try to give them
// derivatives in a module that imports this one with a plain `import`.

module "cross-module-primal";

public float referenceForwardPrimal(__ref float a, float x) { return a * x * x; }

public float referenceBackwardPrimal(__ref float a, float x) { return a * x * x; }

public float groupSharedForwardPrimal(groupshared float a, float x) { return a * x * x; }

public float groupSharedBackwardPrimal(groupshared float a, float x) { return a * x * x; }

public float referenceForwardExtended(__ref float a, float x) { return a * x * x; }

public float referenceBackwardExtended(__ref float a, float x) { return a * x * x; }

public float referenceApplyExtended(__ref float a, float x) { return a * x * x; }

public float groupSharedForwardExtended(groupshared float a, float x) { return a * x * x; }

public float groupSharedBackwardExtended(groupshared float a, float x) { return a * x * x; }

public float groupSharedApplyExtended(groupshared float a, float x) { return a * x * x; }
