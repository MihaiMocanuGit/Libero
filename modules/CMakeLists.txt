set(library_name "modules")
set(module_names "Utilities;SmartResource;HelpersSDL;ECS")

foreach(module ${module_names})
    add_subdirectory(${module})
endforeach()

add_library(${library_name} INTERFACE)
target_link_libraries(${library_name} INTERFACE ${module_names})
target_include_directories(${library_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
