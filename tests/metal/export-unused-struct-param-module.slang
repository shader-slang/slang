// Library module for export-unused-struct-param-link.slang. It is compiled to a .slang-module
// by that test; this file has no test directives of its own.

module "export-unused-struct-param-module";

public struct Payload
{
    public float3 a;
    public float b;
}

export public float unusedExport(Payload p)
{
    return p.a.x + p.b;
}
