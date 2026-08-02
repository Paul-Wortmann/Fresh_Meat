#include "animation_system.hpp"
#include "model_system.hpp"

#include <algorithm>
#include <cassert>

//--------------------------------------
// AnimationInstance
//--------------------------------------
void AnimationInstance::update(float dt, const cModelSystem* modelSystem)
{
    current.time += dt;

    if (next.animationIndex >= 0)
    {
        next.time += dt;
        stateMachine.update(dt);

        if (!stateMachine.isBlending())
        {
            current = next;
            next.animationIndex = -1;
        }
    }
}

void AnimationInstance::transitionTo(int animationIndex, float duration, bool loop)
{
    next.animationIndex = animationIndex;
    next.time = 0.0f;
    next.loop = loop;
    stateMachine.startTransition(duration);
}

//--------------------------------------
// System
//--------------------------------------
bool cAnimationSystem::initialize()
{
    return true;
}

void cAnimationSystem::terminate()
{
    m_instances.clear();
}

void cAnimationSystem::update(float dt)
{
    for (auto& [id, inst] : m_instances)
        inst.update(dt, m_modelSystem);
}

//--------------------------------------
// Control
//--------------------------------------
void cAnimationSystem::playAnimation(int modelComponentIndex, int animationIndex, bool loop)
{
    AnimationInstance& inst = m_instances[modelComponentIndex];
    inst.modelComponentIndex = modelComponentIndex;

    inst.current.animationIndex = animationIndex;
    inst.current.time = 0.0f;
    inst.current.loop = loop;

    inst.next.animationIndex = -1;
}

void cAnimationSystem::transitionTo(int modelComponentIndex, int animationIndex, float duration, bool loop)
{
    auto it = m_instances.find(modelComponentIndex);
    if (it == m_instances.end()) return;

    it->second.transitionTo(animationIndex, duration, loop);
}

void cAnimationSystem::stopAnimation(int modelComponentIndex)
{
    m_instances.erase(modelComponentIndex);
}

//--------------------------------------
// Pose evaluation
//--------------------------------------
void cAnimationSystem::evaluatePose(
    const sComponentModel& model,
    const sAnimation& anim,
    float time,
    std::vector<NodeTRS>& outPose) const
{
    size_t count = model.node.size();
    outPose.resize(count);

    // base pose
    for (size_t i = 0; i < count; ++i)
    {
        outPose[i].translation = model.node[i].translation;
        outPose[i].rotation    = model.node[i].rotation;
        outPose[i].scale       = model.node[i].scale;
    }

    for (const auto& channel : anim.channels)
    {
        const auto& sampler = anim.samplers[channel.sampler];
        int node = channel.targetNode;

        switch (channel.targetPath)
        {
        case eAnimTargetPath::translation:
            if (!sampler.translations.empty())
                outPose[node].translation =
                    interpolateVec3(time, sampler.times, sampler.translations);
            break;

        case eAnimTargetPath::rotation:
            if (!sampler.rotations.empty())
                outPose[node].rotation =
                    interpolateQuat(time, sampler.times, sampler.rotations);
            break;

        case eAnimTargetPath::scale:
            if (!sampler.scales.empty())
                outPose[node].scale =
                    interpolateVec3(time, sampler.times, sampler.scales);
            break;
        }
    }
}

void cAnimationSystem::blendPoses(
    const std::vector<NodeTRS>& a,
    const std::vector<NodeTRS>& b,
    float t,
    std::vector<NodeTRS>& out) const
{
    size_t count = a.size();
    out.resize(count);

    for (size_t i = 0; i < count; ++i)
    {
        out[i].translation = glm::mix(a[i].translation, b[i].translation, t);
        out[i].scale       = glm::mix(a[i].scale, b[i].scale, t);
        out[i].rotation    = glm::slerp(a[i].rotation, b[i].rotation, t);
    }
}

//--------------------------------------
// Matrices
//--------------------------------------
void cAnimationSystem::buildLocalMatrices(
    const std::vector<NodeTRS>& pose,
    std::vector<glm::mat4>& outLocal) const
{
    size_t count = pose.size();
    outLocal.resize(count);

    for (size_t i = 0; i < count; ++i)
    {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), pose[i].translation);
        m *= glm::mat4_cast(pose[i].rotation);
        m = glm::scale(m, pose[i].scale);
        outLocal[i] = m;
    }
}

void cAnimationSystem::computeGlobalTransforms(
    const sComponentModel& model,
    int node,
    const glm::mat4& parent,
    std::vector<glm::mat4>& out) const
{
    glm::mat4 global = parent * m_localMatrices[node];
    out[node] = global;

    for (int child : model.node[node].children)
        computeGlobalTransforms(model, child, global, out);
}

//--------------------------------------
// Main output
//--------------------------------------
std::vector<glm::mat4> cAnimationSystem::getJointMatrices(int modelComponentIndex) const
{
    assert(m_modelSystem);

    const auto& model = m_modelSystem->getComponent(modelComponentIndex);
    auto it = m_instances.find(modelComponentIndex);

    if (it == m_instances.end())
        return {};

    const AnimationInstance& inst = it->second;

    const auto& animA = m_modelSystem->getAnimation(modelComponentIndex, inst.current.animationIndex);

    evaluatePose(model, animA, inst.current.time, m_poseA);

    float blend = inst.stateMachine.getBlendFactor();

    if (inst.next.animationIndex >= 0)
    {
        const auto& animB = m_modelSystem->getAnimation(modelComponentIndex, inst.next.animationIndex);

        evaluatePose(model, animB, inst.next.time, m_poseB);
        blendPoses(m_poseA, m_poseB, blend, m_finalPose);
    }
    else
    {
        m_finalPose = m_poseA;
    }

    buildLocalMatrices(m_finalPose, m_localMatrices);

    m_globalMatrices.resize(model.node.size());
    computeGlobalTransforms(model, model.rootNode, glm::mat4(1.0f), m_globalMatrices);

    std::vector<glm::mat4> joints(model.skin.joints.size());

    for (size_t i = 0; i < joints.size(); ++i)
    {
        int node = model.skin.joints[i];
        joints[i] = m_globalMatrices[node] * model.skin.inverseBindMatrices[i];
    }

    return joints;
}

//--------------------------------------
// Interpolation (binary search)
//--------------------------------------
glm::vec3 cAnimationSystem::interpolateVec3(
    float time,
    const std::vector<float>& times,
    const std::vector<glm::vec3>& values) const
{
    if (time <= times.front()) return values.front();
    if (time >= times.back()) return values.back();

    auto it = std::upper_bound(times.begin(), times.end(), time);
    size_t i = std::distance(times.begin(), it);

    float t0 = times[i - 1];
    float t1 = times[i];

    float factor = (time - t0) / (t1 - t0);

    return glm::mix(values[i - 1], values[i], factor);
}

glm::quat cAnimationSystem::interpolateQuat(
    float time,
    const std::vector<float>& times,
    const std::vector<glm::quat>& values) const
{
    if (time <= times.front()) return values.front();
    if (time >= times.back()) return values.back();

    auto it = std::upper_bound(times.begin(), times.end(), time);
    size_t i = std::distance(times.begin(), it);

    float t0 = times[i - 1];
    float t1 = times[i];

    float factor = (time - t0) / (t1 - t0);

    return glm::slerp(values[i - 1], values[i], factor);
}
