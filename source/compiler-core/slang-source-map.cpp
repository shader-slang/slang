#include "slang-source-map.h"

#include "../../slang-com-helper.h"


#include "slang-json-native.h"

namespace Slang {

static const StructRttiInfo _makeJSONSourceMap_Rtti()
{
    JSONSourceMap obj;

    StructRttiBuilder builder(&obj, "SourceMap", nullptr);

    builder.addField("version", &obj.version);
    builder.addField("file", &obj.file);
	builder.addField("sourceRoot", &obj.sourceRoot);
	builder.addField("sources", &obj.sources);
	builder.addField("sourcesContent", &obj.sourcesContent);
	builder.addField("names", &obj.names);
	builder.addField("mappings", &obj.mappings);

    return builder.make();
}
/* static */const StructRttiInfo JSONSourceMap::g_rttiInfo = _makeJSONSourceMap_Rtti();

} // namespace Slang
