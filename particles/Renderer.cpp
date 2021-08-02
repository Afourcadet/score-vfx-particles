#include "Renderer.hpp"

#include <score/tools/Debug.hpp>
#include <Gfx/Graph/NodeRenderer.hpp>
// This header is used because some function names change between Qt 5 and Qt 6
namespace particles
{
Renderer::~Renderer() { }

// This function is only useful to reimplement if the node has an
// input port (e.g. if it's an effect / filter / ...)
score::gfx::TextureRenderTarget Renderer::renderTargetForInput(const score::gfx::Port& p)
{
    return {};
}

// The pipeline is the object which contains all the state
// needed by the graphics card when issuing draw calls
score::gfx::Pipeline Renderer::buildPipeline(
        const score::gfx::RenderList& renderer,
        const score::gfx::Mesh& mesh,
        const QShader& vertexS,
        const QShader& fragmentS,
        const score::gfx::TextureRenderTarget& rt,
        QRhiShaderResourceBindings* srb)
{
    auto& rhi = *renderer.state.rhi;
    auto ps = rhi.newGraphicsPipeline();
    ps->setName("Node::ps");
    SCORE_ASSERT(ps);

    // Set various graphics options
    QRhiGraphicsPipeline::TargetBlend premulAlphaBlend;
    premulAlphaBlend.enable = true;
    ps->setTargetBlends({premulAlphaBlend});

    ps->setSampleCount(1);

    ps->setDepthTest(true);
    ps->setDepthWrite(true);
    ps->setDepthOp(QRhiGraphicsPipeline::Less);

    // Matches the vertex data
    ps->setTopology(QRhiGraphicsPipeline::Triangles);
    ps->setCullMode(QRhiGraphicsPipeline::CullMode::Back);
    ps->setFrontFace(QRhiGraphicsPipeline::FrontFace::CCW);

    // Set the shaders used
    ps->setShaderStages(
                {{QRhiShaderStage::Vertex, vertexS},
                 {QRhiShaderStage::Fragment, fragmentS}});

    // Set the mesh specification
    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings(
                mesh.vertexInputBindings.begin(), mesh.vertexInputBindings.end());

    inputLayout.setAttributes(
                mesh.vertexAttributeBindings.begin(),
                mesh.vertexAttributeBindings.end());
    ps->setVertexInputLayout(inputLayout);

    // Set the shader resources (input UBOs, samplers & textures...)
    ps->setShaderResourceBindings(srb);

    // Where we are rendering
    SCORE_ASSERT(rt.renderPass);
    ps->setRenderPassDescriptor(rt.renderPass);

    SCORE_ASSERT(ps->create());
    return {ps, srb};
}
void Renderer::init(score::gfx::RenderList& renderer)
{
    std::cout << "started init renderer\n";
    const auto& mesh = TexturedMeshForParticles::instance();

    particleOffsets = renderer.state.rhi->newBuffer(
                QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer | QRhiBuffer::StorageBuffer, maxparticles * 4 * sizeof(float));
    SCORE_ASSERT(particleOffsets->create());
    particleSpeeds = renderer.state.rhi->newBuffer(
                QRhiBuffer::Immutable, QRhiBuffer::StorageBuffer, maxparticles * 4 * sizeof(float));
    SCORE_ASSERT(particleSpeeds->create());
    particleControls = renderer.state.rhi->newBuffer(
                QRhiBuffer::Immutable, QRhiBuffer::UniformBuffer, sizeof(Controls));
    SCORE_ASSERT(particleControls->create());

    // Load the mesh data into the GPU
    {
        auto [mbuffer, ibuffer] = renderer.initMeshBuffer(mesh);
                m_meshBuffer = mbuffer;
                m_idxBuffer = ibuffer;
                m_particleOffsets = particleOffsets;
                m_particleSpeeds = particleSpeeds;
                m_particleControls = particleControls;
    }

                // Initialize the Process UBO (provides timing information, etc.)
        {
            processUBOInit(renderer);
        }

        // Initialize our camera
        {
            m_material.size = sizeof(score::gfx::ModelCameraUBO);
            m_material.buffer = renderer.state.rhi->newBuffer(
                        QRhiBuffer::Dynamic,
                        QRhiBuffer::UniformBuffer,
                        sizeof(score::gfx::ModelCameraUBO));
            SCORE_ASSERT(m_material.buffer->create());
        }

        auto& n = static_cast<const Node&>(this->node);
        auto& rhi = *renderer.state.rhi;

        // Create GPU textures for the image
        const QSize sz = n.m_image.size();
        m_texture = rhi.newTexture(
                    QRhiTexture::BGRA8,
                    QSize{sz.width(), sz.height()},
                    1,
                    QRhiTexture::Flag{});

        m_texture->setName("Node::tex");
        m_texture->create();

        // Create the sampler in which we are going to put the texture
        {
            auto sampler = rhi.newSampler(
                        QRhiSampler::Linear,
                        QRhiSampler::Linear,
                        QRhiSampler::None,
                        QRhiSampler::Repeat,
                        QRhiSampler::Repeat);

            sampler->setName("Node::sampler");
            sampler->create();
            m_samplers.push_back({sampler, m_texture});
        }
        SCORE_ASSERT(n.m_vertexS.isValid());
        SCORE_ASSERT(n.m_fragmentS.isValid());

        // Create the rendering pipelines for each output of this node.
        for (score::gfx::Edge* edge : this->node.output[0]->edges)
        {
            auto rt = renderer.renderTargetForOutput(*edge);
            if (rt.renderTarget)
            {
                auto bindings = createDefaultBindings(
                            renderer, rt, m_processUBO, m_material.buffer, m_samplers);
                auto pipeline = buildPipeline(
                            renderer, mesh, n.m_vertexS, n.m_fragmentS, rt, bindings);
                m_p.emplace_back(edge, pipeline);
            }
        }

        {
            QString comp = QString(R"_(#version 450
    layout (local_size_x = 256) in;
    struct Pos
    {
        vec4 pos;
    };
    struct Speed
    {
        vec4 spd;
    };
    struct Ctrl
    {
        float speedMod;
    };
    layout(std140, binding = 0) buffer PBuf
    {
        Pos d[];
    } pbuf;
    layout(std140, binding = 1) buffer SBuf
    {
        Speed d[];
    } sbuf;
    layout(std140, binding = 2) uniform Controls
    {
        Ctrl c;
    } controls;
    void main()
    {
        vec4 cs = vec4(0, -0.001, 0, 0);
        uint index = gl_GlobalInvocationID.x;
        if (index < %1) {
            vec4 p = pbuf.d[index].pos;
            vec4 s = sbuf.d[index].spd;
            s += cs;
            vec4 ns = controls.c.speedMod*s;
            p += ns;
            pbuf.d[index].pos = p;
            sbuf.d[index].spd = s;
        }
    }
    )_").arg(maxparticles);
            QShader computeShader = score::gfx::makeCompute(comp);
            compute = rhi.newComputePipeline();

            auto csrb = rhi.newShaderResourceBindings();
            {
                QRhiShaderResourceBinding bindings[3] = {
                    QRhiShaderResourceBinding::bufferLoadStore(0, QRhiShaderResourceBinding::ComputeStage, particleOffsets),
                    QRhiShaderResourceBinding::bufferLoadStore(1, QRhiShaderResourceBinding::ComputeStage, particleSpeeds),
                    QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::ComputeStage, particleControls)
                };

                csrb->setBindings(bindings, bindings + 3);
                SCORE_ASSERT(csrb->build());
            }
            compute->setShaderResourceBindings(csrb);
            compute->setShaderStage(QRhiShaderStage(QRhiShaderStage::Compute, computeShader));
            SCORE_ASSERT(compute->build());
        }
    }

    int m_rotationCount = 0;
    void Renderer::update(score::gfx::RenderList& renderer, QRhiResourceUpdateBatch& res)
    {
        auto& n = static_cast<const Node&>(this->node);
        {
            // Set up a basic camera
            auto& ubo = (score::gfx::ModelCameraUBO&)n.ubo;

            // We use the Qt class QMatrix4x4 as an utility
            // as it provides everything needed for projection transformations

            // Our object rotates in a very crude way
            QMatrix4x4 model;
            model.scale(0.005);
            //model.rotate(m_rotationCount++, QVector3D(1, 1, 1));

            // The camera and viewports are fixed
            QMatrix4x4 view;
            view.lookAt(QVector3D{0, 0, 1}, QVector3D{0, 0, 0}, QVector3D{0, 1, 0});

            QMatrix4x4 projection;
            projection.perspective(90, 16. / 9., 0.001, 1000.);

            QMatrix4x4 mv = view * model;
            QMatrix4x4 mvp = projection * mv;
            QMatrix3x3 norm = model.normalMatrix();

            score::gfx::copyMatrix(mvp, ubo.mvp);
            score::gfx::copyMatrix(mv, ubo.mv);
            score::gfx::copyMatrix(model, ubo.model);
            score::gfx::copyMatrix(view, ubo.view);
            score::gfx::copyMatrix(projection, ubo.projection);
            score::gfx::copyMatrix(norm, ubo.modelNormal);

            // Send the camera UBO to the graphics card
            res.updateDynamicBuffer(m_material.buffer, 0, m_material.size, &ubo);
        }

        // Update the process UBO (indicates timing)
        res.updateDynamicBuffer(
                    m_processUBO, 0, sizeof(score::gfx::ProcessUBO), &this->node.standardUBO);

        if(!particlesUploaded) {
            for(int i = 0; i < maxparticles * 4; i++) {
                data[i] = 50 * double(rand()) / RAND_MAX - 1;
                speed[i] = (30 * double(rand()) / RAND_MAX - 1) * 0.01;
            }
            res.uploadStaticBuffer(particleSpeeds, 0, maxparticles * 4 * sizeof(float), speed.get());
            res.uploadStaticBuffer(particleOffsets, 0, maxparticles * 4 * sizeof(float), data.get());
            particlesUploaded = true;
        }

        res.uploadStaticBuffer(particleControls, 0, sizeof(float), &n.particlesSpeedMod);
        res.uploadStaticBuffer(particleControls, sizeof(float), sizeof(int), &n.particlesNumber);

        instances = n.particlesNumber;

        // If images haven't been uploaded yet, upload them.
        if (!m_uploaded)
        {
            res.uploadTexture(m_texture, n.m_image);
            m_uploaded = true;
        }
    }

    void Renderer::runInitialPasses(
                score::gfx::RenderList& renderer,
                QRhiCommandBuffer& cb,
                QRhiResourceUpdateBatch*& res,
                score::gfx::Edge& edge)
    {
        if(compute)
        {
            cb.beginComputePass(nullptr);
            cb.setComputePipeline(compute);
            cb.setShaderResources(compute->shaderResourceBindings());
            cb.dispatch(std::max(1, maxparticles / 256), 1, 1);
            cb.endComputePass();
        }
    }

    // Everything is set up, we can render our mesh
    void Renderer::runRenderPass(
                score::gfx::RenderList& renderer,
                QRhiCommandBuffer& cb,
                score::gfx::Edge& edge)
    {
        const auto& mesh = TexturedMeshForParticles::instance();
        auto it = ossia::find_if(m_p, [ptr=&edge] (const auto& p){ return p.first == ptr; });
        SCORE_ASSERT(it != m_p.end());
        {
            const auto sz = renderer.state.size;
            cb.setGraphicsPipeline(it->second.pipeline);
            cb.setShaderResources(it->second.srb);
            cb.setViewport(QRhiViewport(0, 0, sz.width(), sz.height()));

            assert(this->m_meshBuffer);
            assert(this->m_meshBuffer->usage().testFlag(QRhiBuffer::VertexBuffer));

            const QRhiCommandBuffer::VertexInput bindings[]
                    = {
                {this->m_meshBuffer, 0},
                {this->m_meshBuffer, mesh.vertexCount*sizeof(float)},
                {this->m_particleOffsets, 0},
                {this->m_particleSpeeds, 0},
                {this->m_particleControls, 0}
            };

            cb.setVertexInput(0, 5, bindings, this->m_idxBuffer, 0, QRhiCommandBuffer::IndexFormat::IndexUInt32);

            mesh.setupBindings(*this->m_meshBuffer, this->m_idxBuffer, cb);

            if(this->m_idxBuffer)
                cb.drawIndexed(mesh.indexCount, instances, 0, mesh.indexCount * 3 * sizeof(float));
            else
                cb.draw(mesh.vertexCount, instances, 0, mesh.indexCount * 3 * sizeof(float));
        }
    }

    // Free resources allocated in this class
    void Renderer::release(score::gfx::RenderList& r)
    {
        m_texture->deleteLater();
        m_texture = nullptr;
        m_particleOffsets->releaseAndDestroyLater();
        m_particleOffsets = nullptr;
        m_particleSpeeds->releaseAndDestroyLater();
        m_particleSpeeds = nullptr;
        m_particleControls->releaseAndDestroyLater();
        m_particleControls = nullptr;

        // This will free all the other resources - material & process UBO, etc
        defaultRelease(r);
    }

    QRhiTexture* m_texture{};
    bool m_uploaded = false;
};
