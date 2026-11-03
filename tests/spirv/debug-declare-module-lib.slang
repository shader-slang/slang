// Library half of debug-declare-module-g0.slang. Its COMPILE step precompiles it at -g2, so the
// serialized module carries `DebugVarStorage` decorations on `stepped`'s local array and on its
// `inout` parameter.

module debug_declare_module_lib;

public void stepped(inout float value, int k)
{
    float history[2];
    history[0] = value;
    history[1] = value * 2;
    value = history[k & 1];
}
