//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static int globalBacking;

property int globalWriteOnly
{
    set { globalBacking = newValue; }
}

void writeGlobalProperty()
{
    // A module-scope property has no receiver expression. Assignment still selects its setter.
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
