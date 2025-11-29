#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <cmath>
#include <cstdint>

// EDEN ENGINE Standard Library
#include "stdlib/vulkan.h"
#include "stdlib/glfw.h"
#include "stdlib/math.h"
#include "stdlib/eden_imgui.h"

// SOA (Structure-of-Arrays) mesh data - optimized for CUDA/OptiX
struct Mesh {
        std::vector<Vec3> positions;
        std::vector<Vec2> uvs;
        std::vector<Vec3> colors;
        std::vector<int32_t> indices;
};

// SOA (Structure-of-Arrays) component - optimized for ECS iteration
struct Velocity {
        std::vector<float> x;
        std::vector<float> y;
        std::vector<float> z;
};

struct Position {
        float x;
        float y;
        float z;
};


int heidic_main();

int heidic_main() {
        std::cout << "SOA types defined successfully" << std::endl;
        return 0;
}

int main(int argc, char* argv[]) {
    heidic_main();
    return 0;
}
