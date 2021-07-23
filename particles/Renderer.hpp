#pragma once
#include "Node.hpp"
#include "MeshParticles.hpp"

namespace particles
{
#include <Gfx/Qt5CompatPush> // clang-format: keep
const int instances = 100;

class Renderer : public score::gfx::GenericNodeRenderer
{
public:
    using GenericNodeRenderer::GenericNodeRenderer;
    QRhiBuffer* m_particleOffsets{};
    QRhiBuffer* m_particleSpeeds{};

private:
    ~Renderer();

    // This function is only useful to reimplement if the node has an
    // input port (e.g. if it's an effect / filter / ...)
    score::gfx::TextureRenderTarget renderTargetForInput(const score::gfx::Port& p) override;

    // The pipeline is the object which contains all the state
    // needed by the graphics card when issuing draw calls
    score::gfx::Pipeline buildPipeline(
            const score::gfx::RenderList& renderer,
            const score::gfx::Mesh& mesh,
            const QShader& vertexS,
            const QShader& fragmentS,
            const score::gfx::TextureRenderTarget& rt,
            QRhiShaderResourceBindings* srb);
    QRhiBuffer* particleOffsets{};
    QRhiBuffer* particleSpeeds{};
    bool particlesUploaded{};
    QRhiComputePipeline* compute{};
    QRhiTexture* m_texture{};
    bool m_uploaded = false;
    float data[instances * 4];
    float speed[instances * 4];
    void init(score::gfx::RenderList& renderer) override;
    void update(score::gfx::RenderList& renderer, QRhiResourceUpdateBatch& res) override;
    void runInitialPasses(
                score::gfx::RenderList& renderer,
                QRhiCommandBuffer& cb,
                QRhiResourceUpdateBatch*& res,
                score::gfx::Edge& edge) override;
    QRhiResourceUpdateBatch* runRenderPass(
                score::gfx::RenderList& renderer,
                QRhiCommandBuffer& cb,
                score::gfx::Edge& edge) override;
    void release(score::gfx::RenderList& r) override;
};
}