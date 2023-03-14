#ifndef SLANG_COMPILER_CORE_SOURCE_MAP_H
#define SLANG_COMPILER_CORE_SOURCE_MAP_H

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-string.h"
#include "../core/slang-list.h"
#include "../core/slang-rtti-info.h"

#include "slang-json-value.h"

namespace Slang {

/* 
Support for source maps. Source maps provide a standardized mechanism to associate a location in one output file 
with another.

* [Source Map Proposal](https://docs.google.com/document/d/1U1RGAehQwRypUTovF1KRlpiOFze0b-_2gc6fAH0KY0k/edit?hl=en_US&pli=1&pli=1)
* [Chrome Source Map post](https://developer.chrome.com/blog/sourcemaps/)

Example...

{
"version" : 3,
"file": "out.js",
"sourceRoot": "",
"sources": ["foo.js", "bar.js"],
"sourcesContent": [null, null],
"names": ["src", "maps", "are", "fun"],
"mappings": "A,AAAB;;ABCDE;"
}
*/

struct JSONSourceMap
{
		/// File version (always the first entry in the object) and must be a positive integer.
	int32_t version = 3;					
		/// An optional name of the generated code that this source map is associated with.
	String file;							
		/// An optional source root, useful for relocating source files on a server or removing repeated values in 
		/// the “sources” entry.  This value is prepended to the individual entries in the “source” field.
	String sourceRoot;						
		/// A list of original sources used by the “mappings” entry.
	List<UnownedStringSlice> sources;			
		/// An optional list of source content, useful when the “source” can’t be hosted. The contents are listed in the same order as the sources in line 5. 
		/// “null” may be used if some original sources should be retrieved by name.
		/// Because could be a string or nullptr, we use JSONValue to hold value.
	List<JSONValue> sourcesContent;	
		/// A list of symbol names used by the “mappings” entry.
	List<UnownedStringSlice> names;
		/// A string with the encoded mapping data.
	UnownedStringSlice mappings;

	static const StructRttiInfo g_rttiInfo;
};

} // namespace Slang

#endif // SLANG_COMPILER_CORE_SOURCE_MAP_H
