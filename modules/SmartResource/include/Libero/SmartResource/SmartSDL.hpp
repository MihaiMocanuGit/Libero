
#include "Libero/SmartResource/SmartResource.hpp"

#include <SDL3/SDL_render.h>

namespace lbr::smartresource
{
// TODO: A separate SDL wrapper module?
using SmartSDL_Texture = SmartResource<SDL_Texture, SDL_DestroyTexture>;
using SmartSDL_Surface = SmartResource<SDL_Surface, SDL_DestroySurface>;
using SmartSDL_Window = SmartResource<SDL_Window, SDL_DestroyWindow>;
using SmartSDL_Renderer = SmartResource<SDL_Renderer, SDL_DestroyRenderer>;
} // namespace lbr::smartresource
