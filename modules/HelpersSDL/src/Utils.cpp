#include "Libero/HelpersSDL/Utils.hpp"

namespace lbr::hsdl
{
std::expected<SmartSDL_Texture, std::string_view> loadTexture(SmartSDL_Renderer &renderer,
                                                              const std::filesystem::path &path)
{
    SmartSDL_Texture texture {IMG_LoadTexture(renderer.res, path.c_str())};
    if (not texture.res)
        return std::unexpected(SDL_GetError());
    return texture;
}
} // namespace lbr::hsdl
