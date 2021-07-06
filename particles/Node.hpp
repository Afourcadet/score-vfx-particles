#pragma once
#include <Gfx/Graph/Node.hpp>
#include <Gfx/Graph/RenderList.hpp>
#include <Gfx/Graph/CommonUBOs.hpp>

namespace particles
{
class Renderer;
class Node : public score::gfx::NodeModel
{
public:
  Node();
  virtual ~Node();

  score::gfx::NodeRenderer*
  createRenderer(score::gfx::RenderList& r) const noexcept override;

private:
  score::gfx::ModelCameraUBO ubo;

  friend Renderer;
  QImage m_image;
};
}
