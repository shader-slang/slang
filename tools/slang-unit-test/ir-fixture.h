// ir-fixture.h
//
// Test fixture for IR-pass unit tests. See
// `docs/design/ir-pass-unit-testing.md` for the design.
//
// Two builder modes are described in the design doc:
//   * Option A — `compileSlangToIR(source)`: compile a Slang source
//     string and observe / mutate the resulting `IRModule*`.
//     Implemented here.
//   * Option B — `IRFixtureBuilder`: a fluent C++ DSL that builds
//     the IR directly via `IRBuilder`. Sketched in the design doc;
//     not implemented yet because it requires symbol visibility
//     of internal `IRBuilder` methods (tracked as #10950).
//
// Both styles produce an `IRFixture` so tests can be ported between
// them as their inputs evolve.
//
// Note on the API surface: every accessor here has been chosen so
// it can be implemented using only inline IR accessors and public
// IR fields. Anything that would require an out-of-line symbol from
// `libslang-compiler.dylib` (e.g. `IRInst::getOperands`,
// `IRConstant::getStringSlice`, `IRInstListBase::Iterator`) is
// deliberately omitted until #10950 unblocks symbol visibility.

#ifndef SLANG_TOOLS_UNIT_TEST_IR_FIXTURE_H
#define SLANG_TOOLS_UNIT_TEST_IR_FIXTURE_H

#include "../../include/slang-com-ptr.h"
#include "../../source/core/slang-basic.h"
#include "../../source/core/slang-string.h"

namespace slang
{
struct IGlobalSession;
struct ISession;
struct IModule;
} // namespace slang

namespace Slang
{

struct IRModule;
struct IRInst;
struct IRFunc;
class Module;

// Opcode identifier used by `countInsts` / opcode-based queries.
// We use the bare integer to avoid pulling the full `kIROp_*` enum
// header (and its FIDDLE-generated bits) into every test TU.
typedef int IROpCode;

class IRFixture
{
public:
    /// The IRModule wrapped by this fixture. nullptr if the fixture
    /// was constructed from a compile that failed.
    IRModule* module() const { return m_irModule; }

    /// The compiled module wrapper (Option A only).
    Module* slangModule() const { return m_slangModule; }

    /// Diagnostic text from the underlying compile, or empty if
    /// the compile succeeded.
    const String& errorMessage() const { return m_errorMessage; }

    /// True iff `module()` is non-null. (Deeper invariant checks
    /// will be wired through `validateIRModule` once tests need
    /// post-pass invariant assertions; that path requires a
    /// DiagnosticSink and is intentionally deferred.)
    bool isValid() const;

    /// Count instructions whose opcode is `op` anywhere in the
    /// module (recursively walks every instruction's combined
    /// decoration+child list using the linked-list pointers
    /// directly — no out-of-line accessor calls).
    Index countInsts(IROpCode op) const;

    /// Return the first instruction in the module (in document
    /// order) whose opcode equals `op`, or nullptr if none.
    /// Walks the same linked list as `countInsts`.
    IRInst* findFirstInst(IROpCode op) const;

    /// Move-only: the fixture owns a `RefPtr<Module>` and a
    /// session reference. Don't copy.
    IRFixture(IRFixture const&) = delete;
    IRFixture& operator=(IRFixture const&) = delete;
    IRFixture(IRFixture&&) noexcept;
    IRFixture& operator=(IRFixture&&) noexcept;
    ~IRFixture();

private:
    friend IRFixture compileSlangToIR(const char* source, const char* entryPointName);
    friend class IRFixtureBuilder;

    IRFixture();

    // Approach A path: we keep the Slang Module object alive (it
    // owns the IRModule's storage) and the session that created it.
    //
    // Approach B path: m_iModule / m_slangModule are null;
    // m_ownedModule holds a strong reference to the hand-built
    // IRModule (which owns its own MemoryArena), and m_session is
    // a ComPtr<ISession> that keeps the underlying Slang::Session
    // alive for the IRModule's lifetime.
    ComPtr<slang::ISession> m_session;
    ComPtr<slang::IModule> m_iModule;
    Module* m_slangModule = nullptr;
    IRModule* m_irModule = nullptr;
    RefPtr<IRModule> m_ownedModule;
    String m_errorMessage;
};

/// Compile `source` through the Slang frontend and return an
/// IRFixture wrapping the resulting IRModule.
///
/// `entryPointName` is the function to mark as the entry point.
/// If null, the source is compiled as a library module — useful
/// when the test wants to exercise a pass on arbitrary functions
/// without entry-point legalization.
///
/// On compile error returns a fixture with `module() == nullptr`
/// and `errorMessage()` populated. The caller should
/// `SLANG_CHECK_ABORT(f.module() != nullptr)` early.
IRFixture compileSlangToIR(const char* source, const char* entryPointName = nullptr);

} // namespace Slang

#endif // SLANG_TOOLS_UNIT_TEST_IR_FIXTURE_H
