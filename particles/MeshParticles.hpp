#pragma once

#include <Gfx/Graph/CommonUBOs.hpp>
#include <Gfx/Graph/NodeRenderer.hpp>
#include <Gfx/Graph/RenderList.hpp>
#include <score/tools/Debug.hpp>
#include "Loadmesh.h"
#ifndef LOADMESH_H
#define LOADMESH_H
#include "tiny_obj_loader.h"
#endif
namespace particles
{
/** Here we define a mesh fairly manually and in a fairly suboptimal way
 * (based on this: https://pastebin.com/DXKEmvap)
 */
struct TexturedMeshForParticles
{
    TexturedMeshForParticles();

    void setMesh(std::vector<float>& mesh, meshdata& myData);

    ossia::small_vector<QRhiVertexInputBinding, 2> vertexInputBindings;
    ossia::small_vector<QRhiVertexInputAttribute, 2> vertexAttributeBindings;

    gsl::span<const float> vertexArray;
    gsl::span<const unsigned int> indexArray;
    int vertexCount{};
    int indexCount{};

    std::size_t vertexFloatCount{};
};
}
