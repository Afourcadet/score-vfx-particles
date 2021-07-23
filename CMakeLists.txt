cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

if(NOT TARGET score_lib_base)
  include("${SCORE_SOURCE_DIR}/cmake/ScoreExternalAddon.cmake")
endif()

if(NOT TARGET score_plugin_gfx)
  message("Requires gfx plug-in")
  return()
endif()

project(score_addon_particles LANGUAGES CXX)

# General initialization
score_common_setup()

# Creation of the library
add_library(score_addon_particles
  particles/Executor.hpp
  particles/Metadata.hpp
  particles/Process.hpp
  particles/Layer.hpp
  particles/Executor.cpp
  particles/Process.cpp
  particles/Node.hpp
  particles/Node.cpp
  particles/Loadmesh.h
  particles/Loadmesh.cpp
  particles/Renderer.cpp
  particles/Renderer.hpp
  particles/MeshParticles.hpp
  particles/MeshParticles.cpp

  score_addon_particles.hpp
  score_addon_particles.cpp
)

# Link
target_link_libraries(score_addon_particles PUBLIC
  score_plugin_gfx
)

# Target-specific options
setup_score_plugin(score_addon_particles)

