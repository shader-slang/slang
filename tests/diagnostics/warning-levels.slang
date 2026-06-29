// Tests the warning-level (group) mechanism. `implicit-conversion-to-double` (E30082) is tagged
// as a "pedantic" group warning, so it is suppressed by default and only emitted when its group
// is enabled. Each directive below is a separate slangc run with its own diagnostic-check buffer;
// the two runs that should emit the warning share the `WON` buffer so they check one annotation.

// 1. Default: the pedantic warning is suppressed, so the run is diagnostic-free.
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT): -target cpp -entry cs -stage compute

// 2. -Wall enables a different group, so the pedantic warning is still suppressed.
//DIAGNOSTIC_TEST:SIMPLE(diag=WALL): -Wall -target cpp -entry cs -stage compute

// 3. -Wpedantic enables the group, so the warning is emitted.
//DIAGNOSTIC_TEST:SIMPLE(diag=WON): -Wpedantic -target cpp -entry cs -stage compute

// 4. An explicit per-id enable also turns it on, even without enabling the whole group.
//DIAGNOSTIC_TEST:SIMPLE(diag=WON): -Wimplicit-conversion-to-double -target cpp -entry cs -stage compute

// 5. Enabling the group but disabling this id keeps it suppressed (-Wno wins).
//DIAGNOSTIC_TEST:SIMPLE(diag=WNO): -Wpedantic -Wno-implicit-conversion-to-double -target cpp -entry cs -stage compute

double sink_d(double x)
{
    return x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cs()
{
    float f = 1.0f;
    double d = sink_d(f);
//WON:                ^ implicit float-to-double conversion
//WON:                ^ implicit float-to-double conversion may cause unexpected performance issues, use explicit cast if intended.
    printf("%f\n", d);
}
