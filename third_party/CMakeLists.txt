add_subdirectory(Catch2)
add_subdirectory(SDL3)
add_subdirectory(Boost)

add_library(third_party INTERFACE)
target_link_libraries(third_party INTERFACE Catch2::Catch2WithMain SDL3::SDL3 Boost::container)
target_include_directories(third_party INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
