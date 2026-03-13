#pragma once

#include <cstdint>

namespace lbr::ecs
{

struct Entity
{
    using eid = uint32_t;
    static constexpr eid neid {static_cast<eid>(-1)};

    eid id;
};

using SizeEid = Entity::eid;

} // namespace lbr::ecs
