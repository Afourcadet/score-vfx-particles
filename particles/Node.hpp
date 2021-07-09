#pragma once
#include <Gfx/Graph/Node.hpp>
#include <Gfx/Graph/RenderList.hpp>
#include <Gfx/Graph/CommonUBOs.hpp>

#include "Loadmesh.h"

#ifndef LOADMESH_H
#define LOADMESH_H
#include "tiny_obj_loader.h"
#endif

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
