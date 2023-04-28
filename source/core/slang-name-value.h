#ifndef SLANG_CORE_NAME_VALUE_H
#define SLANG_CORE_NAME_VALUE_H

#include "slang-basic.h"

#include "slang-array-view.h"

namespace Slang
{

// When associating values with names we use this type
typedef int32_t ValueInt;

struct NameValue
{
    ValueInt value;
    const char* name;
};

struct NamesValue
{
    ValueInt value;
    const char* names;          ///< Can hold comma delimited list of names
    
};

struct NamesDescriptionValue
{
    ValueInt value;
    const char* names;          ///< Can hold comma delimited list of names 
    const char* description;    ///< Optional description, can hold null ptr or empty string if not defined
};

struct NameValueUtil
{
        /// Use the default type to infer the actual type desired
    template <typename T>
    static T findValue(const ConstArrayView<NameValue>& opts, const UnownedStringSlice& slice, T defaultValue) { return (T)findValue(opts, slice, ValueInt(defaultValue)); }
    template <typename T>
    static T findValue(const ConstArrayView<NamesValue>& opts, const UnownedStringSlice& slice, T defaultValue) { return (T)findValue(opts, slice, ValueInt(defaultValue)); }
    template <typename T>
    static T findValue(const ConstArrayView<NamesDescriptionValue>& opts, const UnownedStringSlice& slice, T defaultValue) { return (T)findValue(opts, slice, ValueInt(defaultValue)); }

        /// Given a slice finds the associated value. If no entry is found, defaultValue is returned
    static ValueInt findValue(const ConstArrayView<NameValue>& opts, const UnownedStringSlice& slice, ValueInt defaultValue = -1);
    static ValueInt findValue(const ConstArrayView<NamesValue>& opts, const UnownedStringSlice& slice, ValueInt defaultValue = -1);
    static ValueInt findValue(const ConstArrayView<NamesDescriptionValue>& opts, const UnownedStringSlice& slice, ValueInt defaultValue = -1);

        /// Given a value find the name. If there are multiple names, returns the first name
    static UnownedStringSlice findName(const ConstArrayView<NameValue>& opts, ValueInt value, const UnownedStringSlice& defaultName = UnownedStringSlice());
    static UnownedStringSlice findName(const ConstArrayView<NamesValue>& opts, ValueInt value, const UnownedStringSlice& defaultName = UnownedStringSlice());
    static UnownedStringSlice findName(const ConstArrayView<NamesDescriptionValue>& opts, ValueInt value, const UnownedStringSlice& defaultName = UnownedStringSlice());

        /// Get the description
    static UnownedStringSlice findDescription(const ConstArrayView<NamesDescriptionValue>& opts, ValueInt value, const UnownedStringSlice& defaultDescription = UnownedStringSlice());
};

} // namespace Slang

#endif 
