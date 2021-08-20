#include "Executor.hpp"

#include <Gfx/GfxApplicationPlugin.hpp>
#include <Gfx/GfxContext.hpp>

#include <Gfx/Graph/PhongNode.hpp>
#include <Gfx/Graph/TextNode.hpp>
#include <Gfx/TexturePort.hpp>
#include <Gfx/Text/Process.hpp>
#include <Process/Dataflow/Port.hpp>
#include <Process/ExecutionContext.hpp>

#include <score/document/DocumentContext.hpp>

#include <ossia/dataflow/port.hpp>

#include <particles/Node.hpp>
#include <particles/Process.hpp>

namespace particles
{
class mesh_node final : public Gfx::gfx_exec_node
{
public:
  mesh_node(Gfx::GfxExecutionAction& ctx)
      : gfx_exec_node{ctx}
  {
    root_outputs().push_back(new ossia::texture_outlet);

    auto n = std::make_unique<particles::Node>();

    id = exec_context->ui->register_node(std::move(n));
  }

  ~mesh_node() { exec_context->ui->unregister_node(id); }

  std::string label() const noexcept override { return "particles"; }
};

ProcessExecutorComponent::ProcessExecutorComponent(
    particles::Model& element,
    const Execution::Context& ctx,
    QObject* parent)
    : ProcessComponent_T{element, ctx, "particlesExecutorComponent", parent}
{
  try
  {
    auto n = std::make_shared<mesh_node>(
        ctx.doc.plugin<Gfx::DocumentPlugin>().exec);
    for (std::size_t i = 0; i < 4; i++)
    {
      auto ctrl = qobject_cast<Process::ControlInlet*>(element.inlets()[i]);
      auto& p = n->add_control();
      p->value = ctrl->value();
      p->changed = true;

      QObject::connect(
                  ctrl,
                  &Process::ControlInlet::valueChanged,
                  this,
                  Gfx::con_unvalidated{ctx, i, 0, n});
    }
    this->node = n;
    m_ossia_process = std::make_shared<ossia::node_process>(n);
  }
  catch (...)
  {
  }
}
}
