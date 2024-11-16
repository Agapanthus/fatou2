#include "offRenderer.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const std::vector<Vertex> vertices = {
    {{-1.f, -1.f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{1.0f, -1.f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-1.f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};

const std::vector<Vertex2> vertices2 = {{{-1.f, -1.f}, {1.0f, 0.0f}},
                                        {{1.0f, -1.f}, {0.0f, 0.0f}},
                                        {{1.0f, 1.0f}, {0.0f, 1.0f}},
                                        {{-1.f, 1.0f}, {1.0f, 1.0f}}};
const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0};
