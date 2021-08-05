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
struct TexturedMeshForParticles final : score::gfx::Mesh
{
    // Generate our mesh data
    meshdata myData;
    std::vector<float> mesh;

    TexturedMeshForParticles(std::string meshName);
    // Utility singleton
    static TexturedMeshForParticles& instance(std::string meshName) noexcept;

    // Ignore this function
    const char* defaultVertexShader() const noexcept override;

    // This function is called when running the draw calls,
    // it tells the pipeline which buffers are going to be bound
    // to each attribute defined above
    void setupBindings(
            QRhiBuffer& vtxData,
            QRhiBuffer* idxData,
            QRhiCommandBuffer& cb) const noexcept override;
};
}
