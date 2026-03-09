// ============================================================
//  PhysicsSystem.cpp
// ============================================================

#include <scene/PhysicsSystem.hpp>

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Core/JobSystemThreadPool.h>

#include <Jolt/Physics/Collision/Shape/ScaledShape.h>

#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include <stdexcept>
#include <thread>
#include <cassert>
#include <cstdarg>

// Jolt needs these to be defined exactly once
JPH_SUPPRESS_WARNINGS

namespace vks
{
    // ============================================================
    //  Layer filter implementations
    //  These are tiny boilerplate classes Jolt requires.
    // ============================================================

    struct PhysicsSystem::BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
        BPLayerInterfaceImpl()
        {
            m_objectToBP[Layers::NON_MOVING] = BPLayers::NON_MOVING;
            m_objectToBP[Layers::MOVING] = BPLayers::MOVING;
        }

        JPH::uint GetNumBroadPhaseLayers() const override
        {
            return BPLayers::NUM_LAYERS;
        }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
        {
            assert(layer < Layers::NUM_LAYERS);
            return m_objectToBP[layer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
        {
            switch ((JPH::BroadPhaseLayer::Type)layer)
            {
            case (JPH::BroadPhaseLayer::Type)BPLayers::NON_MOVING: return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)BPLayers::MOVING: return "MOVING";
            default: return "UNKNOWN";
            }
        }
#endif

    private:
        JPH::BroadPhaseLayer m_objectToBP[Layers::NUM_LAYERS];
    };

    struct PhysicsSystem::ObjVsBPLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
        bool ShouldCollide(JPH::ObjectLayer objLayer, JPH::BroadPhaseLayer bpLayer) const override
        {
            switch (objLayer)
            {
            case Layers::NON_MOVING:
                // Static objects only need to collide with moving objects
                return bpLayer == BPLayers::MOVING;
            case Layers::MOVING:
                // Dynamic objects collide with everything
                return true;
            default:
                return false;
            }
        }
    };

    struct PhysicsSystem::ObjLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
        bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
        {
            switch (a)
            {
            case Layers::NON_MOVING:
                return b == Layers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                return false;
            }
        }
    };

    static void JoltTraceImpl(const char* inFMT, ...)
    {
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);
        printf("[Jolt] %s\n", buffer);
    }
#ifdef JPH_ENABLE_ASSERTS
    static bool JoltAssertFailed(const char* inExpression, const char* inMessage,
                                 const char* inFile, uint32_t inLine)
    {
        printf("[Jolt Assert] %s:%u: (%s) %s\n",
               inFile, inLine, inExpression, inMessage ? inMessage : "");
        return true; // true = trigger breakpoint
    }
