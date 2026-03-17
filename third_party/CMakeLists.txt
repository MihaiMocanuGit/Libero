###################################################################################################
# Boost
###################################################################################################
set(Boost_version "1.88.0")
set(Boost_components container)
set(Boost_header_only_components)
set(BOOST_ENABLE_CMAKE ON)
find_package(Boost ${Boost_version} QUIET CONFIG COMPONENTS ${Boost_components} ${Boost_header_only_components})
if(Boost_FOUND)
    message(STATUS "Boost was found.")
else()
    message(STATUS "Boost was not found, fetching...")
    include(FetchContent)
    FetchContent_Declare(Boost
        GIT_REPOSITORY "https://github.com/boostorg/boost.git"
        GIT_TAG "boost-${Boost_version}"
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
    FetchContent_MakeAvailable(Boost)
endif()
set(Boost_targets ${Boost_components})
list(TRANSFORM Boost_targets PREPEND "Boost::")
message(STATUS "Boost_targets = ${Boost_targets}")

###################################################################################################
# Catch2v3
###################################################################################################
set(Catch2_version "3.4.0")
find_package(Catch2 ${Catch2_version} CONFIG QUIET)
if(Catch2_FOUND)
    message(STATUS "Catch2 was found.")
else()
    message(STATUS "Catch2 was not found, fetching...")
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
        GIT_TAG "v${Catch2_version}"
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()
include(CTest)
include(Catch)

###################################################################################################
# SDL3
###################################################################################################
set(SDL3_version "3.4.0")
find_package(SDL3 ${SDL3_version} QUIET CONFIG COMPONENTS SDL3)
if(SDL3_FOUND)
    message(STATUS "SDL3 was found.")
else()
    message(STATUS "SDL3 was not found, fetching...")
    Include(FetchContent)
    FetchContent_Declare(SDL3 
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL"
        GIT_TAG "release-${SDL3_version}"
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL)
    FetchContent_MakeAvailable(SDL3)
endif()

###################################################################################################
# SDL3_image
###################################################################################################
set(SDL3_image_version "3.4.0")
find_package(SDL3_image ${SDL3_image_version} QUIET)
if(SDL3_image_FOUND)
    message(STATUS "SDL3_image was found.")
else()
    message(STATUS "SDL3_image was not found, fetching...")
    Include(FetchContent)
    FetchContent_Declare(SDL3_image
        GIT_REPOSITORY "https://github.com/libsdl-org/SDL_image"
        GIT_TAG "release-${SDL3_image_version}"
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL)
    FetchContent_MakeAvailable(SDL3_image)
endif()

###################################################################################################
# Linking all into third_party
###################################################################################################
add_library(third_party INTERFACE)
target_link_libraries(third_party INTERFACE Catch2::Catch2WithMain SDL3_image::SDL3_image SDL3::SDL3 ${Boost_targets})
target_include_directories(third_party INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
