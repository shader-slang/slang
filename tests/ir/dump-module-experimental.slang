// `-dump-module` must inherit `-experimental-feature` so that a module importing an experimental
// standard module (slang.neural) disassembles rather than failing the experimental-feature gate.
// The negative run also pins that inheritance carries the flag's actual value: without
// `-experimental-feature` the gate must still fire (E00104), not be silently force-enabled.

//TEST:COMPILE: tests/ir/dump-module-experimental.slang -o tests/ir/dump-module-experimental.slang-module -experimental-feature

// SIMPLE_EX (not SIMPLE) so slang-test does not prepend this source file as a compile input — the
// runs must exercise only `-dump-module` on the compiled module, otherwise the negative run's
// diagnostic could come from compiling this file's `import slang.neural` rather than from the dump.
//TEST:SIMPLE_EX(filecheck=CHECK): -experimental-feature -dump-module tests/ir/dump-module-experimental.slang-module
//TEST:SIMPLE_EX(filecheck=CHECKNEG): -dump-module tests/ir/dump-module-experimental.slang-module

module dump_module_experimental;
import slang.neural;

public int addOneDumpExp(int x)
{
    return x + 1;
}

// With -experimental-feature the module disassembles (tokens only a successful dump emits):
// CHECK: nameHint("addOneDumpExp")
// CHECK: func %addOneDumpExp

// Without it the experimental-feature gate still fires and no disassembly is produced:
// CHECKNEG: error{{.*}}experimental module
// CHECKNEG-NOT: func %addOneDumpExp