#endif


    // ============================================================
    //  Lifecycle
    // ============================================================

    // Defined here (not in the header) so that the compiler only
    // instantiates unique_ptr<*Impl> destructors after the three
    // Impl structs above are fully defined — avoids the
    // "incomplete type in unique_ptr" error.
    PhysicsSystem::PhysicsSystem() = default;

    PhysicsSystem::~PhysicsSystem()
    {
        if (m_initialised)
            shutdown();
    }

    void PhysicsSystem::onInit(uint32_t maxBodies, uint32_t numThreads, glm::vec3 gravity)
    {
        assert(!m_initialised && "PhysicsSystem::init() called twice");
        // Must be set before RegisterDefaultAllocator()
        JPH::Trace = JoltTraceImpl;

        JPH_IF_ENABLE_ASSERTS(
            JPH::AssertFailed = JoltAssertFailed;
        )

        // --- Jolt global setup (safe to call multiple times) ---
        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();

        // --- Allocators ----------------------------------------
        constexpr size_t TEMP_ALLOC_SIZE = 16 * 1024 * 1024; // 16 MB
        m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(TEMP_ALLOC_SIZE);

        // --- Job system ----------------------------------------
        // Clamp to [1, hardware-1] — never pass 0 threads or Jolt has no
        // workers to drain the queue, triggering "no job available".
        const uint32_t hwThreads = std::thread::hardware_concurrency();
        if (numThreads == 0)
            numThreads = (hwThreads > 1) ? (hwThreads - 1) : 1;
        numThreads = std::max(numThreads, 1u);

        // Use explicit sizes rather than cMaxPhysicsJobs/cMaxPhysicsBarriers —
        // those constants can be too small depending on Jolt build config.
        constexpr uint32_t MAX_JOBS = 2048;
        constexpr uint32_t MAX_BARRIERS = 8;

        m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
            MAX_JOBS,
            MAX_BARRIERS,
            (int)numThreads);

        // --- Layer interfaces ----------------------------------
        m_bpLayerInterface = std::make_unique<BPLayerInterfaceImpl>();
        m_objVsBPFilter = std::make_unique<ObjVsBPLayerFilterImpl>();
        m_objLayerPairFilter = std::make_unique<ObjLayerPairFilterImpl>();

        // --- Physics world -------------------------------------
        m_physicsSystem = std::make_unique<JPH::PhysicsSystem>();
        m_physicsSystem->Init(
            maxBodies,
            0, // body mutexes — 0 = auto
            maxBodies, // max body pairs
            maxBodies, // max contact constraints
            *m_bpLayerInterface,
            *m_objVsBPFilter,
            *m_objLayerPairFilter);

        m_physicsSystem->SetGravity(toJolt(gravity));
        m_debugRenderer.init();

        m_initialised = true;
    }

    void PhysicsSystem::shutdown()
    {
        assert(m_initialised);
        m_debugRenderer.clear();

        JPH::BodyInterface& bi = m_physicsSystem->GetBodyInterface();

        JPH::BodyIDVector bodyIDs;
        m_physicsSystem->GetBodies(bodyIDs);

        for (const JPH::BodyID& id : bodyIDs)
        {
            bi.RemoveBody(id);
            bi.DestroyBody(id);
        }

        m_physicsSystem.reset();
        m_jobSystem.reset();
        m_tempAllocator.reset();
        m_bpLayerInterface.reset();
        m_objVsBPFilter.reset();
        m_objLayerPairFilter.reset();

        // Clean up Jolt global factory
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;

        m_initialised = false;
    }

    // ============================================================
    //  Body registration
    // ============================================================

    void PhysicsSystem::addRigidBody(Scene& scene, Entity entity, const ShapeDesc& shape)
    {
        assert(m_initialised);
        assert(scene.hasComponent<Transform>(entity) && "Entity needs a Transform");
        assert(scene.hasComponent<RigidBody>(entity) && "Entity needs a RigidBody");

        auto& tf = scene.getComponent<Transform>(entity);
        auto& rb = scene.getComponent<RigidBody>(entity);

        JPH::BodyInterface& bi = m_physicsSystem->GetBodyInterface();

        // Flip Y to convert from engine space (Vulkan Y-down) to Jolt space (Y-up)
        JPH::Vec3 joltPos = JPH::Vec3(tf.position.x, -tf.position.y, tf.position.z);

        JPH::BodyCreationSettings settings(
            buildScaledShape(shape, tf.scale).GetPtr(),
            joltPos,
            eulerToJoltQuat(tf.rotation),
            JPH::EMotionType::Dynamic,
            Layers::MOVING);

        // Push the mass from the component into Jolt
        settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMassPropertiesOverride.mMass = rb.mass;

        // Seed velocity from the component (useful for pre-spawned projectiles etc.)
        settings.mLinearVelocity = toJolt(rb.velocity);

        JPH::Body* body = bi.CreateBody(settings);
        if (!body)
            throw std::runtime_error("Jolt: failed to create rigid body (body limit reached?)");

        bi.AddBody(body->GetID(), JPH::EActivation::Activate);

        scene.addComponent<PhysicsBody>(entity, PhysicsBody{
                                            body->GetID(),
                                            false,
                                            shape, // originalShape
                                            tf.position, // lastPosition
                                            tf.rotation,
                                            tf.scale // lastScale
                                        });
    }

    void PhysicsSystem::addStaticBody(Scene& scene, Entity entity, const ShapeDesc& shape)
    {
        assert(m_initialised);
        assert(scene.hasComponent<Transform>(entity) && "Entity needs a Transform");

        auto& tf = scene.getComponent<Transform>(entity);

        JPH::BodyInterface& bi = m_physicsSystem->GetBodyInterface();

        JPH::Vec3 joltPos = JPH::Vec3(tf.position.x, -tf.position.y, tf.position.z);

        JPH::BodyCreationSettings settings(
            buildScaledShape(shape, tf.scale).GetPtr(),
            joltPos,
            eulerToJoltQuat(tf.rotation),
            JPH::EMotionType::Static,
            Layers::NON_MOVING);

        JPH::Body* body = bi.CreateBody(settings);
        if (!body)
            throw std::runtime_error("Jolt: failed to create static body (body limit reached?)");

        bi.AddBody(body->GetID(), JPH::EActivation::DontActivate);

        scene.addComponent<PhysicsBody>(entity, PhysicsBody{
                                            body->GetID(),
                                            true,
                                            shape, // originalShape
                                            tf.position, // lastPosition
                                            tf.rotation, // lastRotation
                                            tf.scale // lastScale
                                        });
    }

    void PhysicsSystem::removeBody(Scene& scene, Entity entity)
    {
        if (!scene.hasComponent<PhysicsBody>(entity))
            return;

        auto& pb = scene.getComponent<PhysicsBody>(entity);
        JPH::BodyInterface& bi = m_physicsSystem->GetBodyInterface();

        bi.RemoveBody(pb.bodyID);
        bi.DestroyBody(pb.bodyID);

        scene.getRegistry().remove<PhysicsBody>(entity);
    }

    // ============================================================
    //  Per-frame update
    // ============================================================

    void PhysicsSystem::update(Scene& scene, float dt, int substeps)
    {
        assert(m_initialised);

        JPH::BodyInterface& bi = m_physicsSystem->GetBodyInterface();

        // 1. Detect editor-driven changes and sync them into Jolt.
        {
            auto view = scene.view<Transform, PhysicsBody>();
            for (auto [entity, tf, pb] : view.each())
            {
                constexpr float kEps = 1e-5f;
                bool posChanged = glm::length(tf.position - pb.lastPosition) > kEps;
                bool rotChanged = glm::length(tf.rotation - pb.lastRotation) > kEps;
                bool scaleChanged = glm::length(tf.scale - pb.lastScale) > kEps;

                if (posChanged || rotChanged)
                {
                    JPH::Vec3 joltPos(tf.position.x, -tf.position.y, tf.position.z);
                    bi.SetPositionAndRotation(
                        pb.bodyID,
                        joltPos,
                        eulerToJoltQuat(tf.rotation),
                        pb.isStatic
                            ? JPH::EActivation::DontActivate
                            : JPH::EActivation::Activate);

                    // Only zero velocity for dynamic bodies
                    if (!pb.isStatic)
                    {
                        bi.SetLinearVelocity(pb.bodyID, JPH::Vec3::sZero());
                        bi.SetAngularVelocity(pb.bodyID, JPH::Vec3::sZero());
                    }

                    pb.lastPosition = tf.position;
                    pb.lastRotation = tf.rotation;
                }

                if (scaleChanged)
                {
                    JPH::ShapeRefC newShape = buildScaledShape(pb.originalShape, tf.scale);
                    bi.SetShape(pb.bodyID, newShape.GetPtr(), false,
                                pb.isStatic
                                    ? JPH::EActivation::DontActivate
                                    : JPH::EActivation::Activate);
                    pb.lastScale = tf.scale;
                }
            }
        }

        // 2. Push acceleration forces from gameplay code
        {
            auto view = scene.view<Transform, RigidBody, PhysicsBody>();
            for (auto [entity, tf, rb, pb] : view.each())
            {
                if (pb.isStatic) continue;
                if (glm::length(rb.acceleration) > 1e-6f)
                {
                    bi.AddForce(pb.bodyID, toJolt(rb.acceleration) * rb.mass);
                    rb.acceleration = {};
                }
            }
        }

        // 3. Step
        m_physicsSystem->Update(dt, substeps, m_tempAllocator.get(), m_jobSystem.get());

        // 4. Write Jolt results back into Transform components
        {
            auto view = scene.view<Transform, PhysicsBody>();
            for (auto [entity, tf, pb] : view.each())
            {
                if (pb.isStatic) continue;

                JPH::Vec3 pos = bi.GetPosition(pb.bodyID);
                JPH::Quat rot = bi.GetRotation(pb.bodyID);

                glm::vec3 rawPos = toGLM(pos);
                tf.position = glm::vec3(rawPos.x, -rawPos.y, rawPos.z);

                glm::vec3 euler = quatToEuler(rot);
                tf.rotation = glm::vec3(-euler.x, euler.y, -euler.z);

                tf.updateTransform();

                pb.lastPosition = tf.position;
                pb.lastRotation = tf.rotation;

                if (scene.hasComponent<RigidBody>(entity))
                    scene.getComponent<RigidBody>(entity).velocity =
                        toGLM(bi.GetLinearVelocity(pb.bodyID));
            }
        }
    }

    // ============================================================
    //  Impulse / force helpers
    // ============================================================

    void PhysicsSystem::applyImpulse(Scene& scene, Entity entity, glm::vec3 impulse)
    {
        assert(scene.hasComponent<PhysicsBody>(entity));
        auto& pb = scene.getComponent<PhysicsBody>(entity);
        m_physicsSystem->GetBodyInterface().AddImpulse(pb.bodyID, toJolt(impulse));
    }

    void PhysicsSystem::setLinearVelocity(Scene& scene, Entity entity, glm::vec3 velocity)
    {
        assert(scene.hasComponent<PhysicsBody>(entity));
        auto& pb = scene.getComponent<PhysicsBody>(entity);
        m_physicsSystem->GetBodyInterface().SetLinearVelocity(pb.bodyID, toJolt(velocity));
    }

    void PhysicsSystem::setGravityFactor(Scene& scene, Entity entity, float factor)
    {
        assert(scene.hasComponent<PhysicsBody>(entity));
        auto& pb = scene.getComponent<PhysicsBody>(entity);
        m_physicsSystem->GetBodyInterface().SetGravityFactor(pb.bodyID, factor);
    }

    // ============================================================
    //  Private helpers
    // ============================================================

    JPH::ShapeRefC PhysicsSystem::buildShape(const ShapeDesc& desc) const
    {
        switch (desc.type)
        {
        case ShapeDesc::Type::Box:
            {
                JPH::BoxShapeSettings settings(JPH::Vec3(desc.halfX, desc.halfY, desc.halfZ));
                return settings.Create().Get();
            }
        case ShapeDesc::Type::Sphere:
            {
                JPH::SphereShapeSettings settings(desc.radius);
                return settings.Create().Get();
            }
        case ShapeDesc::Type::Capsule:
            {
                JPH::CapsuleShapeSettings settings(desc.halfY, desc.radius);
                return settings.Create().Get();
            }
        case ShapeDesc::Type::Mesh:
            {
                assert(desc.model && "ShapeDesc::mesh() requires a valid Model pointer");
                assert(desc.model->hasCPUGeometry() && "Model has no CPU geometry — was it uploaded?");

                const auto& verts = desc.model->getCPUVertices();
                const auto& indices = desc.model->getCPUIndices();

                // Build Jolt vertex list from your Geometry::Vertex positions
                JPH::VertexList joltVerts;
                joltVerts.reserve(verts.size());
                for (const auto& v : verts)
                    joltVerts.push_back(JPH::Float3(v.pos[0], v.pos[1], v.pos[2]));

                // Build indexed triangle list (indices must be in groups of 3)
                assert(indices.size() % 3 == 0 && "Mesh indices must be a multiple of 3");
                JPH::IndexedTriangleList triangles;
                triangles.reserve(indices.size() / 3);
                for (size_t i = 0; i + 2 < indices.size(); i += 3)
                    triangles.push_back(JPH::IndexedTriangle(
                        indices[i], indices[i + 1], indices[i + 2]));

                JPH::MeshShapeSettings settings(std::move(joltVerts), std::move(triangles));
                auto result = settings.Create();
                if (result.HasError())
                    throw std::runtime_error(
                        std::string("Jolt MeshShape error: ") + result.GetError().c_str());

                return result.Get();
            }
        case ShapeDesc::Type::ConvexHull:
            {
                assert(desc.model && "ShapeDesc::convexHull() requires a valid Model pointer");
                assert(desc.model->hasCPUGeometry());

                const auto& verts = desc.model->getCPUVertices();

                JPH::Array<JPH::Vec3> points;
                points.reserve(verts.size());
                for (const auto& v : verts)
                    points.push_back(JPH::Vec3(v.pos[0], v.pos[1], v.pos[2]));

                JPH::ConvexHullShapeSettings settings(points);
                auto result = settings.Create();
                if (result.HasError())
                    throw std::runtime_error(
                        std::string("Jolt ConvexHull error: ") + result.GetError().c_str());

                return result.Get();
            }
        default:
            throw std::runtime_error("PhysicsSystem: unknown shape type");
        }
    }

    JPH::ShapeRefC PhysicsSystem::buildScaledShape(const ShapeDesc& desc, const glm::vec3& scale) const
    {
        JPH::ShapeRefC base = buildShape(desc);

        // Only wrap if scale is meaningfully non-unit
        constexpr float kEps = 1e-4f;
        bool uniform = glm::abs(scale.x - 1.f) < kEps &&
            glm::abs(scale.y - 1.f) < kEps &&
            glm::abs(scale.z - 1.f) < kEps;
        if (uniform) return base;

        JPH::ScaledShapeSettings scaled(base, JPH::Vec3(scale.x, scale.y, scale.z));
        auto result = scaled.Create();
        if (result.HasError())
            throw std::runtime_error(
                std::string("Jolt ScaledShape error: ") + result.GetError().c_str());

        return result.Get();
    }

    JPH::Vec3 PhysicsSystem::toJolt(const glm::vec3& v) const
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }

    JPH::Quat PhysicsSystem::eulerToJoltQuat(const glm::vec3& eulerRad) const
    {
        glm::quat q = glm::quat(eulerRad); // glm expects pitch/yaw/roll in radians
        return JPH::Quat(q.x, q.y, q.z, q.w);
    }

    glm::vec3 PhysicsSystem::toGLM(const JPH::Vec3& v) const
    {
        return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
    }

    glm::vec3 PhysicsSystem::quatToEuler(const JPH::Quat& q) const
    {
        glm::quat gq(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
        return glm::eulerAngles(gq); // returns (pitch, yaw, roll) in radians
    }
} // namespace vks
