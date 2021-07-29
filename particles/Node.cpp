#include "Node.hpp"
#include "Renderer.hpp"

#include <QPainter>
#include <ossia/detail/math.hpp>
#include <ossia/network/value/value.hpp>
#include <ossia/network/value/value_conversion.hpp>
#include <ossia/gfx/port_index.hpp>
namespace particles
{
// Here we define basic shaders to display a textured cube with a camera
static const constexpr auto vertex_shader = R"_(#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec4 offset;

layout(location = 1) out vec2 v_texcoord;

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

layout(binding = 3) uniform sampler2D y_tex;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
  v_texcoord = vec2(texcoord.x, texcoordAdjust.y + texcoordAdjust.x * texcoord.y);
  gl_Position = clipSpaceCorrMatrix * matrixModelViewProjection * vec4(position + offset.xyz, 1.0);
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
    particlesSpeedMod = 0.f;
  
    std::cout << "new_debug";

    // Load ubo address in m_materialData
    m_materialData.reset((char*)&ubo);

    // Generate the shaders
    std::tie(m_vertexS, m_fragmentS)
            = score::gfx::makeShaders(vertex_shader, fragment_shader);
    SCORE_ASSERT(m_vertexS.isValid());
    SCORE_ASSERT(m_fragmentS.isValid());
    // Create an input port
    input.push_back(
                new score::gfx::Port{this, {}, score::gfx::Types::Float, {}});
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

void Node::process(const score::gfx::Message& msg)
{
  ProcessNode::process(msg.token);

  int32_t p = 0;
  for (const score::gfx::gfx_input& m: msg.input)
  {
    if(auto val = std::get_if<ossia::value>(&m))
    {
    switch(p)
    {
      case 0:
      {
        // Speed of particles
        {
          particlesSpeedMod = 0.01*ossia::convert<float>(*val);
          mustRerender = true;
        }
        break;
      }
    }
    }
    p++;
  }
}

#include <Gfx/Qt5CompatPop> // clang-format: keep

    score::gfx::NodeRenderer*
    Node::createRenderer(score::gfx::RenderList& r) const noexcept
    {
        return new Renderer{*this};
    }
}
