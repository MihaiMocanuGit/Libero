#pragma once

#include <cstdint>

namespace lbr::ecs::entity
{
using eid = uint32_t;
constexpr eid neid {static_cast<eid>(-1)};

struct Entity
{
    eid id;
};
} // namespace lbr::ecs::entity

namespace lbr::ecs
{
using SizeEid = entity::eid;
}
