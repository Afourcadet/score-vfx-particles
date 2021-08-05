#include "MeshParticles.hpp"

namespace particles
{
TexturedMeshForParticles::TexturedMeshForParticles()
{
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
}

void TexturedMeshForParticles::setMesh(std::vector<float>& mesh, meshdata& myData)
{
  vertexArray = mesh;
  vertexFloatCount = myData.vertices_length;
  vertexCount = myData.vertices_length / 3;
}

}
