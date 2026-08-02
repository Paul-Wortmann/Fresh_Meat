
#include "model_gltf.hpp"

#include "tiny_gltf.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// -----------------------------------------------------------------------------
// Helper: read float scalars from an accessor
// -----------------------------------------------------------------------------
static void ReadFloatAccessor(const tinygltf::Model& model,
                              const tinygltf::Accessor& acc,
                              std::vector<float>& out)
{
    out.resize(acc.count);
    const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = model.buffers[bv.buffer];
    const unsigned char* base = buf.data.data() + bv.byteOffset + acc.byteOffset;
    size_t stride = acc.ByteStride(bv);
    if (stride == 0) stride = sizeof(float);

    for (size_t i = 0; i < acc.count; ++i)
    {
        out[i] = *reinterpret_cast<const float*>(base + i * stride);
    }
}

// -----------------------------------------------------------------------------
// Helper: read vec3 from an accessor
// -----------------------------------------------------------------------------
static void ReadVec3Accessor(const tinygltf::Model& model,
                             const tinygltf::Accessor& acc,
                             std::vector<glm::vec3>& out)
{
    out.resize(acc.count);
    const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = model.buffers[bv.buffer];
    const unsigned char* base = buf.data.data() + bv.byteOffset + acc.byteOffset;
    size_t stride = acc.ByteStride(bv);
    if (stride == 0) stride = 3 * sizeof(float);

    for (size_t i = 0; i < acc.count; ++i)
    {
        const float* p = reinterpret_cast<const float*>(base + i * stride);
        out[i] = glm::vec3(p[0], p[1], p[2]);
    }
}

// -----------------------------------------------------------------------------
// Helper: read vec2 from an accessor
// -----------------------------------------------------------------------------
static void ReadVec2Accessor(const tinygltf::Model& model,
                             const tinygltf::Accessor& acc,
                             std::vector<glm::vec2>& out)
{
    out.resize(acc.count);
    const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = model.buffers[bv.buffer];
    const unsigned char* base = buf.data.data() + bv.byteOffset + acc.byteOffset;
    size_t stride = acc.ByteStride(bv);
    if (stride == 0) stride = 2 * sizeof(float);

    for (size_t i = 0; i < acc.count; ++i)
    {
        const float* p = reinterpret_cast<const float*>(base + i * stride);
        out[i] = glm::vec2(p[0], p[1]);
    }
}

// -----------------------------------------------------------------------------
// Helper: read quaternion (x,y,z,w) from an accessor and store as glm::quat
// -----------------------------------------------------------------------------
static void ReadQuatAccessor(const tinygltf::Model& model,
                             const tinygltf::Accessor& acc,
                             std::vector<glm::quat>& out)
{
    out.resize(acc.count);
    const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = model.buffers[bv.buffer];
    const unsigned char* base = buf.data.data() + bv.byteOffset + acc.byteOffset;
    size_t stride = acc.ByteStride(bv);
    if (stride == 0) stride = 4 * sizeof(float);

    for (size_t i = 0; i < acc.count; ++i)
    {
        const float* p = reinterpret_cast<const float*>(base + i * stride);
        // glTF quaternion: x,y,z,w   glm::quat constructor: w,x,y,z
        out[i] = glm::quat(p[3], p[0], p[1], p[2]);
    }
}

// -----------------------------------------------------------------------------
// Helper: read ivec4 (joints) from an accessor
// -----------------------------------------------------------------------------
static void ReadIVec4Accessor(const tinygltf::Model& model,
                              const tinygltf::Accessor& acc,
                              std::vector<glm::ivec4>& out)
{
    out.resize(acc.count);
    const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = model.buffers[bv.buffer];
    const unsigned char* base = buf.data.data() + bv.byteOffset + acc.byteOffset;
    size_t stride = acc.ByteStride(bv);
    if (stride == 0)
    {
        if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
            stride = 4 * sizeof(uint16_t);
        else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
            stride = 4 * sizeof(uint8_t);
        else
            stride = 4 * sizeof(uint32_t); // fallback
    }

    for (size_t i = 0; i < acc.count; ++i)
    {
        if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            const uint16_t* p = reinterpret_cast<const uint16_t*>(base + i * stride);
            out[i] = glm::ivec4(p[0], p[1], p[2], p[3]);
        }
        else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
        {
            const uint8_t* p = reinterpret_cast<const uint8_t*>(base + i * stride);
            out[i] = glm::ivec4(p[0], p[1], p[2], p[3]);
        }
        else // assume uint32_t
        {
            const uint32_t* p = reinterpret_cast<const uint32_t*>(base + i * stride);
            out[i] = glm::ivec4(p[0], p[1], p[2], p[3]);
        }
    }
}

