//TEST_IGNORE_FILE:

// module-library-buffer-element-import.slang

// A library for precompiled-spirv-buffer-element-import.slang (#14004).

module "module-library-buffer-element-import";

public struct S
{
    public bool b;
    public float3x3 m;
    public float x;
}

public float readS(S s)
{
    return s.b ? s.m[0][0] : s.x;
}
