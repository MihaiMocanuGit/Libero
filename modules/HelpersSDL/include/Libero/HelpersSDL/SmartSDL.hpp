
#include "Libero/SmartResource/SmartResource.hpp"

#include <SDL3/SDL_render.h>

namespace lbr::hsdl
{
using SmartSDL_Texture = lbr::smartresource::SmartResource<SDL_Texture, SDL_DestroyTexture>;
using SmartSDL_Surface = lbr::smartresource::SmartResource<SDL_Surface, SDL_DestroySurface>;
using SmartSDL_Window = lbr::smartresource::SmartResource<SDL_Window, SDL_DestroyWindow>;
using SmartSDL_Renderer = lbr::smartresource::SmartResource<SDL_Renderer, SDL_DestroyRenderer>;
} // namespace lbr::hsdl
