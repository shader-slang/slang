// unit-test-diagnostic-warning-level.cpp
//
// Contract under test
// -------------------
// `DiagnosticSink` supports clang/gcc-style warning groups. Each `DiagnosticInfo`
// carries a `WarningLevel`; the always-on `Default` group is emitted unconditionally.
// The other groups are independent (not nested): `Extra` is on by default, while `All`
// and `Pedantic` are off by default and opt-in via `enableWarningLevel`. Group gating is
// applied during `diagnose`, after per-id overrides and before the treat-warnings-as-errors
// flag.
//
// These tests drive the public `diagnose` API and observe the result via its return
// value (false when a diagnostic is suppressed) and `getErrorCount` (for the
// warnings-as-errors interaction), so a regression that ignored `DiagnosticInfo::level`
// would be caught here.

#include "compiler-core/slang-diagnostic-sink.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{
// Build a warning `DiagnosticInfo` in the given group. The id is arbitrary but distinct
// per group so a per-id override in one test does not leak into another.
DiagnosticInfo makeWarning(int id, WarningLevel level)
{
    DiagnosticInfo info{id, Severity::Warning, "test-warning", "test warning message", level};
    return info;
}

// True if the diagnostic was emitted (not suppressed by group gating / overrides).
bool emitted(DiagnosticSink& sink, const DiagnosticInfo& info)
{
    return sink.diagnose(SourceLoc(), info);
}
} // namespace

// Default state: Default and Extra groups are emitted; All and Pedantic are suppressed.
SLANG_UNIT_TEST(warningLevelDefaultGating)
{
    DiagnosticSink sink;

    SLANG_CHECK(emitted(sink, makeWarning(1, WarningLevel::Default)) == true);
    SLANG_CHECK(emitted(sink, makeWarning(2, WarningLevel::Extra)) == true);
    SLANG_CHECK(emitted(sink, makeWarning(3, WarningLevel::All)) == false);
    SLANG_CHECK(emitted(sink, makeWarning(4, WarningLevel::Pedantic)) == false);
}

// Enabling a group is additive and affects only that group. Both groups exercised here are
// off by default (All and Pedantic), so enabling one must not enable the other.
SLANG_UNIT_TEST(warningLevelEnableIsAdditive)
{
    DiagnosticSink sink;
    sink.enableWarningLevel(WarningLevel::All);

    SLANG_CHECK(emitted(sink, makeWarning(3, WarningLevel::All)) == true);
    // Pedantic was not enabled, so it stays suppressed.
    SLANG_CHECK(emitted(sink, makeWarning(4, WarningLevel::Pedantic)) == false);

    sink.enableWarningLevel(WarningLevel::Pedantic);
    SLANG_CHECK(emitted(sink, makeWarning(4, WarningLevel::Pedantic)) == true);
}

// An explicit per-id enable (e.g. -W<name>) force-enables a grouped warning even when its
// group is off, and a per-id disable (-Wno-<name>) suppresses one even when the group is on.
SLANG_UNIT_TEST(warningLevelPerIdOverrideWins)
{
    DiagnosticSink sink;

    // -W<name> on an off-by-default group warning: the override is kept (the cascading
    // overrideDiagnosticSeverity fix) and forces the warning on.
    auto allWarning = makeWarning(3, WarningLevel::All);
    sink.overrideDiagnosticSeverity(allWarning.id, Severity::Warning, &allWarning);
    SLANG_CHECK(emitted(sink, allWarning) == true);

    // -Wno-<name> on an on-by-default extra warning: suppressed despite the group being on.
    auto extraWarning = makeWarning(2, WarningLevel::Extra);
    sink.overrideDiagnosticSeverity(extraWarning.id, Severity::Disable, &extraWarning);
    SLANG_CHECK(emitted(sink, extraWarning) == false);
}

// Treat-warnings-as-errors runs after group gating: a suppressed grouped warning stays
// suppressed (does not become an error), but once its group is enabled it becomes an error.
SLANG_UNIT_TEST(warningLevelWarningsAsErrorsInteraction)
{
    DiagnosticSink sink;
    sink.setFlag(DiagnosticSink::Flag::TreatWarningsAsErrors);

    // A default-group warning becomes an error.
    SLANG_CHECK(sink.getErrorCount() == 0);
    emitted(sink, makeWarning(1, WarningLevel::Default));
    SLANG_CHECK(sink.getErrorCount() == 1);

    // An off-by-default group warning is gated out before warnings-as-errors applies, so it
    // is neither emitted nor counted as an error.
    SLANG_CHECK(emitted(sink, makeWarning(3, WarningLevel::All)) == false);
    SLANG_CHECK(sink.getErrorCount() == 1);

    // Once its group is enabled, it does become an error.
    sink.enableWarningLevel(WarningLevel::All);
    emitted(sink, makeWarning(3, WarningLevel::All));
    SLANG_CHECK(sink.getErrorCount() == 2);
}

// A child sink built from a parent inherits the parent's enabled warning groups. Sinks are
// spun up per-translation-unit / per-module from a parent sink via the parent-copying ctor,
// so an opt-in group enabled on the parent must carry over to nested/downstream compilations.
SLANG_UNIT_TEST(warningLevelInheritedFromParentSink)
{
    DiagnosticSink parent;
    parent.enableWarningLevel(WarningLevel::All);

    // The parent-copying ctor forwards the enabled-group bitmask alongside flags/color/unicode.
    DiagnosticSink child(nullptr, nullptr, &parent);

    SLANG_CHECK(child.isWarningLevelEnabled(WarningLevel::All) == true);
    SLANG_CHECK(emitted(child, makeWarning(3, WarningLevel::All)) == true);
    // An off-by-default group the parent did not enable stays off in the child (Pedantic is off
    // by default; Extra would be a poor probe here since it is on by default).
    SLANG_CHECK(child.isWarningLevelEnabled(WarningLevel::Pedantic) == false);
    SLANG_CHECK(emitted(child, makeWarning(4, WarningLevel::Pedantic)) == false);
}

// Out-of-range group values (only reachable via a bogus int cast through the public
// WarningLevel option) must not trigger an out-of-range shift / undefined behavior.
SLANG_UNIT_TEST(warningLevelOutOfRangeIsSafe)
{
    DiagnosticSink sink;
    const auto bogus = (WarningLevel)9999;
    sink.enableWarningLevel(bogus); // must be a no-op, not UB
    SLANG_CHECK(sink.isWarningLevelEnabled(bogus) == false);
    // Valid groups are unaffected by the bogus call.
    SLANG_CHECK(emitted(sink, makeWarning(1, WarningLevel::Default)) == true);
}
