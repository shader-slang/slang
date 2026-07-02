// unit-test-diagnostic-warning-level.cpp
//
// Contract under test
// -------------------
// `DiagnosticSink` supports clang/gcc-style warning groups. Each `DiagnosticInfo`
// carries a `WarningLevel`; the always-on `Default` group is emitted unconditionally,
// while the `All`/`Extra`/`Pedantic` groups are opt-in via `enableWarningLevel`.
// Group gating is applied during `diagnose`, after per-id overrides and before the
// treat-warnings-as-errors flag.
//
// These tests drive the public `diagnose` API and observe the result via its return
// value (false when a diagnostic is suppressed) and `getErrorCount` (for the
// warnings-as-errors interaction). They exercise the off-by-default groups that no
// production warning is tagged with yet, so a regression that ignored
// `DiagnosticInfo::level` would be caught here.

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

// Default state: Default and Pedantic groups are emitted; All and Extra are suppressed.
SLANG_UNIT_TEST(warningLevelDefaultGating)
{
    DiagnosticSink sink;

    SLANG_CHECK(emitted(sink, makeWarning(1, WarningLevel::Default)) == true);
    SLANG_CHECK(emitted(sink, makeWarning(2, WarningLevel::Pedantic)) == true);
    SLANG_CHECK(emitted(sink, makeWarning(3, WarningLevel::All)) == false);
    SLANG_CHECK(emitted(sink, makeWarning(4, WarningLevel::Extra)) == false);
}

// Enabling a group is additive and affects only that group.
SLANG_UNIT_TEST(warningLevelEnableIsAdditive)
{
    DiagnosticSink sink;
    sink.enableWarningLevel(WarningLevel::All);

    SLANG_CHECK(emitted(sink, makeWarning(3, WarningLevel::All)) == true);
    // Extra was not enabled, so it stays suppressed.
    SLANG_CHECK(emitted(sink, makeWarning(4, WarningLevel::Extra)) == false);

    sink.enableWarningLevel(WarningLevel::Extra);
    SLANG_CHECK(emitted(sink, makeWarning(4, WarningLevel::Extra)) == true);
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

    // -Wno-<name> on an on-by-default pedantic warning: suppressed despite the group being on.
    auto pedanticWarning = makeWarning(2, WarningLevel::Pedantic);
    sink.overrideDiagnosticSeverity(pedanticWarning.id, Severity::Disable, &pedanticWarning);
    SLANG_CHECK(emitted(sink, pedanticWarning) == false);
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
    // A group the parent did not enable is not inherited either.
    SLANG_CHECK(child.isWarningLevelEnabled(WarningLevel::Extra) == false);
    SLANG_CHECK(emitted(child, makeWarning(4, WarningLevel::Extra)) == false);
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
