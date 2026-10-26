// Compile-and-link checks for the optional technology stack. Each section only
// builds when its ENGINE_WITH_<NAME> option is on (see the "full" preset).

#include <catch2/catch_test_macros.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/vec3.hpp>

#ifdef ENGINE_HAS_ENTT
#include <entt/entity/registry.hpp>
#endif
#ifdef ENGINE_HAS_BOX2D
#include <box2d/box2d.h>
#endif
#ifdef ENGINE_HAS_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#endif
#ifdef ENGINE_HAS_MINIAUDIO
#include <miniaudio.h>
#endif
#ifdef ENGINE_HAS_IMGUI
#include <imgui.h>
#endif
#ifdef ENGINE_HAS_STB
#include <stb_image.h>
#endif
#ifdef ENGINE_HAS_JSON
#include <nlohmann/json.hpp>
#endif
#ifdef ENGINE_HAS_SPDLOG
#include <spdlog/spdlog.h>
#endif
#ifdef ENGINE_HAS_TRACY
#include <tracy/Tracy.hpp>
#endif

namespace {
// Shared components from the engine design notes.
struct Transform {
    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};
};
struct Velocity {
    glm::vec3 linear{0.0f};
};
} // namespace

#ifdef ENGINE_HAS_ENTT
TEST_CASE("EnTT view iterates components", "[thirdparty]")
{
    entt::registry registry;
    const auto entity = registry.create();
    registry.emplace<Transform>(entity);
    registry.emplace<Velocity>(entity, glm::vec3{2.0f, 0.0f, 0.0f});

    const double dt = 0.5;
    auto view = registry.view<Transform, Velocity>();
    for (auto e : view) {
        auto& transform = view.get<Transform>(e);
        auto& velocity = view.get<Velocity>(e);
        transform.position += velocity.linear * static_cast<float>(dt);
    }
    CHECK(registry.get<Transform>(entity).position.x == 1.0f);
}
#endif

#ifdef ENGINE_HAS_BOX2D
TEST_CASE("Box2D world steps", "[thirdparty]")
{
    b2WorldDef def = b2DefaultWorldDef();
    const b2WorldId world = b2CreateWorld(&def);
    b2World_Step(world, 1.0f / 60.0f, 4);
    CHECK(b2World_IsValid(world));
    b2DestroyWorld(world);
}
#endif

#ifdef ENGINE_HAS_JOLT
TEST_CASE("Jolt registers its types", "[thirdparty]")
{
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    JPH::UnregisterTypes();
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
    SUCCEED();
}
#endif

#ifdef ENGINE_HAS_MINIAUDIO
TEST_CASE("miniaudio links", "[thirdparty]")
{
    CHECK(ma_version_string() != nullptr);
}
#endif

#ifdef ENGINE_HAS_IMGUI
TEST_CASE("Dear ImGui context", "[thirdparty]")
{
    IMGUI_CHECKVERSION();
    ImGuiContext* context = ImGui::CreateContext();
    CHECK(context != nullptr);
    ImGui::DestroyContext(context);
}
#endif

#ifdef ENGINE_HAS_JSON
TEST_CASE("nlohmann/json round-trips", "[thirdparty]")
{
    const auto value = nlohmann::json::parse(R"({"lives": 3})");
    CHECK(value.at("lives").get<int>() == 3);
}
#endif

#ifdef ENGINE_HAS_SPDLOG
TEST_CASE("spdlog logs", "[thirdparty]")
{
    spdlog::info("spdlog is available");
    SUCCEED();
}
#endif

#ifdef ENGINE_HAS_TRACY
TEST_CASE("Tracy zone compiles", "[thirdparty]")
{
    ZoneScoped;
    SUCCEED();
}
#endif

TEST_CASE("GLM quaternion identity", "[thirdparty]")
{
    const Transform transform;
    CHECK(transform.rotation.w == 1.0f);
}
