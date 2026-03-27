import sys
import json
import os

def generate_cpp_code(namespace_name, parent_header, types):
    # Header and enum definition
    cpp_code = f'''#pragma once

// Recommended usage:
// In the c++ header file in which you define your components, include the generated header at the
// end of the file, outside any namespaces.
// Example:
// namespace cmps  {{
// struct Component1  {{
//     float field;
// }};
// }} // end namespace cmps
//
// #include "generated.hxx"

// Recommendation:
// Create a symlink in your project directory to this script and integrate it into your build
// system.

// Not necessary to compile, but is needed to stop false positive LSP errors.
#include "{parent_header}"
#include "Libero/ECS/Components.hpp"

namespace {namespace_name}
{{
enum class ETypes : lbr::ecs::SizeEType
{{
    ''' + ',\n    '.join([f"{name} = {i}" for i, name in enumerate(types)]) + ',\n    countEType, // Important to be last\n};\n'
    cpp_code += 'static_assert(lbr::ecs::EMetaType<ETypes>, "ETypes is not an EMetaType");\n'
    cpp_code += f'}} // namespace {namespace_name}\n\n'
    cpp_code += f'// Appending into the namespace is important\nnamespace lbr::ecs\n{{\n\n'

    # Generate template specializations for each name
    for i, name in enumerate(types):
        cpp_code += f'''////////////////////////////////////////////////////////////////////////////////////////////////////
// {name.upper()} = {i}
template <>
struct EType2CType<{namespace_name}::ETypes, {namespace_name}::ETypes::{name}>
{{
    using CType = {namespace_name}::{name};
}};
template <>
struct CType2EType<{namespace_name}::ETypes, {namespace_name}::{name}>
{{
    static constexpr {namespace_name}::ETypes EType = {namespace_name}::ETypes::{name};
}};
static_assert(CType<{namespace_name}::{name}, {namespace_name}::ETypes>, "{name} is not a CType");

'''
    cpp_code += '}\n'  # Close namespace
    return cpp_code

def main():
    if len(sys.argv) != 2:
        print("Usage: python generate.py <config_json>")
        sys.exit(1)

    config_path = sys.argv[1]

    # Validate if config file exists
    if not os.path.exists(config_path):
        print(f"Error: Config file not found at '{config_path}'")
        sys.exit(1)

    # Load config from JSON file
    with open(config_path, 'r') as f:
        try:
            config = json.load(f)
        except json.JSONDecodeError as e:
            print(f"Error parsing JSON: {e}")
            sys.exit(1)

    # Validate required fields
    required_fields = ["namespace_name", "parent_header", "types", "output_path"]
    missing_fields = [field for field in required_fields if field not in config]

    if missing_fields:
        print(f"Error: Missing required config fields: {', '.join(missing_fields)}")
        sys.exit(1)

    namespace_name = config["namespace_name"]
    print(f"namespace_name={namespace_name}")

    parent_header = config["parent_header"]
    print(f"parent_header={parent_header}")

    output_path = config["output_path"]
    print(f"output_path={output_path}")

    types = config["types"]
    print(f"types={types}")

    # Check if output file exists; if so, rename it
    if os.path.exists(output_path):
        backup_path = output_path + ".bk"
        try:
            os.rename(output_path, backup_path)
            print(f"Existing file {output_path} renamed to {backup_path}")
        except OSError as e:
            print(f"Error renaming existing file: {e}")
            sys.exit(1)

    # Generate C++ code
    cpp_code = generate_cpp_code(namespace_name, parent_header, types)

    # Write to output file
    try:
        with open(output_path, 'w') as f:
            f.write(cpp_code)
        print(f"Generated C++ boilerplate code in {output_path}")
    except IOError as e:
        print(f"Error writing to output file: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
