//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

__target_intrinsic(hgbc, "OpWindowType")
struct Window {}

__target_intrinsic(hgbc, "OpTimerType")
struct Timer {}

Window createWindow(String title, uint width, uint height)
{
    return hgbc_asm {
        result : $$Window = OpCreateWindow $title $width $height;
    };
}

void setTitle(Window window, String title)
{
    hgbc_asm {
        OpSetWindowTitle $window $title;
    };
}

String format<each T>(String pattern, expand each T args)
{
    return hgbc_asm {
        result : $$String = OpFormatString $pattern $expand each args;
    };
}

Timer createTimer()
{
    return hgbc_asm {
        result : $$Timer = OpCreateTimer;
    };
}

float elapsedSeconds(Timer timer)
{
    return hgbc_asm {
        result : $$float = OpTimerElapsed $timer;
    };
}

void resetTimer(Timer timer)
{
    hgbc_asm {
        OpResetTimer $timer;
    };
}

struct StringHolder
{
    String value;
}

[host]
String makeTitle(String prefix, uint count, float fps, bool active)
{
    StringHolder holder = {format("{} - {} items - {:.1f} FPS - {}", prefix, count, fps, active)};
    return holder.value;
}

[host]
void main()
{
    String title = makeTitle("HGBC", 12, 34.5, true);
    Window window = createWindow(title, 640, 480);
    setTitle(window, format("{{{0}}}", title));
    Timer timer = createTimer();
    print(elapsedSeconds(timer));
    resetTimer(timer);
    print(title);
}

// CHECK-DAG: [[STRING:%[A-Za-z0-9_]+]] = OpStringType
// CHECK-DAG: [[TIMER:%[A-Za-z0-9_]+]] = OpTimerType
// CHECK: Section("FUNC")
// Helpers remain out of line, so their intrinsic instructions need not follow call order.
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[STRING]] = OpFormatString
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpCreateWindow
// CHECK-DAG: OpSetWindowTitle
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[TIMER]] = OpCreateTimer
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpTimerElapsed
// CHECK-DAG: OpResetTimer
