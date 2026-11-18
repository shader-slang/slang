// Library module for static-init-cross-module.slang: the unreferenced static
// with a side-effecting initializer lives here, in a module separate from the
// entry point, so the test pins the per-module allowlist and the linking-info
// cache across a module boundary.

static int g_state = 0;

export int bump()
{
    g_state = g_state + 3;
    return 6;
}

export int get_state()
{
    return g_state;
}

static int s = bump() + bump();
