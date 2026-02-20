#include "ECS/Lookup.hpp"

#include "ECS/Entity.hpp"

namespace lbr::ecs::lookup
{
Lookup::Lookup(uint32_t entitiesReserve, uint32_t componentsReserve) noexcept
{
    m_entities.data.reserve(entitiesReserve);
    std::apply(
        [componentsReserve](auto &&...args)
        {
            ((args.components.reserve(componentsReserve)), ...);
            ((args.entityRefs.reserve(componentsReserve)), ...);
        },
        m_components);
}
} // namespace lbr::ecs::lookup
