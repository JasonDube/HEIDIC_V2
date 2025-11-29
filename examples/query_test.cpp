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

// ECS Query Types
// ECS Query for components: Position, Velocity
struct Query_Position_Velocity {
    std::vector<Position>* position_components;
    std::vector<Velocity>* velocity_components;
    size_t count;  // Number of entities with all components
};

// Helper to iterate over Query_Position_Velocity query
template<typename Func>
void for_each_query_position_velocity(Query_Position_Velocity& query, Func func) {
    for (size_t i = 0; i < query.count; ++i) {
        // Access components for entity i
        Position& position = (*query.position_components)[i];
        Velocity& velocity = (*query.velocity_components)[i];
        // Call function with entity components
        func(position, velocity);
    }
}


struct Position {
        float x;
        float y;
        float z;
};

struct Velocity {
        float x;
        float y;
        float z;
};


void update_transforms(Query_Position_Velocity q);
int heidic_main();

void update_transforms(Query_Position_Velocity q) {
        std::cout << "Query function defined\n" << std::endl;
}

int heidic_main() {
        std::cout << "ECS Query syntax test\n" << std::endl;
        return 0;
}

int main(int argc, char* argv[]) {
    heidic_main();
    return 0;
}
