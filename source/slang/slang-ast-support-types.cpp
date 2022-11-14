#include "slang-ast-support-types.h"
#include "slang-ast-base.h"
#include "slang-ast-type.h"

namespace Slang
{
QualType::QualType(Type* type)
    : type(type)
    , isLeftValue(false)
{
    if (as<RefType>(type))
    {
        isLeftValue = true;
    }
}

void removeModifier(ModifiableSyntaxNode* syntax, Modifier* toRemove)
{
    Modifier* prev = nullptr;
    for (auto modifier = syntax->modifiers.first; modifier; modifier = modifier->next)
    {
        if (modifier == toRemove)
        {
            if (prev)
            {
                prev->next = modifier->next;
            }
            else
            {
                syntax->modifiers.first = syntax->modifiers.first->next;
            }
            break;
        }
        prev = modifier;
    }
}
}
