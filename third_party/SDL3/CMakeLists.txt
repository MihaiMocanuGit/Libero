Include(FetchContent)

FetchContent_Declare(
        SDL3 
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG release-3.4.0
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(SDL3)
