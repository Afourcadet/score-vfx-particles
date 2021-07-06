#pragma once
#include <Process/ProcessMetadata.hpp>

namespace particles
{
class Model;
}

PROCESS_METADATA(
    ,
    particles::Model,
    "56311e09-afd6-4f66-bc18-1ee30c402d3f",
    "particles",                           // Internal name
    "particles",                           // Pretty name
    Process::ProcessCategory::Visual,  // Category
    "GFX",                             // Category
    "My VFX",                          // Description
    "ossia team",                      // Author
    (QStringList{"shader", "gfx"}),    // Tags
    {},                                // Inputs
    {},                                // Outputs
    Process::ProcessFlags::SupportsAll // Flags
)
