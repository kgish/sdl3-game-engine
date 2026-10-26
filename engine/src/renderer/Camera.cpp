#include <glm/ext/matrix_clip_space.hpp>

#include "engine/Renderer.hpp"

namespace engine {

glm::mat4 Camera2D::viewProjection() const
{
    // top < bottom in world units gives a +Y-down screen space.
    return glm::ortho(position.x, position.x + size.x, position.y + size.y, position.y, -1.0f, 1.0f);
}

} // namespace engine