// -----------------------------------------------------------------------------
// Helper: read vec4 (weights) from an accessor
// -----------------------------------------------------------------------------
static void ReadVec4Accessor(const tinygltf::Model& model,
                             const tinygltf::Accessor& acc,
                             std::vector<glm::vec4>& out)
{
    out.resize(acc.count);
    const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = model.buffers[bv.buffer];
    const unsigned char* base = buf.data.data() + bv.byteOffset + acc.byteOffset;
    size_t stride = acc.ByteStride(bv);
    if (stride == 0) stride = 4 * sizeof(float);

    for (size_t i = 0; i < acc.count; ++i)
    {
        const float* p = reinterpret_cast<const float*>(base + i * stride);
        out[i] = glm::vec4(p[0], p[1], p[2], p[3]);
    }
}

// -----------------------------------------------------------------------------
// Helper: read indices (uint32_t)
// -----------------------------------------------------------------------------
static void ReadIndexAccessor(const tinygltf::Model& model,
                              const tinygltf::Accessor& acc,
                              std::vector<uint32_t>& out)
{
    out.resize(acc.count);
    const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
    const tinygltf::Buffer& buf = model.buffers[bv.buffer];
    const unsigned char* base = buf.data.data() + bv.byteOffset + acc.byteOffset;
    size_t stride = acc.ByteStride(bv);
    if (stride == 0)
        stride = (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) ? 2 : 4;

    for (size_t i = 0; i < acc.count; ++i)
    {
        if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            const uint16_t* p = reinterpret_cast<const uint16_t*>(base + i * stride);
            out[i] = static_cast<uint32_t>(*p);
        }
        else if (acc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
        {
            const uint32_t* p = reinterpret_cast<const uint32_t*>(base + i * stride);
            out[i] = *p;
        }
    }
}

// -----------------------------------------------------------------------------
// Compute local matrix from TRS components
// -----------------------------------------------------------------------------
static glm::mat4 NodeToMatrix(const tinygltf::Node& node)
{
    if (!node.matrix.empty())
    {
        return glm::make_mat4(node.matrix.data());
    }

    glm::vec3 translation(0.0f);
    if (!node.translation.empty())
        translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);

    glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
    if (!node.rotation.empty())
        rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);

    glm::vec3 scale(1.0f);
    if (!node.scale.empty())
        scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);

    glm::mat4 mat = glm::translate(glm::mat4(1.0f), translation);
    mat = mat * glm::mat4_cast(rotation);
    mat = glm::scale(mat, scale);
    return mat;
}

