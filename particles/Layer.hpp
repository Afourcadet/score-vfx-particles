#pragma once
#include <Control/DefaultEffectItem.hpp>
#include <Effect/EffectFactory.hpp>
#include <Process/GenericProcessFactory.hpp>

#include <particles/Process.hpp>

namespace particles
{
using LayerFactory = Process::GenericDefaultLayerFactory<particles::Model>;
}
