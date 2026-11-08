//TEST_IGNORE_FILE:

// Functions without derivatives, for tests that give them derivatives in a module that imports
// this one with a plain `import`.

module "cross-module-primal";

public float splitForwardPrimal(no_diff float a, float x) { return a * x * x; }

public float mainForwardPrimal(no_diff float a, float x) { return a * x * x; }

public float mainForwardExtended(no_diff float a, float x) { return a * x * x; }

public float mainApplyExtended(no_diff float a, float x) { return a * x * x; }

public float splitForwardExtended(no_diff float a, float x) { return a * x * x; }

public float splitBackwardExtended(no_diff float a, float x) { return a * x * x; }

public float splitApplyExtended(no_diff float a, float x) { return a * x * x; }

public float nonExportedBackwardPrimal(no_diff float a, float x) { return a * x * x; }

public float nonExportedBackwardExtended(no_diff float a, float x) { return a * x * x; }

public float nonExportedLibraryBackwardPrimal(no_diff float a, float x) { return a * x * x; }
