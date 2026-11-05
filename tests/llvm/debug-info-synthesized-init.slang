//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -g2 -O0 -validate-ir

// Check that LLVM emits debug records for generated and user-written constructors.
// An artificial flag on the Slang debug function must not suppress LLVM debug info.
//CHECK-DAG: !DISubprogram(name: "Generated.$init"
//CHECK-DAG: !DISubprogram(name: "Authored.$init"
//CHECK-DAG: !DISubprogram(name: "main"

struct Generated
{
    int value = 7;
};
struct Authored
{
    int value;
    __init(int input) { value = input + 1; }
};
export __extern_cpp int main(int argc, Ptr<NativeString> argv)
{
    Generated generated = {};
    Authored authored = Authored(argc);
    return generated.value + authored.value;
}
