#include "score_addon_particles.hpp"

#include <score/plugins/FactorySetup.hpp>

#include <particles/Executor.hpp>
#include <particles/Layer.hpp>
#include <particles/Process.hpp>
#include <score_plugin_engine.hpp>
#include <score_plugin_gfx.hpp>

score_addon_particles::score_addon_particles() { }

score_addon_particles::~score_addon_particles() { }

std::vector<std::unique_ptr<score::InterfaceBase>>
score_addon_particles::factories(
    const score::ApplicationContext& ctx,
    const score::InterfaceKey& key) const
{
  return instantiate_factories<
      score::ApplicationContext,
      FW<Process::ProcessModelFactory, particles::ProcessFactory>,
      FW<Execution::ProcessComponentFactory,
         particles::ProcessExecutorComponentFactory>>(ctx, key);
}

auto score_addon_particles::required() const -> std::vector<score::PluginKey>
{
  return {score_plugin_gfx::static_key()};
}

#include <score/plugins/PluginInstances.hpp>
SCORE_EXPORT_PLUGIN(score_addon_particles)
