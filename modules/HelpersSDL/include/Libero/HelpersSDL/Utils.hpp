#pragma once
#include "Libero/HelpersSDL/SmartSDL.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3_image/SDL_image.h>
#include <expected>
#include <filesystem>

namespace lbr::hsdl
{

std::expected<SmartSDL_Texture, std::string_view> loadTexture(SmartSDL_Renderer &renderer,
                                                              const std::filesystem::path &path);

} // namespace lbr::hsdl
