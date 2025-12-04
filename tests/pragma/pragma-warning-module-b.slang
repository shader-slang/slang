module "pragma-warning-module-b";

// Module B with shorter content to have lower absolute source locations
public bool moduleBFunction(float input, out float result) {
#pragma warning(disable: 41018)  // This should NOT generate warning 15615 anymore
    if (input > 5.0) return true;  // This should still generate warning 41018
#pragma warning(default: 41018)
    result = 0.0;
    return false;
}