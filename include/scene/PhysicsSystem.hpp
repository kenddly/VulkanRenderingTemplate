#pragma once

#include <scene/Components.hpp>
#include <scene/Scene.hpp>
#include <scene/Model.hpp>       // for mesh collider

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include <glm/glm.hpp>
#include <memory>

#include "PhysicsDebugRenderer.hpp"

namespace vks
{
    namespace Layers
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::uint NUM_LAYERS = 2;
    }

    namespace BPLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING{0};
        static constexpr JPH::BroadPhaseLayer MOVING{1};
        static constexpr JPH::uint NUM_LAYERS{2};
    }

    // --------------------------------------------------------
    //  ShapeDesc
    //  Use the static factory helpers:
    //    ShapeDesc::box(hx, hy, hz)
    //    ShapeDesc::sphere(r)
    //    ShapeDesc::capsule(halfHeight, r)
    //    ShapeDesc::mesh(model)
    // --------------------------------------------------------
    struct ShapeDesc
    {
        enum class Type { Box, Sphere, Capsule, Mesh, ConvexHull };

        Type type = Type::Box;
        float halfX = 0.5f;
        float halfY = 0.5f;
        float halfZ = 0.5f;
        float radius = 0.5f;

        // Only set for Type::Mesh — non-owning, must outlive the PhysicsBody
        const Model* model = nullptr;

        static ShapeDesc box(float hx, float hy, float hz)
        {
            return {Type::Box, hx, hy, hz, 0.f, nullptr};
        }

        static ShapeDesc sphere(float r)
        {
            return {Type::Sphere, 0.f, 0.f, 0.f, r, nullptr};
        }

        static ShapeDesc capsule(float halfHeight, float r)
        {
            return {Type::Capsule, 0.f, halfHeight, 0.f, r, nullptr};
        }

        /**
         * @brief Build a mesh collider directly from a Model's CPU geometry.
         *
         * IMPORTANT: Jolt mesh shapes are STATIC only.
         * Always use addStaticBody() with this shape, never addRigidBody().
         * The model must outlive the PhysicsBody (asset lifetime is fine).
         */
        static ShapeDesc mesh(const Model& m)
        {
            ShapeDesc d;
            d.type = Type::Mesh;
            d.model = &m;
            return d;
        }

        /**
        * @brief Convex hull built from the model's vertices.
        * Safe for dynamic bodies. Less accurate than mesh but fully supported.
        */
        static ShapeDesc convexHull(const Model& m)
        {
            ShapeDesc d;
            d.type = Type::ConvexHull;
            d.model = &m;
            return d;
        }
    };

    // --------------------------------------------------------
    //  PhysicsBody component — added automatically by PhysicsSystem
    // --------------------------------------------------------
    struct PhysicsBody
    {
        const JPH::BodyID bodyID;
        bool isStatic = false;

        ShapeDesc originalShape;
        glm::vec3 lastPosition = {0.f, 0.f, 0.f};
        glm::vec3 lastRotation = {0.f, 0.f, 0.f};
        glm::vec3 lastScale = {1.f, 1.f, 1.f};
    };

    // --------------------------------------------------------
    //  PhysicsSystem
    // --------------------------------------------------------
    class PhysicsSystem
    {
    public:
        PhysicsSystem();
        ~PhysicsSystem();

        void onInit(
            uint32_t maxBodies = 1024,
            uint32_t numThreads = 0,
            glm::vec3 gravity = {0.f, -9.81f, 0.f});

        void shutdown();

        // Dynamic rigid body (NOT compatible with Mesh shape)
        void addRigidBody(Scene& scene, Entity entity, const ShapeDesc& shape);

        // Static collider (compatible with ALL shapes including Mesh)
        void addStaticBody(Scene& scene, Entity entity, const ShapeDesc& shape);

        void removeBody(Scene& scene, Entity entity);

        void update(Scene& scene, float dt, int substeps = 1);

        void applyImpulse(Scene& scene, Entity entity, glm::vec3 impulse);
        void setLinearVelocity(Scene& scene, Entity entity, glm::vec3 velocity);
        void setGravityFactor(Scene& scene, Entity entity, float factor);

        JPH::PhysicsSystem& getJoltSystem() { return *m_physicsSystem; }
        PhysicsDebugRenderer& getDebugRenderer() { return m_debugRenderer; }

    private:
        PhysicsDebugRenderer m_debugRenderer;

        std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
        std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
        std::unique_ptr<JPH::PhysicsSystem> m_physicsSystem;

        struct BPLayerInterfaceImpl;
        struct ObjVsBPLayerFilterImpl;
        struct ObjLayerPairFilterImpl;

        std::unique_ptr<BPLayerInterfaceImpl> m_bpLayerInterface;
        std::unique_ptr<ObjVsBPLayerFilterImpl> m_objVsBPFilter;
        std::unique_ptr<ObjLayerPairFilterImpl> m_objLayerPairFilter;

        bool m_initialised = false;

        JPH::ShapeRefC buildShape(const ShapeDesc& desc) const;
        JPH::ShapeRefC buildScaledShape(const ShapeDesc& desc, const glm::vec3& scale) const;
        JPH::Vec3 toJolt(const glm::vec3& v) const;
        JPH::Quat eulerToJoltQuat(const glm::vec3& eulerRad) const;
        glm::vec3 toGLM(const JPH::Vec3& v) const;
        glm::vec3 quatToEuler(const JPH::Quat& q) const;
    };
} // namespace vks
