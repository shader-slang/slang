module "pragma-warning-module-a";

// A longer comment to ensure this module has more content and later source locations
// This simulates the scenario where module A gets processed first and has higher
// absolute source locations than module B, which would cause the original bug.
// More content here to push the locations higher...
// Even more content...
// And more...
public bool moduleAFunction(float input, out float result) {
    if (input <= 0.0) {
#pragma warning(disable: 41018)  // Disable uninitialized out parameter warning
        return false;
#pragma warning(default: 41018)  // Re-enable the warning
    }
    result = input * 2.0;
    return true;
}