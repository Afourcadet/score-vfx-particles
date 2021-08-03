#include "MeshParticles.hpp"

namespace particles
{
TexturedMeshForParticles::TexturedMeshForParticles(std::string meshName)
{
    myData = getmesh(meshName);
    mesh = myData.values;

    vertexInputBindings.push_back({3 * sizeof(float), QRhiVertexInputBinding::PerVertex});
    vertexAttributeBindings.push_back(
                {0, 0, QRhiVertexInputAttribute::Float3, 0});
    // Texcoord
    // Each `in vec2 texcoord;` in the fragment shader will be an [ u v ] element
    vertexInputBindings.push_back({2 * sizeof(float), QRhiVertexInputBinding::PerVertex});
    vertexAttributeBindings.push_back(
                {1, 1, QRhiVertexInputAttribute::Float2, 0});
    // These variables are used by score to upload the texture
    // and send the draw call automatically

    vertexInputBindings.push_back({4 * sizeof(float), QRhiVertexInputBinding::PerInstance});
    vertexAttributeBindings.push_back(
                {2, 2, QRhiVertexInputAttribute::Float4, 0});

    vertexArray = mesh;
    vertexCount = myData.vertices_length / 3;
}

const TexturedMeshForParticles& TexturedMeshForParticles::instance(std::string meshName) noexcept
{
    static const TexturedMeshForParticles newmesh(meshName);
    return newmesh;
}

const char* TexturedMeshForParticles::defaultVertexShader() const noexcept { return ""; }

void TexturedMeshForParticles::setupBindings(
        QRhiBuffer& vtxData,
        QRhiBuffer* idxData,
        QRhiCommandBuffer& cb) const noexcept
{
    const QRhiCommandBuffer::VertexInput bindings[] = {
        {&vtxData, 0},                      // vertex starts at offset zero
        {&vtxData, myData.vertices_length * sizeof(float)} // texcoord starts after all the vertices
    };

    cb.setVertexInput(0, 2, bindings);
}
}
