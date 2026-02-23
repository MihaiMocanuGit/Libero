
#include "Libero/Utilities/SmartResource.hpp"

#include <SDL3/SDL_render.h>

namespace lbr::hsdl
{
using SmartSDL_Texture = lbr::utl::SmartResource<SDL_Texture, SDL_DestroyTexture>;
using SmartSDL_Surface = lbr::utl::SmartResource<SDL_Surface, SDL_DestroySurface>;
using SmartSDL_Window = lbr::utl::SmartResource<SDL_Window, SDL_DestroyWindow>;
using SmartSDL_Renderer = lbr::utl::SmartResource<SDL_Renderer, SDL_DestroyRenderer>;
} // namespace lbr::hsdl
