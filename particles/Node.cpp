#include "Node.hpp"

#include <Gfx/Graph/NodeRenderer.hpp>

#include <score/tools/Debug.hpp>

struct data getmesh()
{
    std::string inputfile = "ponte.obj";
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
      if (!reader.Error().empty()) {
          std::cerr << "TinyObjReader: " << reader.Error();
      }
      exit(1);
    }

    if (!reader.Warning().empty()) {
      std::cout << "TinyObjReader: " << reader.Warning();
    }

    struct data d = attrib_to_data(reader, inputfile, reader_config);

    //std::cout << "data vertices length : " << d.vertices_length << "\n" ;

    return d;
}

namespace particles
{
/** Here we define a mesh fairly manually and in a fairly suboptimal way
 * (based on this: https://pastebin.com/DXKEmvap)
 */
const int instances = 1000;
struct TexturedMesh final : score::gfx::Mesh
{
    // Generate our mesh data
    struct data myData = getmesh();
    const std::vector<float> mesh = myData.values;

    explicit TexturedMesh()
    {
      // Our mesh's attribute data is not interleaved, thus
      // we have multiple bindings stored in the same buffer:
      // [ {x y z} {x y z} {x y z} ... ] [ {u v} {u v} {u v} ... ]

      // Vertex
      // Each `in vec3 position;` in the vertex shader will be an [ x y z ] element
      // The stride is the 3 * sizeof(float) - it means that:
      // * vertex 0's data in this attribute will start at 0 (in bytes)
      // * vertex 1's data in this attribute will start at 3 * sizeof(float)
      // * vertex 2's data in this attribute will start at 6 * sizeof(float)
      // (basically that every vertex position is one after each other).
      vertexInputBindings.push_back({3 * sizeof(float)});
      vertexAttributeBindings.push_back(
          {0, 0, QRhiVertexInputAttribute::Float3, 0});

      // Texcoord
      // Each `in vec2 texcoord;` in the fragment shader will be an [ u v ] element
      vertexInputBindings.push_back({2 * sizeof(float)});
      vertexAttributeBindings.push_back(
          {1, 1, QRhiVertexInputAttribute::Float2, 0});
      // These variables are used by score to upload the texture
      // and send the draw call automatically

      /*vertexInputBindings.push_back({3 * sizeof(float)});
      vertexAttributeBindings.push_back(
          {2, 2, QRhiVertexInputAttribute::Float3, 0});*/

      vertexArray = mesh;
      vertexCount = myData.vertices_length / 3;

      std::cout << "data vertices length : " << vertexCount << "\n" ;

      // Note: if we had an interleaved mesh, where the data is stored like
      // [ { x y z u v } { x y z u v } { x y z u v } ] ...
      // Then we'd have a single binding of stride 5 * sizeof(float)
      // (3 floats for the position and 2 floats for the texture coordinates):

      // vertexInputBindings.push_back({5 * sizeof(float)});

      // And two attributes mapped to that binding
      // The first attribute (position) starts at byte 0 in each element:
      //
      //     vertexAttributeBindings.push_back(
      //         {0, 0, QRhiVertexInputAttribute::Float3, 0});
      //
      // The second attribute (texcoord) starts at byte 3 * sizeof(float)
      // (just after the position):
      //
      //     vertexAttributeBindings.push_back(
      //         {0, 1, QRhiVertexInputAttribute::Float2, 3 * sizeof(float)});
    }

  // Utility singleton
  static const TexturedMesh& instance() noexcept
  {
    static const TexturedMesh newmesh;
    return newmesh;
  }

  // Ignore this function
  const char* defaultVertexShader() const noexcept override { return ""; }

  // This function is called when running the draw calls,
  // it tells the pipeline which buffers are going to be bound
  // to each attribute defined above
  void setupBindings(
      QRhiBuffer& vtxData,
      QRhiBuffer* idxData,
      QRhiCommandBuffer& cb) const noexcept override
  {
    const QRhiCommandBuffer::VertexInput bindings[] = {
        {&vtxData, 0},                      // vertex starts at offset zero
        {&vtxData, myData.vertices_length * sizeof(float)}, // texcoord starts after all the vertices
        {&vtxData, (myData.vertices_length * sizeof(float) * 5)/3}
    };

    cb.setVertexInput(0, 3, bindings);
  }
};

// Here we define basic shaders to display a textured cube with a camera
static const constexpr auto vertex_shader = R"_(#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 offset;

layout(location = 1) out vec2 v_texcoord;
layout(binding = 3) uniform sampler2D y_tex;

layout(std140, binding = 0) uniform renderer_t {
  mat4 clipSpaceCorrMatrix;
  vec2 texcoordAdjust;
  vec2 renderSize;
};

layout(std140, binding = 2) uniform model_t {
  mat4 matrixModelViewProjection;
  mat4 matrixModelView;
  mat4 matrixModel;
  mat4 matrixView;
  mat4 matrixProjection;
  mat3 matrixNormal;
};

out gl_PerVertex { vec4 gl_Position; };

void main()
{
  v_texcoord = vec2(texcoord.x, texcoordAdjust.y + texcoordAdjust.x * texcoord.y);
  gl_Position = clipSpaceCorrMatrix * matrixModelViewProjection * vec4(position + offset, 1.0);
}
)_";

static const constexpr auto fragment_shader = R"_(#version 450
layout(std140, binding = 0) uniform renderer_t {
  mat4 clipSpaceCorrMatrix;
  vec2 texcoordAdjust;
  vec2 renderSize;
};

layout(binding=3) uniform sampler2D y_tex;

layout(location = 1) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;

void main ()
{
  fragColor = texture(y_tex, v_texcoord.xy);
  if(fragColor.a == 0.)
    fragColor = vec4(1.,1.,1., 1.);
}
)_";

