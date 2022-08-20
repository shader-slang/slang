//TEST:EXECUTABLE:
__target_intrinsic(cpp, "printf(\"%s\\n\", ($0).getBuffer())")
void writeln(String text);

import gfx;

public __extern_cpp int main()
{
    gfx.DeviceDesc deviceDesc = {};
    deviceDesc.deviceType = gfx.DeviceType.CPU;
    gfx.IDevice obj;
    if (gfx.succeeded(gfx.gfxCreateDevice(&deviceDesc, obj)))
        writeln("succ");
    else
        writeln("fail");
}