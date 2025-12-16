-- Lua helper functions for generating rich diagnostic C++ structures
-- This file is used by FIDDLE templates in slang-rich-diagnostics.h

-- Load the diagnostic definitions
local diagnostics_module = dofile("source/slang/slang-diagnostics.lua")

local M = {}

-- Helper function to convert space-separated name to PascalCase
local function toPascalCase(name)
    local result = ""
    for word in name:gmatch("%S+") do
        result = result .. word:sub(1,1):upper() .. word:sub(2):lower()
    end
    return result
end

-- Helper function to convert space-separated name to snake_case
local function toSnakeCase(name)
    return name:gsub("%s+", "_"):lower()
end

-- Helper function to convert C++ type names for parameters
local function getCppType(lua_type)
    if lua_type == "string" then
        return "String"
    elseif lua_type == "type" then
        return "Type*"
    elseif lua_type == "int" then
        return "int"
    else
        return lua_type -- fallback
    end
end

-- Generate parameter struct for a diagnostic
function M.generateParamStruct(diagnostic)
    local class_name = toPascalCase(diagnostic.name) .. "Params"
    local lines = {}
    
    table.insert(lines, "struct " .. class_name)
    table.insert(lines, "{")
    
    -- Extract parameters from message_parts
    local seen_params = {}
    for _, part in ipairs(diagnostic.message_parts) do
        if part.type == "interpolation" and not seen_params[part.param_name] then
            seen_params[part.param_name] = true
            local cpp_type = getCppType(part.param_type)
            table.insert(lines, "    " .. cpp_type .. " " .. part.param_name .. ";")
        end
    end
    
    -- Add span locations
    table.insert(lines, "    SourceLoc " .. diagnostic.primary_span.location .. ";")
    
    if diagnostic.secondary_spans then
        for _, span in ipairs(diagnostic.secondary_spans) do
            table.insert(lines, "    SourceLoc " .. span.location .. ";")
        end
    end
    
    table.insert(lines, "};")
    table.insert(lines, "")
    
    return table.concat(lines, "\n")
end

-- Generate diagnostic builder struct 
function M.generateBuilderStruct(diagnostic)
    local class_name = toPascalCase(diagnostic.name) .. "Builder"
    local param_struct = toPascalCase(diagnostic.name) .. "Params"
    local lines = {}
    
    table.insert(lines, "struct " .. class_name)
    table.insert(lines, "{")
    table.insert(lines, "    " .. param_struct .. " params;")
    table.insert(lines, "    DiagnosticSink* sink;")
    table.insert(lines, "")
    table.insert(lines, "    " .. class_name .. "(DiagnosticSink* _sink) : sink(_sink) {}")
    table.insert(lines, "")
    
    -- Generate setter methods for each parameter
    local seen_params = {}
    for _, part in ipairs(diagnostic.message_parts) do
        if part.type == "interpolation" and not seen_params[part.param_name] then
            seen_params[part.param_name] = true
            local cpp_type = getCppType(part.param_type)
            table.insert(lines, "    " .. class_name .. "& " .. part.param_name .. "(" .. cpp_type .. " value) {")
            table.insert(lines, "        params." .. part.param_name .. " = value;")
            table.insert(lines, "        return *this;")
            table.insert(lines, "    }")
            table.insert(lines, "")
        end
    end
    
    -- Generate setter methods for span locations
    table.insert(lines, "    " .. class_name .. "& " .. diagnostic.primary_span.location .. "(SourceLoc loc) {")
    table.insert(lines, "        params." .. diagnostic.primary_span.location .. " = loc;")
    table.insert(lines, "        return *this;")
    table.insert(lines, "    }")
    table.insert(lines, "")
    
    if diagnostic.secondary_spans then
        for _, span in ipairs(diagnostic.secondary_spans) do
            table.insert(lines, "    " .. class_name .. "& " .. span.location .. "(SourceLoc loc) {")
            table.insert(lines, "        params." .. span.location .. " = loc;")
            table.insert(lines, "        return *this;")
            table.insert(lines, "    }")
            table.insert(lines, "")
        end
    end
    
    -- Generate build method
    table.insert(lines, "    void build() {")
    table.insert(lines, "        // TODO: Create RichDiagnostic and emit it")
    table.insert(lines, "        // For now, fall back to traditional diagnostic")
    table.insert(lines, "        sink->diagnose(params." .. diagnostic.primary_span.location .. ", Diagnostics::" .. toSnakeCase(diagnostic.name):gsub("_", "") .. ", ")
    
    -- Add parameters in order they appear in message
    local param_list = {}
    for _, part in ipairs(diagnostic.message_parts) do
        if part.type == "interpolation" then
            table.insert(param_list, "params." .. part.param_name)
        end
    end
    
    table.insert(lines, "            " .. table.concat(param_list, ", ") .. ");")
    table.insert(lines, "    }")
    table.insert(lines, "};")
    table.insert(lines, "")
    
    return table.concat(lines, "\n")
end

-- Generate all diagnostic structures
function M.generateAllDiagnostics()
    local lines = {}
    
    for _, diagnostic in ipairs(diagnostics_module) do
        table.insert(lines, "// " .. diagnostic.name .. " (code " .. diagnostic.code .. ")")
        table.insert(lines, M.generateParamStruct(diagnostic))
        table.insert(lines, M.generateBuilderStruct(diagnostic))
    end
    
    return table.concat(lines, "\n")
end

-- Generate helper factory functions
function M.generateFactoryFunctions()
    local lines = {}
    
    table.insert(lines, "// Factory functions for creating diagnostic builders")
    table.insert(lines, "namespace RichDiagnostics {")
    table.insert(lines, "")
    
    for _, diagnostic in ipairs(diagnostics_module) do
        local builder_class = toPascalCase(diagnostic.name) .. "Builder"
        local function_name = toSnakeCase(diagnostic.name)
        
        table.insert(lines, "inline " .. builder_class .. " " .. function_name .. "(DiagnosticSink* sink) {")
        table.insert(lines, "    return " .. builder_class .. "(sink);")
        table.insert(lines, "}")
        table.insert(lines, "")
    end
    
    table.insert(lines, "} // namespace RichDiagnostics")
    
    return table.concat(lines, "\n")
end

return M