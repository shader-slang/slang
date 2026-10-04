// Lexical debug IR must remain consumable by the LLVM and C-like emitters.
//TEST:SIMPLE(filecheck=LLVM): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -g3 -O0 -validate-ir
//TEST:SIMPLE(filecheck=CPP): -target cpp -whole-program -g3 -O0 -validate-ir

// LLVM: define i32 @main
// LLVM: !DILocalVariable(name: "scopedValue"
// CPP: main(
// CPP: return

[ForceInline]
int addScoped(int value)
{
    {
        int scopedValue = value + 1;
        return scopedValue;
    }
}

export __extern_cpp int main(int argc, Ptr<NativeString> argv)
{
    int value = argc;
    {
        int scopedValue = addScoped(value);
        value += scopedValue;
    }
    return value;
}
