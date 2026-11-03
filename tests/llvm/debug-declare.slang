// A local variable that keeps its storage is described by one debug variable
// declared on its stack slot, rather than by a second debug variable that the
// LLVM emitter invents for every named alloca.

//TEST:SIMPLE(filecheck=LLVM): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -g3 -O0 -validate-ir
//TEST:SIMPLE(filecheck=NODUP): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -g3 -O0 -validate-ir

struct Pair
{
    int a;
    int b;
};

export __extern_cpp int main(int argc, Ptr<NativeString> argv)
{
    // `pair` is stored to field by field, so SSA construction keeps its storage.
    Pair pair;
    pair.a = argc;
    pair.b = argc + 1;
    return pair.a + pair.b;
}

// LLVM: #dbg_declare(ptr %pair, [[PAIR:![0-9]+]], !DIExpression()
// LLVM: [[PAIR]] = !DILocalVariable(name: "pair"

// NODUP: !DILocalVariable(name: "pair"
// NODUP-NOT: !DILocalVariable(name: "pair"
