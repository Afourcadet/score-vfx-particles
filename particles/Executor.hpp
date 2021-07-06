#pragma once
#include <Process/Execution/ProcessComponent.hpp>

#include <ossia/dataflow/node_process.hpp>

namespace particles
{
class Model;
class ProcessExecutorComponent final
    : public Execution::ProcessComponent_T<particles::Model, ossia::node_process>
{
  COMPONENT_METADATA("ce4f8306-e912-4e32-8a0a-7a495e64aa80")
public:
  ProcessExecutorComponent(
      Model& element,
      const Execution::Context& ctx,
      QObject* parent);
};

using ProcessExecutorComponentFactory
    = Execution::ProcessComponentFactory_T<ProcessExecutorComponent>;
}
