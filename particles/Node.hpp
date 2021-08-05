#pragma once
#include <Gfx/Graph/Node.hpp>
#include "MeshParticles.hpp"

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
  void process(const score::gfx::Message& msg) override;
  float particlesSpeedMod = 1.f;
  int particlesNumber = 0;
  std::string meshName;
  std::atomic_bool mustRerender{true};
private:
  score::gfx::ModelCameraUBO ubo;

  friend Renderer;
  QImage m_image;
};
}