Node::Node()
{
  // This texture is provided by score
  m_image = QImage(":/ossia-score.png");

  std::cout << "new_debug";

  // Load ubo address in m_materialData
  m_materialData.reset((char*)&ubo);

  // Generate the shaders
  std::tie(m_vertexS, m_fragmentS)
      = score::gfx::makeShaders(vertex_shader, fragment_shader);
  SCORE_ASSERT(m_vertexS.isValid());
  SCORE_ASSERT(m_fragmentS.isValid());

  // Create an output port to indicate that this node
  // draws something
  output.push_back(
      new score::gfx::Port{this, {}, score::gfx::Types::Image, {}});
}


Node::~Node()
{
  // We do not want to free m_materialData as it is
  // not allocated dynamically
  m_materialData.release();
}

// This header is used because some function names change between Qt 5 and Qt 6
#include <Gfx/Qt5CompatPush> // clang-format: keep
class Renderer : public score::gfx::GenericNodeRenderer
{
public:
  using GenericNodeRenderer::GenericNodeRenderer;

private:
  ~Renderer() { }

  // This function is only useful to reimplement if the node has an
  // input port (e.g. if it's an effect / filter / ...)
  score::gfx::TextureRenderTarget
  renderTargetForInput(const score::gfx::Port& p) override
  {
    return {};
  }

  // The pipeline is the object which contains all the state
  // needed by the graphics card when issuing draw calls
  score::gfx::Pipeline buildPipeline(
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

  void init(score::gfx::RenderList& renderer) override
  {
    std::cout << "started init renderer\n";
    const auto& mesh = TexturedMesh::instance();

    QRhiBuffer* particleOffsets = renderer.state.rhi->newBuffer(
          QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer | QRhiBuffer::StorageBuffer, instances * 3 * sizeof(float));
    SCORE_ASSERT(particleOffsets->build());
    /*particleSpeeds = rhi.newBuffer(
          QRhiBuffer::Immutable, QRhiBuffer::StorageBuffer, instances * 2 * sizeof(float));
    SCORE_ASSERT(particleSpeeds->build());*/

    // Load the mesh data into the GPU
    {
      auto [mbuffer, ibuffer] = renderer.initMeshBuffer(mesh);
      m_meshBuffer = mbuffer;
      m_idxBuffer = ibuffer;
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
  }

  int m_rotationCount = 0;
  void update(score::gfx::RenderList& renderer, QRhiResourceUpdateBatch& res)
      override
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
      model.rotate(m_rotationCount++, QVector3D(1, 1, 1));

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

    // If images haven't been uploaded yet, upload them.
    if (!m_uploaded)
    {
      res.uploadTexture(m_texture, n.m_image);
      m_uploaded = true;
    }
  }

  // Everything is set up, we can render our mesh
  QRhiResourceUpdateBatch* runRenderPass(
      score::gfx::RenderList& renderer,
      QRhiCommandBuffer& cb,
      score::gfx::Edge& edge) override
  {
      // Update a first time everything

      // PASSINDEX must be set to the last index
      // FIXME
      /*n.standardUBO.passIndex = m_passes.size() - 1;

      update(renderer, res);

      auto updateBatch = &res;

      // Draw the passes
      const auto& pass = m_passes[0];*/
      {
        edge = *m_p.back().first;
        SCORE_ASSERT(renderer.renderTargetForOutput(edge).renderTarget);
        SCORE_ASSERT(m_p.back().second.pipeline);
        SCORE_ASSERT(m_p.back().second.srb);
        // TODO : combine all the uniforms..

        //auto rt = pass.renderTarget.renderTarget;
        auto rt = renderer.renderTargetForOutput(edge).renderTarget;
        //auto pipeline = pass.p.pipeline;
        auto pipeline = m_p.back().second.pipeline;
        auto srb = m_p.back().second.srb;
        auto texture = m_texture;

        const auto& mesh = TexturedMesh::instance();

        cb.beginPass(rt, Qt::black, {1.0f, 0});
        {
          cb.setGraphicsPipeline(pipeline);
          cb.setShaderResources(srb);

          if(texture)
          {
            cb.setViewport(QRhiViewport(0, 0, texture->pixelSize().width(), texture->pixelSize().height()));
          }
          else
          {
            const auto sz = renderer.state.size;
            cb.setViewport(QRhiViewport(0, 0, sz.width(), sz.height()));
          }

          assert(this->m_meshBuffer);
          assert(this->m_meshBuffer->usage().testFlag(QRhiBuffer::VertexBuffer));

          const QRhiCommandBuffer::VertexInput bindings[]
              = {
            {this->m_meshBuffer, 0},
            {this->m_particleOffset, 0}
          };

          cb.setVertexInput(0, 1, bindings, this->m_idxBuffer, 0, QRhiCommandBuffer::IndexFormat::IndexUInt32);

          cb.drawIndexed(mesh.indexCount, instances);
        }
        cb.endPass();
      }
    return nullptr;
    /*const auto& mesh = TexturedMesh::instance();
    defaultRenderPass(renderer, mesh, cb, edge);
    return nullptr;*/
  }

  // Free resources allocated in this class
  void release(score::gfx::RenderList& r) override
  {
    m_texture->deleteLater();
    m_texture = nullptr;

    // This will free all the other resources - material & process UBO, etc
    defaultRelease(r);
  }

  QRhiTexture* m_texture{};
  bool m_uploaded = false;
};
#include <Gfx/Qt5CompatPop> // clang-format: keep

score::gfx::NodeRenderer*
Node::createRenderer(score::gfx::RenderList& r) const noexcept
{
  return new Renderer{*this};
}
}
