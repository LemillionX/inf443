#pragma once

#include "vcl/vcl.hpp"


vcl::vec3 evaluate_terrain(float u, float v);
vcl::mesh create_terrain();
std::vector<vcl::vec3> generate_positions_on_terrain(int N, float dimension);

