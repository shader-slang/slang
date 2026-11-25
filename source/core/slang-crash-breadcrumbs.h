// slang-crash-breadcrumbs.h
#ifndef SLANG_CORE_CRASH_BREADCRUMBS_H
#define SLANG_CORE_CRASH_BREADCRUMBS_H

#include "slang-common.h"

namespace Slang
{

/// A piece of text that a breadcrumb points at, without a copy. The text is a name from
/// Slang's own vocabulary, such as `spirv` or `legalizeVaryingParams`, that lives for the whole
/// process: a string literal or an entry of a static table.
///
/// We store a length because some of those tables hold their names in a comma-separated list,
/// so a name is not always terminated by a NUL.
struct CrashBreadcrumbText
{
    const char* chars = nullptr;
    int length = 0;

    bool isSet() const { return chars != nullptr; }
};

/// Where in the compiler the calling thread is: the facts that a crash report states in its
/// `[report]` part so that a reader knows what the compiler was doing when it failed.
///
/// Consider a crash that happens while slangc emits SPIR-V for a compute entry point and runs
/// the IR pass `legalizeVaryingParams`. The report then holds:
///
///     phase:       emit
///     pass:        legalizeVaryingParams
///     target:      spirv
///     stage:       compute
///
/// Every value is a name that Slang defines, so the `[report]` part stays safe to publish.
struct CrashBreadcrumbs
{
    CrashBreadcrumbText phase;
    CrashBreadcrumbText pass;
    CrashBreadcrumbText target;
    CrashBreadcrumbText stage;
};

/// The fields of `CrashBreadcrumbs`, so that a scope can say which one it sets.
enum class CrashBreadcrumbField
{
    Phase,
    Pass,
    Target,
    Stage,
};

struct CrashBreadcrumbUtil
{
    /// Return the breadcrumbs of the calling thread.
    ///
    /// The storage is per thread, because a host application compiles on several threads and
    /// a report describes the thread that failed. Reading it never allocates, so a signal
    /// handler can read it, once the thread has set any breadcrumb.
    static const CrashBreadcrumbs& getCurrent();

    /// Return a modifiable reference to the breadcrumbs of the calling thread. Only
    /// `CrashBreadcrumbScope` writes them.
    static CrashBreadcrumbs& getCurrentForWriting();
};

/// Set one breadcrumb of the calling thread for the lifetime of this object, and restore the
/// value that it had before when the object is destroyed.
///
/// Scopes nest the way the compiler's work does: emitting SPIR-V sets `phase` to `emit`, a
/// pass inside it sets `pass`, and when the pass ends, `pass` goes back to what it was. An
/// exception that unwinds through a scope restores the value too, so the breadcrumbs that a
/// later report reads are never left over from work that ended. The record of an internal
/// error takes a copy of the breadcrumbs when the error is raised, before that unwinding.
struct CrashBreadcrumbScope
{
    CrashBreadcrumbScope(CrashBreadcrumbField field, const char* text);
    CrashBreadcrumbScope(CrashBreadcrumbField field, const char* chars, int length);
    ~CrashBreadcrumbScope();

    CrashBreadcrumbScope(const CrashBreadcrumbScope&) = delete;
    CrashBreadcrumbScope& operator=(const CrashBreadcrumbScope&) = delete;

private:
    CrashBreadcrumbField m_field;
    CrashBreadcrumbText m_previous;
};

} // namespace Slang

#endif