// -----------------------------------------------------------------------------
// Main loading function
// -----------------------------------------------------------------------------
sModelData* gLoadGLTF(const std::string &_fileName)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, _fileName);
    if (!ret)
    {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, _fileName);
        if (!ret)
        {
            std::cerr << "Failed to load glTF: " << _fileName << "\nError: " << err << std::endl;
            return nullptr;
        }
    }

    if (!warn.empty())
        std::cout << "glTF warning: " << warn << std::endl;

    // Allocate model container
    sModelData* tempModel = new sModelData;

    // -------------------------------------------------------------------------
    // 1. Parse node hierarchy
    // -------------------------------------------------------------------------
    tempModel->node.resize(model.nodes.size());
    for (size_t i = 0; i < model.nodes.size(); ++i)
    {
        const tinygltf::Node& src = model.nodes[i];
        sNode& dst = tempModel->node[i];

        dst.children = src.children;
        dst.localMatrix = NodeToMatrix(src);

        // Store TRS for animation targeting
        if (!src.translation.empty())
            dst.translation = glm::vec3(src.translation[0], src.translation[1], src.translation[2]);
        else
            dst.translation = glm::vec3(0.0f);

        if (!src.rotation.empty())
            dst.rotation = glm::quat(src.rotation[3], src.rotation[0], src.rotation[1], src.rotation[2]);
        else
            dst.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        if (!src.scale.empty())
            dst.scale = glm::vec3(src.scale[0], src.scale[1], src.scale[2]);
        else
            dst.scale = glm::vec3(1.0f);
    }

    // Determine root node (first root of the default scene)
    tempModel->rootNode = -1;
    if (!model.scenes.empty())
    {
        int sceneIdx = model.defaultScene >= 0 ? model.defaultScene : 0;
        const tinygltf::Scene& scene = model.scenes[sceneIdx];
        if (!scene.nodes.empty())
            tempModel->rootNode = scene.nodes[0]; // assuming single root
    }
    if (tempModel->rootNode == -1 && !tempModel->node.empty())
        tempModel->rootNode = 0; // fallback

    // -------------------------------------------------------------------------
    // 2. Parse skin (only first skin is used – typical for glTF)
    // -------------------------------------------------------------------------
    if (!model.skins.empty())
    {
        const tinygltf::Skin& srcSkin = model.skins[0];
        tempModel->skin.joints = srcSkin.joints;

        if (srcSkin.inverseBindMatrices >= 0)
        {
            const tinygltf::Accessor& acc = model.accessors[srcSkin.inverseBindMatrices];
            const tinygltf::BufferView& bv = model.bufferViews[acc.bufferView];
            const tinygltf::Buffer& buf = model.buffers[bv.buffer];
            const float* data = reinterpret_cast<const float*>(buf.data.data() + bv.byteOffset + acc.byteOffset);

            tempModel->skin.inverseBindMatrices.resize(acc.count);
            for (size_t i = 0; i < acc.count; ++i)
            {
                tempModel->skin.inverseBindMatrices[i] = glm::make_mat4(&data[i * 16]);
            }
        }
    }

    // -------------------------------------------------------------------------
    // 3. Parse animations
    // -------------------------------------------------------------------------
    for (const tinygltf::Animation& srcAnim : model.animations)
    {
        sAnimation dstAnim;
        dstAnim.name = srcAnim.name;

        // Process samplers
        for (const tinygltf::AnimationSampler& srcSampler : srcAnim.samplers)
        {
            sAnimationSampler dstSampler;
            dstSampler.input = srcSampler.input;
            dstSampler.output = srcSampler.output;
            dstSampler.interpolation = srcSampler.interpolation;

            // Read time keys
            const tinygltf::Accessor& timeAcc = model.accessors[srcSampler.input];
            ReadFloatAccessor(model, timeAcc, dstSampler.times);

            dstAnim.samplers.push_back(dstSampler);
        }

        // Process channels and fill sampler value vectors based on target path
        for (const tinygltf::AnimationChannel& srcChannel : srcAnim.channels)
        {
            sAnimationChannel dstChannel;
            dstChannel.sampler = srcChannel.sampler;
            dstChannel.targetNode = srcChannel.target_node;
            if (srcChannel.target_path == "translation")
                dstChannel.targetPath = eAnimTargetPath::translation;
            if (srcChannel.target_path == "rotation")
                dstChannel.targetPath = eAnimTargetPath::rotation;
            if (srcChannel.target_path == "scale")
                dstChannel.targetPath = eAnimTargetPath::scale;

            sAnimationSampler& sampler = dstAnim.samplers[dstChannel.sampler];
            const tinygltf::Accessor& valAcc = model.accessors[sampler.output];

            if (dstChannel.targetPath == eAnimTargetPath::translation)
            {
                if (sampler.translations.empty())
                    ReadVec3Accessor(model, valAcc, sampler.translations);
            }
            else if (dstChannel.targetPath == eAnimTargetPath::rotation)
            {
                if (sampler.rotations.empty())
                    ReadQuatAccessor(model, valAcc, sampler.rotations);
            }
            else if (dstChannel.targetPath == eAnimTargetPath::scale)
            {
                if (sampler.scales.empty())
                    ReadVec3Accessor(model, valAcc, sampler.scales);
            }
            // "weights" (morph targets) not implemented here

            dstAnim.channels.push_back(dstChannel);
        }

        // Compute duration (max of all sampler times)
        float maxTime = 0.0f;
        for (const auto& s : dstAnim.samplers)
            if (!s.times.empty() && s.times.back() > maxTime)
                maxTime = s.times.back();
        dstAnim.duration = maxTime;

        tempModel->animation.push_back(dstAnim);
    }

    // -------------------------------------------------------------------------
    // 4. Parse meshes, also load skinning attributes
    // -------------------------------------------------------------------------
    //std::cout << "mesh count: " << model.meshes.size() << std::endl;
    for (const tinygltf::Mesh& mesh : model.meshes)
    {
        for (size_t primIdx = 0; primIdx < mesh.primitives.size(); ++primIdx)
        {
            const tinygltf::Primitive& prim = mesh.primitives[primIdx];
            if (prim.mode != TINYGLTF_MODE_TRIANGLES) continue;

            // --- Positions (mandatory) ---
            auto posIt = prim.attributes.find("POSITION");
            if (posIt == prim.attributes.end()) continue;
            const tinygltf::Accessor& posAcc = model.accessors[posIt->second];
            std::vector<glm::vec3> positions;
            ReadVec3Accessor(model, posAcc, positions);
            if (positions.empty()) continue;

            // --- Normals ---
            std::vector<glm::vec3> normals;
            auto norIt = prim.attributes.find("NORMAL");
            if (norIt != prim.attributes.end())
            {
                const tinygltf::Accessor& norAcc = model.accessors[norIt->second];
                ReadVec3Accessor(model, norAcc, normals);
            }
            if (normals.empty())
                normals.assign(positions.size(), glm::vec3(0.0f, 1.0f, 0.0f));

            // --- Texcoords ---
            std::vector<glm::vec2> texcoords;
            auto texIt = prim.attributes.find("TEXCOORD_0");
            if (texIt != prim.attributes.end())
            {
                const tinygltf::Accessor& texAcc = model.accessors[texIt->second];
                ReadVec2Accessor(model, texAcc, texcoords);
            }
            if (texcoords.empty())
                texcoords.assign(positions.size(), glm::vec2(0.0f));

            // --- Joints (skinning) ---
            std::vector<glm::ivec4> joints;
            auto jointIt = prim.attributes.find("JOINTS_0");
            if (jointIt != prim.attributes.end())
            {
                const tinygltf::Accessor& jointAcc = model.accessors[jointIt->second];
                ReadIVec4Accessor(model, jointAcc, joints);
            }
            if (joints.empty())
                joints.assign(positions.size(), glm::ivec4(0));

            // --- Weights (skinning) ---
            std::vector<glm::vec4> weights;
            auto weightIt = prim.attributes.find("WEIGHTS_0");
            if (weightIt != prim.attributes.end())
            {
                const tinygltf::Accessor& weightAcc = model.accessors[weightIt->second];
                ReadVec4Accessor(model, weightAcc, weights);
            }
            if (weights.empty())
                weights.assign(positions.size(), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

            // --- Indices ---
            std::vector<uint32_t> indices;
            if (prim.indices >= 0)
            {
                const tinygltf::Accessor& idxAcc = model.accessors[prim.indices];
                ReadIndexAccessor(model, idxAcc, indices);
            }
            if (indices.empty())
            {
                indices.resize(positions.size());
                for (size_t i = 0; i < positions.size(); ++i) indices[i] = static_cast<uint32_t>(i);
            }

            // Build mesh data (expand non‑shared vertices)
            sMeshData meshData;
            meshData.name = mesh.name + "_" + std::to_string(primIdx);
            //std::cout << "Mesh: " << meshData.name << std::endl;
            meshData.vertex.resize(indices.size());
            meshData.index.resize(indices.size());

            for (size_t i = 0; i < indices.size(); ++i)
            {
                uint32_t idx = indices[i];
                if (idx >= positions.size()) continue; // should not happen

                meshData.vertex[i].position = positions[idx];
                meshData.vertex[i].normal   = normals[idx];
                meshData.vertex[i].texCoord = texcoords[idx];
                meshData.vertex[i].joints   = joints[idx];
                meshData.vertex[i].weights  = weights[idx];
                meshData.index[i] = static_cast<uint32_t>(i);
            }

            tempModel->mesh.push_back(std::move(meshData));
        }
    }

    return tempModel;
}
