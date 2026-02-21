#pragma once

#include <cstdint>
namespace lbr::ecs::entity
{
using eid = uint32_t;
constexpr eid nid {static_cast<eid>(-1)};
struct Entity
{
    eid id;
};
} // namespace lbr::ecs::entity
