//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry useWindow

__target_intrinsic(hgbc, "OpWindowType") public struct Window
{
    [noinline]
    public static Window nullWindow()
    {
        return hgbc_asm
        {
        result:
            $$Window = OpNullLit;
        };
    }

    [noinline]
    public bool processEvents()
    {
        return hgbc_asm
        {
        result:
            $$bool = OpProcessWindowEvents $this;
        };
    }
}

public bool processWindowEvents(Window window)
{
    return hgbc_asm
    {
    result:
        $$bool = OpProcessWindowEvents $window;
    };
}

[host]
public bool useWindow(Window window)
{
    let other = Window::nullWindow();
    return other.processEvents() && processWindowEvents(window);
}

// CHECK: Section("EXPT")
// CHECK: OpExport Function {{%[A-Za-z0-9_]+}} [[USE_WINDOW:%[A-Za-z0-9_]+]]
// CHECK-NOT: OpExport Type
// CHECK-NOT: OpExport StaticMethod
// CHECK-NOT: OpExport Method
// CHECK: Section("META")
// CHECK: [[WINDOW:%[A-Za-z0-9_]+]] = OpWindowType
// CHECK: [[USE_WINDOW]]
// CHECK: OpProcessWindowEvents
