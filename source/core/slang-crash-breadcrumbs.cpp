// slang-crash-breadcrumbs.cpp
#include "slang-crash-breadcrumbs.h"

#include <string.h>

namespace Slang
{

// A plain aggregate with constant initialization, so a thread-local of it needs no constructor
// call and its first use does not run code that could allocate.
static thread_local CrashBreadcrumbs t_breadcrumbs;

/* static */ const CrashBreadcrumbs& CrashBreadcrumbUtil::getCurrent()
{
    return t_breadcrumbs;
}

/* static */ CrashBreadcrumbs& CrashBreadcrumbUtil::getCurrentForWriting()
{
    return t_breadcrumbs;
}

/// Return the field `field` of `breadcrumbs`.
static CrashBreadcrumbText& _getField(CrashBreadcrumbs& breadcrumbs, CrashBreadcrumbField field)
{
    switch (field)
    {
    case CrashBreadcrumbField::Phase:
        return breadcrumbs.phase;
    case CrashBreadcrumbField::Pass:
        return breadcrumbs.pass;
    case CrashBreadcrumbField::Target:
        return breadcrumbs.target;
    case CrashBreadcrumbField::Stage:
        break;
    }
    return breadcrumbs.stage;
}

CrashBreadcrumbScope::CrashBreadcrumbScope(CrashBreadcrumbField field, const char* text)
    : CrashBreadcrumbScope(field, text, text ? int(strlen(text)) : 0)
{
}

CrashBreadcrumbScope::CrashBreadcrumbScope(
    CrashBreadcrumbField field,
    const char* chars,
    int length)
    : m_field(field)
{
    CrashBreadcrumbText& current = _getField(CrashBreadcrumbUtil::getCurrentForWriting(), field);
    m_previous = current;
    current.chars = chars;
    current.length = chars ? length : 0;
}

CrashBreadcrumbScope::~CrashBreadcrumbScope()
{
    _getField(CrashBreadcrumbUtil::getCurrentForWriting(), m_field) = m_previous;
}

} // namespace Slang
