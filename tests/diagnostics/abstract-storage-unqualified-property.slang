//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Module-scope properties have no receiver expression, but semantic checking must still select
// and validate the accessor required by each use.

static int globalBacking;

property int globalWriteOnly
{
    set { globalBacking = newValue; }
}

void writeGlobalProperty()
{
    // A module-scope property has no receiver expression. Semantic checking still selects the
    // setter for assignment.
    globalWriteOnly = 42;
}

int rejectGlobalPropertyRead()
{
    // Semantic checking must reject the unsupported getter operation before lowering tries to
    // materialize the property value.
    return globalWriteOnly;
//CHECK:   ^^^^^^^^^^^^^^^ property or subscript cannot be read
//CHECK:   ^^^^^^^^^^^^^^^ cannot read this property because it has no `get` or `ref` accessor
}
