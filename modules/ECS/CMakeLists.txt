set(module_name ECS)

set(public_headers "Components.hpp;Entity.hpp;Lookup.hpp")
set(private_headers "")
set(detail_headers "Lookup.hxx")
set(sources "Lookup.cpp")
set(tests "Test_Lookup.cpp")

set(public_deps "Utilities")
set(private_deps "third_party")
set(test_deps "")

set(include_dir "${CMAKE_CURRENT_SOURCE_DIR}/include/")
set(private_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/private/")
set(detail_dir "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/${module_name}/detail/")
set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set(test_dir "${CMAKE_CURRENT_SOURCE_DIR}/tests/")

list(TRANSFORM public_headers PREPEND "${include_dir}/${PROJECT_NAME}/${module_name}/")
list(TRANSFORM private_headers PREPEND "${private_dir}")
list(TRANSFORM detail_headers PREPEND "${detail_dir}")
list(TRANSFORM sources PREPEND "${src_dir}")
list(TRANSFORM tests PREPEND "${test_dir}")

add_library(${module_name} STATIC ${public_headers} ${detail_headers} ${private_headers} ${sources})
target_include_directories(${module_name} PUBLIC ${include_dir}
    PRIVATE ${private_headers})
target_link_libraries(${module_name} PUBLIC ${public_deps}
    PRIVATE ${private_deps})

set_target_properties(${module_name} PROPERTIES LINKER_LANGUAGE CXX)

target_sources(unit_tests PUBLIC ${tests})
target_link_libraries(unit_tests PRIVATE ${module_name} ${test_deps})
