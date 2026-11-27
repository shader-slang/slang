// module-library-static-table.slang

// A library whose public function reads a module-scope `static const` table
// with a dynamic index. Precompiled to SPIR-V with -embed-downstream-ir, this
// exercises the entry-point-less legalization path: with no entry points in
// the module there is no legal site for a shared global's initializing store,
// so the read must keep its self-contained per-read-site copy.

module "static-table-lib";

static const float kTable[8] = {0.0, 0.125, 0.25, 0.375, 0.5, 0.625, 0.75, 0.875};

public float tableLookup(float x)
{
    int i = clamp(int(x), 0, 7);
    return kTable[i];
}
