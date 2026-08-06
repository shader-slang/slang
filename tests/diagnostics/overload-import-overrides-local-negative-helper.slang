//TEST_IGNORE_FILE:
module "overload-import-overrides-local-negative-helper";

// Exports one name the importing module does not declare, and one it does. The first has no
// same-module competitor; the second loses to the importing module's exact match.
public uint noCompetitor(uint val)
{
    return val * 11U;
}

public uint exampleFunction(uint val)
{
    return val * 42U;
}
