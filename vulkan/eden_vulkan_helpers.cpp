// EDEN ENGINE Vulkan Helper Functions Implementation
// Updated for Top Down Example (ImGui + Cube + Push Constants + Lines)

#include "eden_vulkan_helpers.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <thread>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <array>
#include <fstream>
#include <cstdlib>  // For std::abs
#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"
#include <sstream>

// ImGui
#include "../third_party/imgui/imgui.h"
#include "../third_party/imgui/backends/imgui_impl_glfw.h"
#include "../third_party/imgui/backends/imgui_impl_vulkan.h"

// Validation Layers & Debug Utils
#ifdef NDEBUG
const bool g_enableValidationLayers = false;
#else
const bool g_enableValidationLayers = true;
#endif

const std::vector<const char*> g_validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> g_debugUtilsExtensions = {
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

// Vertex structure (supports both textured and colored rendering)
struct Vertex {
    float pos[3];
    float uv[2];
    float color[3];  // RGB color for colored rendering
    
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, uv);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

// Uniform buffer object (View/Proj only)
struct UniformBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
};

// Push Constants (Model)
struct PushConsts {
    glm::mat4 model;
};

// Global Vulkan state
static VkInstance g_instance = VK_NULL_HANDLE;
static VkPhysicalDevice g_physicalDevice = VK_NULL_HANDLE;
static VkSurfaceKHR g_surface = VK_NULL_HANDLE;
static VkDevice g_device = VK_NULL_HANDLE;
static VkSwapchainKHR g_swapchain = VK_NULL_HANDLE;
static VkRenderPass g_renderPass = VK_NULL_HANDLE;
static VkPipeline g_pipeline = VK_NULL_HANDLE;
static VkPipeline g_linePipeline = VK_NULL_HANDLE; // Line Pipeline
static VkPipelineLayout g_pipelineLayout = VK_NULL_HANDLE;
static std::vector<VkFramebuffer> g_framebuffers;
static VkCommandPool g_commandPool = VK_NULL_HANDLE;
static std::vector<VkCommandBuffer> g_commandBuffers;
static VkQueue g_graphicsQueue = VK_NULL_HANDLE;
static uint32_t g_graphicsQueueFamilyIndex = 0;
static uint32_t g_currentFrame = 0;
static VkExtent2D g_swapchainExtent = {};

// Additional state
static std::vector<VkImage> g_swapchainImages;
static std::vector<VkImageView> g_swapchainImageViews;
static std::vector<VkSemaphore> g_imageAvailableSemaphores;  // Per swapchain image
static std::vector<VkSemaphore> g_renderFinishedSemaphores;  // Per swapchain image
static VkFence g_inFlightFence = VK_NULL_HANDLE;
static VkFence g_imageAvailableFence = VK_NULL_HANDLE;  // Separate fence for image acquisition
static uint32_t g_swapchainImageCount = 0;
static VkFormat g_swapchainImageFormat = VK_FORMAT_UNDEFINED;

// Depth buffer
static VkImage g_depthImage = VK_NULL_HANDLE;
static VkDeviceMemory g_depthImageMemory = VK_NULL_HANDLE;
static VkImageView g_depthImageView = VK_NULL_HANDLE;
static VkFormat g_depthFormat = VK_FORMAT_D32_SFLOAT;

// Debug messenger (for validation layers)
static VkDebugUtilsMessengerEXT g_debugMessenger = VK_NULL_HANDLE;

// Descriptor sets
static VkDescriptorSetLayout g_descriptorSetLayout = VK_NULL_HANDLE;
static VkDescriptorPool g_descriptorPool = VK_NULL_HANDLE;
static VkDescriptorPool g_imguiDescriptorPool = VK_NULL_HANDLE;
static std::vector<VkDescriptorSet> g_descriptorSets;
static std::vector<VkBuffer> g_uniformBuffers;
static std::vector<VkDeviceMemory> g_uniformBuffersMemory;

// Texture resources (single global texture)
static VkImage g_textureImage = VK_NULL_HANDLE;
static VkDeviceMemory g_textureImageMemory = VK_NULL_HANDLE;
static VkImageView g_textureImageView = VK_NULL_HANDLE;
static VkSampler g_textureSampler = VK_NULL_HANDLE;

// Camera matrices for raycasting
static glm::mat4 g_currentView = glm::mat4(1.0f);
static glm::mat4 g_currentProj = glm::mat4(1.0f);
static glm::vec3 g_currentCamPos = glm::vec3(0.0f);

// Forward declarations
static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
static void createTextureAndDescriptors(GLFWwindow* window);

// Helper: transition image layout
static void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = g_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(g_device, &allocInfo, &cmd);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(
        cmd,
        srcStage, dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_graphicsQueue);

    vkFreeCommandBuffers(g_device, g_commandPool, 1, &cmd);
}

static void createTextureAndDescriptors(GLFWwindow* /*window*/) {
    if (g_textureImage != VK_NULL_HANDLE) {
        // Already created
        // Still need to write descriptors for UBO + sampler
    }

    // Load PNG (test.png) - try multiple paths
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = nullptr;
    
    const char* paths[] = {
        "../models/test.png",      // From examples/ascii_import_test/
        "../../models/test.png",   // From examples/ascii_import_test/ (going up to heidic_v2)
        "models/test.png",         // From heidic_v2/
        "../test.png"              // Fallback
    };
    
    for (int i = 0; i < 4; i++) {
        pixels = stbi_load(paths[i], &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (pixels) {
            std::cout << "Loaded texture from: " << paths[i] << std::endl;
            std::cout.flush();
            break;
        }
    }
    
    // Always use a bright white texture for cubes to ensure full brightness
    // This ensures vertex colors are displayed at full intensity
    bool usingFallback = false;
    if (!pixels) {
        std::cerr << "Failed to load texture image from all paths. Using fallback 1x1 white texture." << std::endl;
        usingFallback = true;
    }
    
    // Force white texture for consistent bright rendering
    // Free any loaded texture and create a white one
    if (pixels && !usingFallback) {
        stbi_image_free(pixels);
    }
    
    // Create a bright white texture (ensures full brightness)
    texWidth = 1;
    texHeight = 1;
    texChannels = 4;
    pixels = new stbi_uc[4];
    pixels[0] = 255; // R - full brightness
    pixels[1] = 255; // G - full brightness
    pixels[2] = 255; // B - full brightness
    pixels[3] = 255; // A
    usingFallback = true; // Mark as fallback so we use delete[] instead of stbi_image_free

    VkDeviceSize imageSize = (VkDeviceSize)(texWidth) * texHeight * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(g_device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vkUnmapMemory(g_device, stagingBufferMemory);

    // Free pixels - always use delete[] since we're now always creating a white texture
    delete[] pixels;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = texWidth;
    imageInfo.extent.height = texHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateImage(g_device, &imageInfo, nullptr, &g_textureImage);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(g_device, g_textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(g_device, &allocInfo, nullptr, &g_textureImageMemory);
    vkBindImageMemory(g_device, g_textureImage, g_textureImageMemory, 0);

    // Transition, copy, transition
    transitionImageLayout(g_textureImage, imageInfo.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkCommandBufferAllocateInfo cbAlloc = {};
    cbAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAlloc.commandPool = g_commandPool;
    cbAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAlloc.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(g_device, &cbAlloc, &cmd);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = { (uint32_t)texWidth, (uint32_t)texHeight, 1 };

    vkCmdCopyBufferToImage(
        cmd,
        stagingBuffer,
        g_textureImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_graphicsQueue);

    vkFreeCommandBuffers(g_device, g_commandPool, 1, &cmd);

    transitionImageLayout(g_textureImage, imageInfo.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(g_device, stagingBuffer, nullptr);
    vkFreeMemory(g_device, stagingBufferMemory, nullptr);

    // Create image view
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = g_textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageInfo.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(g_device, &viewInfo, nullptr, &g_textureImageView);

    // Create sampler
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    vkCreateSampler(g_device, &samplerInfo, nullptr, &g_textureSampler);

    // Write descriptors: UBO + texture sampler
    for (size_t i = 0; i < g_swapchainImageCount; i++) {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = g_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo2 = {};
        imageInfo2.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo2.imageView = g_textureImageView;
        imageInfo2.sampler = g_textureSampler;

        VkWriteDescriptorSet writes[2] = {};
        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = g_descriptorSets[i];
        writes[0].dstBinding = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].descriptorCount = 1;
        writes[0].pBufferInfo = &bufferInfo;

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = g_descriptorSets[i];
        writes[1].dstBinding = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[1].descriptorCount = 1;
        writes[1].pImageInfo = &imageInfo2;

        vkUpdateDescriptorSets(g_device, 2, writes, 0, nullptr);
    }
}

// Cube resources
static VkBuffer g_cubeVertexBuffer = VK_NULL_HANDLE;
static VkDeviceMemory g_cubeVertexMemory = VK_NULL_HANDLE;
static uint32_t g_cubeVertexCount = 0;

// Line resources
static std::vector<Vertex> g_lineVertices;
static VkBuffer g_lineVertexBuffer = VK_NULL_HANDLE;
static VkDeviceMemory g_lineVertexMemory = VK_NULL_HANDLE;
static VkDeviceSize g_lineBufferSize = 1024 * 1024; // 1MB buffer for lines

// Helper to read binary file (for shaders)
static std::vector<char> readFile(const std::string& filename) {
    std::vector<std::string> paths = {
        filename,
        "../" + filename,
        "../../" + filename,
        "../top_down/" + filename,
        "examples/top_down/" + filename,
        "examples/ascii_import_test/" + filename,
        "shaders/" + filename,
        "../shaders/" + filename
    };
    
    for (const auto& path : paths) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (file.is_open()) {
            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), fileSize);
            file.close();
            return buffer;
        }
    }
    return {};
}

// Memory Helper
static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return 0;
}

static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateBuffer(g_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        std::cerr << "failed to create buffer!" << std::endl;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_device, buffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(g_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        std::cerr << "failed to allocate buffer memory!" << std::endl;
    }
    
    vkBindBufferMemory(g_device, buffer, bufferMemory, 0);
}

// Create Cube Vertex Buffer
static void createCube() {
    // 1x1x1 cube centered at origin, with colored faces
    std::vector<Vertex> vertices = {
        // Front face (Red)
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},

        // Back face (Green)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},

        // Top face (Blue)
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},

        // Bottom face (Yellow)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},

        // Right face (Cyan)
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},

        // Left face (Magenta)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}}
    };
    
    g_cubeVertexCount = static_cast<uint32_t>(vertices.size());
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    void* data;
    vkMapMemory(g_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(g_device, stagingBufferMemory);
    
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, g_cubeVertexBuffer, g_cubeVertexMemory);
    
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = g_commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(g_device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(commandBuffer, stagingBuffer, g_cubeVertexBuffer, 1, &copyRegion);
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_graphicsQueue);
    
    vkFreeCommandBuffers(g_device, g_commandPool, 1, &commandBuffer);
    
    vkDestroyBuffer(g_device, stagingBuffer, nullptr);
    vkFreeMemory(g_device, stagingBufferMemory, nullptr);
}

// Internal Helper for Depth
static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(g_physicalDevice, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

static void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    if (vkCreateImage(g_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(g_device, image, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
    
    if (vkAllocateMemory(g_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }
    
    vkBindImageMemory(g_device, image, imageMemory, 0);
}

static void createDepthResources() {
    g_depthFormat = findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    createImage(g_swapchainExtent.width, g_swapchainExtent.height, g_depthFormat,
                VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, g_depthImage, g_depthImageMemory);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = g_depthImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = g_depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(g_device, &viewInfo, nullptr, &g_depthImageView) != VK_SUCCESS) {
        std::cerr << "[EDEN] Failed to create depth image view!" << std::endl;
    }
}

// Queue Family Helper
static uint32_t findGraphicsQueueFamily(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                return i;
            }
        }
    }
    return UINT32_MAX;
}

// Validation Layer Helpers
static bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    for (const char* layerName : g_validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            std::cerr << "[EDEN] Validation layer not found: " << layerName << std::endl;
            return false;
        }
    }
    return true;
}

// Debug Callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    // Filter: Only show errors and warnings in release, everything in debug
    #ifdef NDEBUG
    if (messageSeverity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        return VK_FALSE;
    }
    #endif
    
    std::cerr << "[Vulkan Validation] ";
    
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << "ERROR: ";
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "WARNING: ";
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        std::cerr << "INFO: ";
    } else {
        std::cerr << "VERBOSE: ";
    }
    
    std::cerr << pCallbackData->pMessage << std::endl;
    
    return VK_FALSE;  // Don't abort
}

// Debug Messenger Creation/Destruction
static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static void setupDebugMessenger() {
    if (!g_enableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
    
    if (CreateDebugUtilsMessengerEXT(g_instance, &createInfo, nullptr, &g_debugMessenger) != VK_SUCCESS) {
        std::cerr << "[EDEN] Failed to set up debug messenger!" << std::endl;
        std::cerr.flush();
    } else {
        std::cout << "[EDEN] Debug messenger initialized" << std::endl;
        std::cout.flush();
    }
}


// Configure GLFW for Vulkan
extern "C" void heidic_glfw_vulkan_hints() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
}

// WRAPPERS for GLFW
extern "C" int heidic_glfw_init() { return glfwInit(); }
extern "C" void heidic_glfw_terminate() { glfwTerminate(); }
// Global window reference for fullscreen toggle
static GLFWwindow* g_mainWindow = nullptr;
static int g_windowWidth = 1280;
static int g_windowHeight = 720;

extern "C" GLFWwindow* heidic_create_window(int width, int height, const char* title) {
    g_windowWidth = width;
    g_windowHeight = height;
    g_mainWindow = glfwCreateWindow(width, height, title, NULL, NULL);
    return g_mainWindow;
}
extern "C" void heidic_destroy_window(GLFWwindow* window) { glfwDestroyWindow(window); }
extern "C" void heidic_set_window_should_close(GLFWwindow* window, int value) { glfwSetWindowShouldClose(window, value); }
extern "C" int heidic_get_key(GLFWwindow* window, int key) { return glfwGetKey(window, key); }

// Initialize Vulkan renderer
extern "C" int heidic_init_renderer(GLFWwindow* window) {
    if (window == nullptr) return 0;
    
    try {
        // ... (Instance, Device, Swapchain, etc. - mostly same) ...
        // I will abbreviate the standard setup to save tokens where it's unchanged, 
        // but since I'm rewriting, I must include ALL of it.
        
        // 1. Instance
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        // Check validation layer support
        if (g_enableValidationLayers && !checkValidationLayerSupport()) {
            std::cerr << "[EDEN] Validation layers requested but not available!" << std::endl;
            std::cerr << "[EDEN] Continuing without validation layers..." << std::endl;
        }
        
        // Build extension list (GLFW extensions + debug utils if needed)
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (g_enableValidationLayers) {
            extensions.insert(extensions.end(), g_debugUtilsExtensions.begin(), g_debugUtilsExtensions.end());
        }
        
        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        instanceInfo.ppEnabledExtensionNames = extensions.data();
        
        // Enable validation layers if requested
        if (g_enableValidationLayers && checkValidationLayerSupport()) {
            instanceInfo.enabledLayerCount = static_cast<uint32_t>(g_validationLayers.size());
            instanceInfo.ppEnabledLayerNames = g_validationLayers.data();
            std::cout << "[EDEN] Validation layers enabled" << std::endl;
            std::cout.flush();
        } else {
            instanceInfo.enabledLayerCount = 0;
        }
        
        if (vkCreateInstance(&instanceInfo, nullptr, &g_instance) != VK_SUCCESS) return 0;
        
        // Setup debug messenger (must be after instance creation)
        setupDebugMessenger();

        // 2. Surface
        if (glfwCreateWindowSurface(g_instance, window, nullptr, &g_surface) != VK_SUCCESS) return 0;

        // 3. Physical Device
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(g_instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(g_instance, &deviceCount, devices.data());
        g_physicalDevice = devices[0];

        // 4. Queue Family
        g_graphicsQueueFamilyIndex = findGraphicsQueueFamily(g_physicalDevice, g_surface);
        
        // 5. Logical Device
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = g_graphicsQueueFamilyIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriority;
        
        const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.enabledExtensionCount = 1;
        deviceInfo.ppEnabledExtensionNames = deviceExtensions;
        
        if (vkCreateDevice(g_physicalDevice, &deviceInfo, nullptr, &g_device) != VK_SUCCESS) return 0;
        vkGetDeviceQueue(g_device, g_graphicsQueueFamilyIndex, 0, &g_graphicsQueue);
        
        // 6. Swapchain
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physicalDevice, g_surface, &capabilities);
        g_swapchainExtent = capabilities.currentExtent;
        g_swapchainImageCount = 3; 
        
        VkSwapchainCreateInfoKHR swapchainInfo = {};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainInfo.surface = g_surface;
        swapchainInfo.minImageCount = g_swapchainImageCount;
        swapchainInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
        swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainInfo.imageExtent = g_swapchainExtent;
        swapchainInfo.imageArrayLayers = 1;
        swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.preTransform = capabilities.currentTransform;
        swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapchainInfo.clipped = VK_TRUE;
        
        if (vkCreateSwapchainKHR(g_device, &swapchainInfo, nullptr, &g_swapchain) != VK_SUCCESS) return 0;
        g_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

        vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_swapchainImageCount, nullptr);
        g_swapchainImages.resize(g_swapchainImageCount);
        vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_swapchainImageCount, g_swapchainImages.data());

        g_swapchainImageViews.resize(g_swapchainImageCount);
        for (size_t i = 0; i < g_swapchainImageCount; i++) {
            VkImageViewCreateInfo viewInfo = {};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = g_swapchainImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = g_swapchainImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;
            vkCreateImageView(g_device, &viewInfo, nullptr, &g_swapchainImageViews[i]);
        }

        // 7. Command Pool
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = g_graphicsQueueFamilyIndex;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkCreateCommandPool(g_device, &poolInfo, nullptr, &g_commandPool);

        // 8. Depth
        createDepthResources();

        // 9. Render Pass
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = g_swapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = g_depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorRef = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthRef = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;
        subpass.pDepthStencilAttachment = &depthRef;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(g_device, &renderPassInfo, nullptr, &g_renderPass) != VK_SUCCESS) return 0;

        // 10. Descriptor Set Layout (UBO + Texture)
        VkDescriptorSetLayoutBinding bindings[2] = {};
        // UBO at binding 0
        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        // Texture sampler at binding 1
        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 2;
        layoutInfo.pBindings = bindings;
        vkCreateDescriptorSetLayout(g_device, &layoutInfo, nullptr, &g_descriptorSetLayout);

        // 11. Pipeline (Triangles)
        auto vertCode = readFile("vert_cube.spv");
        auto fragCode = readFile("frag_cube.spv");
        if (vertCode.empty() || fragCode.empty()) return 0;

        VkShaderModule vertModule, fragModule;
        VkShaderModuleCreateInfo createInfo2 = {};
        createInfo2.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo2.codeSize = vertCode.size();
        createInfo2.pCode = reinterpret_cast<const uint32_t*>(vertCode.data());
        vkCreateShaderModule(g_device, &createInfo2, nullptr, &vertModule);

        createInfo2.codeSize = fragCode.size();
        createInfo2.pCode = reinterpret_cast<const uint32_t*>(fragCode.data());
        vkCreateShaderModule(g_device, &createInfo2, nullptr, &fragModule);

        VkPipelineShaderStageCreateInfo shaderStages[] = {
            {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, vertModule, "main", nullptr},
            {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, fragModule, "main", nullptr}
        };

        // Vertex Input (Uses Vertex struct)
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();
        
        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkViewport viewport = {0.0f, 0.0f, (float)g_swapchainExtent.width, (float)g_swapchainExtent.height, 0.0f, 1.0f};
        VkRect2D scissor = {{0, 0}, g_swapchainExtent};
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.lineWidth = 1.0f; // Standard line width (Vulkan requires 1.0 unless wideLines feature is enabled)
        rasterizer.cullMode = VK_CULL_MODE_NONE; // Disable culling so imported models render regardless of winding
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil = {};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

        VkPipelineColorBlendAttachmentState colorBlend = {};
        colorBlend.colorWriteMask = 0xF;
        colorBlend.blendEnable = VK_FALSE;
        
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlend;

        // Push Constant
        // Note: Shader expects 68 bytes, but sizeof(glm::mat4) is 64 bytes
        // Increase to 128 bytes (aligned to 16) to satisfy shader requirements
        VkPushConstantRange pushConstantRange = {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = 128; // Increased from sizeof(PushConsts) to satisfy shader alignment

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &g_descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        vkCreatePipelineLayout(g_device, &pipelineLayoutInfo, nullptr, &g_pipelineLayout);

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = g_pipelineLayout;
        pipelineInfo.renderPass = g_renderPass;
        pipelineInfo.subpass = 0;
        
        vkCreateGraphicsPipelines(g_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &g_pipeline);

        // LINE PIPELINE
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        rasterizer.cullMode = VK_CULL_MODE_NONE; // No culling for lines
        depthStencil.depthTestEnable = VK_FALSE; // Disable depth test for lines (draw on top)
        vkCreateGraphicsPipelines(g_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &g_linePipeline);

        vkDestroyShaderModule(g_device, vertModule, nullptr);
        vkDestroyShaderModule(g_device, fragModule, nullptr);

        // 12. Framebuffers
        g_framebuffers.resize(g_swapchainImageCount);
        for (size_t i = 0; i < g_swapchainImageCount; i++) {
            std::array<VkImageView, 2> attachments = { g_swapchainImageViews[i], g_depthImageView };
            VkFramebufferCreateInfo fbInfo = {};
            fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbInfo.renderPass = g_renderPass;
            fbInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            fbInfo.pAttachments = attachments.data();
            fbInfo.width = g_swapchainExtent.width;
            fbInfo.height = g_swapchainExtent.height;
            fbInfo.layers = 1;
            vkCreateFramebuffer(g_device, &fbInfo, nullptr, &g_framebuffers[i]);
        }

        // 13. Uniform Buffers
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        g_uniformBuffers.resize(g_swapchainImageCount);
        g_uniformBuffersMemory.resize(g_swapchainImageCount);
        for (size_t i = 0; i < g_swapchainImageCount; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, g_uniformBuffers[i], g_uniformBuffersMemory[i]);
        }

        // 14. Descriptor Pool
        VkDescriptorPoolSize poolSizes[2] = {};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(g_swapchainImageCount);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(g_swapchainImageCount);

        VkDescriptorPoolCreateInfo poolInfo2 = {};
        poolInfo2.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo2.poolSizeCount = 2;
        poolInfo2.pPoolSizes = poolSizes;
        poolInfo2.maxSets = static_cast<uint32_t>(g_swapchainImageCount);
        vkCreateDescriptorPool(g_device, &poolInfo2, nullptr, &g_descriptorPool);

        // 15. Descriptor Sets
        std::vector<VkDescriptorSetLayout> layouts(g_swapchainImageCount, g_descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = g_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(g_swapchainImageCount);
        allocInfo.pSetLayouts = layouts.data();
        g_descriptorSets.resize(g_swapchainImageCount);
        vkAllocateDescriptorSets(g_device, &allocInfo, g_descriptorSets.data());

        // Create texture and write descriptors (UBO + sampler)
        createTextureAndDescriptors(window);

        // 16. Command Buffers
        g_commandBuffers.resize(g_swapchainImageCount);
        VkCommandBufferAllocateInfo cbAllocInfo = {};
        cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbAllocInfo.commandPool = g_commandPool;
        cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cbAllocInfo.commandBufferCount = (uint32_t)g_commandBuffers.size();
        vkAllocateCommandBuffers(g_device, &cbAllocInfo, g_commandBuffers.data());

        // 17. Sync Objects (per swapchain image to avoid semaphore reuse)
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        
        g_imageAvailableSemaphores.resize(g_swapchainImageCount);
        g_renderFinishedSemaphores.resize(g_swapchainImageCount);
        
        for (size_t i = 0; i < g_swapchainImageCount; i++) {
            if (vkCreateSemaphore(g_device, &semaphoreInfo, nullptr, &g_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(g_device, &semaphoreInfo, nullptr, &g_renderFinishedSemaphores[i]) != VK_SUCCESS) {
                std::cerr << "[EDEN] Failed to create semaphores!" << std::endl;
                return 0;
            }
        }
        
        vkCreateFence(g_device, &fenceInfo, nullptr, &g_inFlightFence);
        
        // Create separate fence for image acquisition
        fenceInfo.flags = 0;  // Not signaled initially
        vkCreateFence(g_device, &fenceInfo, nullptr, &g_imageAvailableFence);

        // 18. Cube
        createCube();

        // 19. Line Buffer
        createBuffer(g_lineBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, g_lineVertexBuffer, g_lineVertexMemory);

        // 20. ImGui Init
        VkDescriptorPoolSize imguiPoolSizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo imguiPoolInfo = {};
        imguiPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        imguiPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        imguiPoolInfo.maxSets = 1000 * IM_ARRAYSIZE(imguiPoolSizes);
        imguiPoolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(imguiPoolSizes);
        imguiPoolInfo.pPoolSizes = imguiPoolSizes;
        vkCreateDescriptorPool(g_device, &imguiPoolInfo, nullptr, &g_imguiDescriptorPool);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window, true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = g_instance;
        init_info.PhysicalDevice = g_physicalDevice;
        init_info.Device = g_device;
        init_info.QueueFamily = g_graphicsQueueFamilyIndex;
        init_info.Queue = g_graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = g_imguiDescriptorPool;
        init_info.MinImageCount = g_swapchainImageCount;
        init_info.ImageCount = g_swapchainImageCount;
        init_info.Allocator = nullptr;
        init_info.PipelineInfoMain.RenderPass = g_renderPass;
        init_info.PipelineInfoMain.Subpass = 0;
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        
        ImGui_ImplVulkan_Init(&init_info);
        
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "[EDEN] Init Error: " << e.what() << std::endl;
        return 0;
    }
}

extern "C" void heidic_cleanup_renderer() {
    vkDeviceWaitIdle(g_device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // Cleanup debug messenger
    if (g_enableValidationLayers && g_debugMessenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(g_instance, g_debugMessenger, nullptr);
        g_debugMessenger = VK_NULL_HANDLE;
    }
    
    // Cleanup rest (simplified)
}

extern "C" int heidic_window_should_close(GLFWwindow* window) {
    return glfwWindowShouldClose(window);
}

extern "C" void heidic_poll_events() {
    glfwPollEvents();
}

extern "C" int heidic_is_key_pressed(GLFWwindow* window, int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

extern "C" int heidic_is_mouse_button_pressed(GLFWwindow* window, int button) {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

// FRAME CONTROL
extern "C" void heidic_begin_frame() {
    // Start ImGui Frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Clear lines for this frame
    g_lineVertices.clear();

    vkWaitForFences(g_device, 1, &g_inFlightFence, VK_TRUE, UINT64_MAX);
    
    uint32_t imageIndex;
    // Acquire next image using fence (required: either semaphore or fence must be non-NULL)
    // We'll use imageIndex to select the correct semaphore for render-finished signal
    vkResetFences(g_device, 1, &g_imageAvailableFence);
    VkResult result = vkAcquireNextImageKHR(g_device, g_swapchain, UINT64_MAX, VK_NULL_HANDLE, g_imageAvailableFence, &imageIndex);
    vkWaitForFences(g_device, 1, &g_imageAvailableFence, VK_TRUE, UINT64_MAX);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return;
    }
    
    // Use the actual imageIndex we got from acquire to select the correct semaphores
    // This ensures each swapchain image uses its own semaphore (fixes validation error)
    g_currentFrame = imageIndex;
    vkResetFences(g_device, 1, &g_inFlightFence);
    vkResetCommandBuffer(g_commandBuffers[g_currentFrame], 0);
    
    VkCommandBuffer cb = g_commandBuffers[g_currentFrame];
    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(cb, &beginInfo);
    
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = g_renderPass;
    renderPassInfo.framebuffer = g_framebuffers[g_currentFrame];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = g_swapchainExtent;
    
    VkClearValue clearValues[2];
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;
    
    vkCmdBeginRenderPass(cb, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline);
    
    // Bind global UBO
    vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipelineLayout, 0, 1, &g_descriptorSets[g_currentFrame], 0, nullptr);

    // Bind Cube VB
    VkBuffer vertexBuffers[] = {g_cubeVertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
}

extern "C" void heidic_end_frame() {
    VkCommandBuffer cb = g_commandBuffers[g_currentFrame];
    
    // Draw Lines
    if (!g_lineVertices.empty()) {
        void* data;
        vkMapMemory(g_device, g_lineVertexMemory, 0, sizeof(Vertex) * g_lineVertices.size(), 0, &data);
        memcpy(data, g_lineVertices.data(), sizeof(Vertex) * g_lineVertices.size());
        vkUnmapMemory(g_device, g_lineVertexMemory);

        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, g_linePipeline);
        
        // Bind descriptor set for view/projection matrices (lines need this too)
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipelineLayout, 0, 1, &g_descriptorSets[g_currentFrame], 0, nullptr);
        
        // Push Identity Model Matrix for lines
        PushConsts push;
        push.model = glm::mat4(1.0f);
        vkCmdPushConstants(cb, g_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConsts), &push);

        VkBuffer vertexBuffers[] = {g_lineVertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
        
        vkCmdDraw(cb, static_cast<uint32_t>(g_lineVertices.size()), 1, 0, 0);
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb);
    
    vkCmdEndRenderPass(cb);
    vkEndCommandBuffer(cb);
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    // Don't wait on a semaphore since we didn't use one with acquire
    // The fence already ensures the image is ready
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cb;
    VkSemaphore signalSemaphores[] = {g_renderFinishedSemaphores[g_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, g_inFlightFence);
    
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {g_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &g_currentFrame;
    
    vkQueuePresentKHR(g_graphicsQueue, &presentInfo);
}

// DRAW CUBE
extern "C" void heidic_draw_cube(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz) {
    // Construct Model Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));
    
    // Apply rotations (Degrees!)
    model = glm::rotate(model, glm::radians(rx), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rz), glm::vec3(0.0f, 0.0f, 1.0f));
    
    model = glm::scale(model, glm::vec3(sx, sy, sz));
    
    PushConsts push = {model};
    vkCmdPushConstants(g_commandBuffers[g_currentFrame], g_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConsts), &push);
    
    vkCmdDraw(g_commandBuffers[g_currentFrame], g_cubeVertexCount, 1, 0, 0);
}

// DRAW LINES
extern "C" void heidic_draw_line(float x1, float y1, float z1, float x2, float y2, float z2, float r, float g, float b) {
    // Lines use vertex colors, not textures
    Vertex v1 = {{x1, y1, z1}, {0.0f, 0.0f}, {r, g, b}};
    Vertex v2 = {{x2, y2, z2}, {0.0f, 0.0f}, {r, g, b}};
    g_lineVertices.push_back(v1);
    g_lineVertices.push_back(v2);
}

extern "C" void heidic_draw_model_origin(float x, float y, float z, float rx, float ry, float rz, float length) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));
    model = glm::rotate(model, glm::radians(rx), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rz), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 origin = glm::vec3(model * glm::vec4(0,0,0,1));
    glm::vec3 axisX = glm::vec3(model * glm::vec4(1,0,0,1)); 
    glm::vec3 axisY = glm::vec3(model * glm::vec4(0,1,0,1));
    glm::vec3 axisZ = glm::vec3(model * glm::vec4(0,0,1,1));
    
    // Scale to specified length (in units, where 1 unit = 1 cm)
    axisX = origin + glm::normalize(axisX - origin) * length;
    axisY = origin + glm::normalize(axisY - origin) * length;
    axisZ = origin + glm::normalize(axisZ - origin) * length;

    heidic_draw_line(origin.x, origin.y, origin.z, axisX.x, axisX.y, axisX.z, 1, 0, 0); // Red (X)
    heidic_draw_line(origin.x, origin.y, origin.z, axisY.x, axisY.y, axisY.z, 0, 1, 0); // Green (Y)
    heidic_draw_line(origin.x, origin.y, origin.z, axisZ.x, axisZ.y, axisZ.z, 1, 1, 0); // Yellow (Z)
}

extern "C" void heidic_update_camera(float px, float py, float pz, float rx, float ry, float rz) {
    heidic_update_camera_with_far(px, py, pz, rx, ry, rz, 5000.0f);
}

extern "C" void heidic_update_camera_with_far(float px, float py, float pz, float rx, float ry, float rz, float far_plane) {
    // View Matrix from Pos/Rot
    glm::mat4 cam = glm::mat4(1.0f);
    cam = glm::translate(cam, glm::vec3(px, py, pz));
    cam = glm::rotate(cam, glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));
    cam = glm::rotate(cam, glm::radians(rx), glm::vec3(1.0f, 0.0f, 0.0f));
    cam = glm::rotate(cam, glm::radians(rz), glm::vec3(0.0f, 0.0f, 1.0f));
    
    glm::mat4 view = glm::inverse(cam);
    
    // Proj
    float aspect = (float)g_swapchainExtent.width / (float)g_swapchainExtent.height;
    glm::mat4 proj = glm::perspectiveRH_ZO(glm::radians(60.0f), aspect, 0.1f, far_plane);
    proj[1][1] *= -1;
    
    // Store matrices for raycasting
    g_currentView = view;
    g_currentProj = proj;
    g_currentCamPos = glm::vec3(px, py, pz);
    
    // Update UBO
    UniformBufferObject ubo = {};
    ubo.view = view;
    ubo.proj = proj;
    
    void* data;
    vkMapMemory(g_device, g_uniformBuffersMemory[g_currentFrame], 0, sizeof(UniformBufferObject), 0, &data);
    memcpy(data, &ubo, sizeof(UniformBufferObject));
    vkUnmapMemory(g_device, g_uniformBuffersMemory[g_currentFrame]);
}

extern "C" Camera heidic_create_camera(Vec3 pos, Vec3 rot, float clip_near, float clip_far) {
    Camera cam;
    cam.pos = pos;
    cam.rot = rot;
    cam.clip_near = clip_near;
    cam.clip_far = clip_far;
    return cam;
}

extern "C" void heidic_update_camera_from_struct(Camera camera) {
    // Extract values from Camera struct and update camera
    heidic_update_camera_with_far(
        camera.pos.x, camera.pos.y, camera.pos.z,
        camera.rot.x, camera.rot.y, camera.rot.z,
        camera.clip_far
    );
    // Note: clip_near is currently hardcoded to 0.1f in the projection matrix
    // Could be made configurable in the future
}

extern "C" void heidic_set_video_mode(int windowed) {
    if (g_mainWindow == nullptr) return;
    
    if (windowed == 0) {
        // Fullscreen mode
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(g_mainWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    } else {
        // Windowed mode
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        // Center the window
        int x = (mode->width - g_windowWidth) / 2;
        int y = (mode->height - g_windowHeight) / 2;
        glfwSetWindowMonitor(g_mainWindow, NULL, x, y, g_windowWidth, g_windowHeight, 0);
    }
}

// IMGUI WRAPPERS
extern "C" void heidic_imgui_init(GLFWwindow* window) {
}
extern "C" void heidic_imgui_begin(const char* name) {
    ImGui::Begin(name);
}
extern "C" void heidic_imgui_end() {
    ImGui::End();
}
extern "C" void heidic_imgui_text(const char* text) {
    ImGui::Text("%s", text);
}
extern "C" void heidic_imgui_text_float(const char* label, float value) {
    ImGui::Text("%s: %.3f", label, value);
}
extern "C" bool heidic_imgui_drag_float3(const char* label, Vec3* v, float speed) {
    return ImGui::DragFloat3(label, (float*)v, speed);
}
extern "C" Vec3 heidic_imgui_drag_float3_val(const char* label, Vec3 v, float speed) {
    ImGui::DragFloat3(label, (float*)&v, speed);
    return v;
}
extern "C" float heidic_imgui_drag_float(const char* label, float v, float speed) {
    ImGui::DragFloat(label, &v, speed);
    return v;
}

extern "C" float heidic_get_fps() {
    ImGuiIO& io = ImGui::GetIO();
    return io.Framerate;
}

// VECTOR OPERATIONS
extern "C" Vec3 heidic_vec3(float x, float y, float z) {
    return Vec3(x, y, z);
}

extern "C" Vec3 heidic_vec3_add(Vec3 a, Vec3 b) {
    return Vec3(glm::vec3(a) + glm::vec3(b));
}

extern "C" Vec3 heidic_vec_copy(Vec3 src) {
    return src; // Simple copy - returns the Vec3 value
}

// Camera attachment - returns new camera transform matching player
// Since HEIDIC doesn't support pointers yet, this returns values that can be assigned
extern "C" Vec3 heidic_attach_camera_translation(Vec3 player_translation) {
    return player_translation;
}

extern "C" Vec3 heidic_attach_camera_rotation(Vec3 player_rotation) {
    return player_rotation;
}

// MATH
extern "C" float heidic_convert_degrees_to_radians(float degrees) {
    return glm::radians(degrees);
}
extern "C" float heidic_convert_radians_to_degrees(float radians) {
    return glm::degrees(radians);
}
extern "C" float heidic_sin(float radians) {
    return std::sin(radians);
}
extern "C" float heidic_cos(float radians) {
    return std::cos(radians);
}

// Mesh storage
struct Mesh {
    std::vector<Vertex> vertices;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
    uint32_t vertexCount = 0;
};

static std::vector<Mesh> g_meshes;
static int g_nextMeshId = 0;

// ASCII Model Loader
extern "C" int heidic_load_ascii_model(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // Try relative paths
        std::vector<std::string> paths = {
            std::string(filename),
            std::string("../") + filename,
            std::string("models/") + filename,
            std::string("../models/") + filename
        };
        bool opened = false;
        for (const auto& path : paths) {
            file.open(path);
            if (file.is_open()) {
                opened = true;
                break;
            }
        }
        if (!opened) {
            std::cerr << "Failed to open model file: " << filename << std::endl;
            return -1;
        }
    }
    
    Mesh mesh;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<std::vector<int>> triangles; // vertex indices
    std::vector<std::vector<int>> uvTriangles; // UV indices
    
    std::string line;
    bool inGroup = false;
    bool readingVertices = false;
    bool readingTriangles = false;
    bool readingSkinPoints = false;
    bool readingSkinTriangles = false;
    int vertexCount = 0;
    int triangleCount = 0;
    int uvCount = 0;
    int skinTriangleCount = 0;
    int currentTriangle = 0;
    
    while (std::getline(file, line)) {
        // Trim whitespace safely
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        
        if (line.empty()) continue;
        
        // Check for sections
        if (line.find("Vertices:") == 0) {
            sscanf(line.c_str(), "Vertices: %d;", &vertexCount);
            readingVertices = true;
            readingTriangles = false;
            readingSkinPoints = false;
            readingSkinTriangles = false;
            positions.clear();
            continue;
        }
        
        if (line.find("Triangles:") == 0) {
            sscanf(line.c_str(), "Triangles: %d;", &triangleCount);
            readingVertices = false;
            readingTriangles = true;
            readingSkinPoints = false;
            readingSkinTriangles = false;
            triangles.clear();
            continue;
        }
        
        if (line.find("SkinPoints:") == 0) {
            sscanf(line.c_str(), "SkinPoints: %d;", &uvCount);
            readingVertices = false;
            readingTriangles = false;
            readingSkinPoints = true;
            readingSkinTriangles = false;
            uvs.clear();
            continue;
        }
        
        if (line.find("SkinTriangles:") == 0) {
            sscanf(line.c_str(), "SkinTriangles: %d;", &skinTriangleCount);
            readingVertices = false;
            readingTriangles = false;
            readingSkinPoints = false;
            readingSkinTriangles = true;
            uvTriangles.clear();
            currentTriangle = 0;
            continue;
        }
        
        // Parse vertices
        if (readingVertices && !line.empty() && line.back() == ';') {
            float x, y, z;
            if (sscanf(line.c_str(), "%f %f %f;", &x, &y, &z) == 3) {
                positions.push_back(glm::vec3(x, y, z));
            }
        }
        
        // Parse triangles
        if (readingTriangles && !line.empty() && line.back() == ';') {
            int v1, v2, v3;
            if (sscanf(line.c_str(), "%d %d %d;", &v1, &v2, &v3) == 3) {
                triangles.push_back({v1, v2, v3});
            }
        }
        
        // Parse UVs
        if (readingSkinPoints && !line.empty() && line.back() == ';') {
            float u, v;
            if (sscanf(line.c_str(), "%f %f;", &u, &v) == 2) {
                uvs.push_back(glm::vec2(u, v));
            }
        }
        
        // Parse UV triangles
        if (readingSkinTriangles && !line.empty() && line.back() == ';') {
            int triIdx, uv1, uv2, uv3;
            if (sscanf(line.c_str(), "%d, %d %d %d;", &triIdx, &uv1, &uv2, &uv3) == 4) {
                uvTriangles.push_back({uv1, uv2, uv3});
            }
        }
    }
    
    file.close();
    
    // Build vertex buffer from positions, triangles, and UVs
    // Note: Imported model units are assumed to be meters; EDEN uses centimeters (1 unit = 1 cm)
    // So we scale positions by 100.0 to bring them into world scale.
    const float POSITION_SCALE = 100.0f;
    for (size_t i = 0; i < triangles.size(); i++) {
        const auto& tri = triangles[i];
        const auto& uvTri = (i < uvTriangles.size()) ? uvTriangles[i] : std::vector<int>{0, 0, 0};
        
        for (int j = 0; j < 3; j++) {
            Vertex v;
            int vIdx = tri[j];
            int uvIdx = (j < (int)uvTri.size()) ? uvTri[j] : 0;
            
            if (vIdx < static_cast<int>(positions.size())) {
                v.pos[0] = positions[vIdx].x * POSITION_SCALE;
                v.pos[1] = positions[vIdx].y * POSITION_SCALE;
                v.pos[2] = positions[vIdx].z * POSITION_SCALE;
            } else {
                v.pos[0] = v.pos[1] = v.pos[2] = 0.0f;
            }
            
            if (uvIdx < static_cast<int>(uvs.size())) {
                v.uv[0] = uvs[uvIdx].x;
                v.uv[1] = 1.0f - uvs[uvIdx].y; // Flip V for Vulkan
            } else {
                v.uv[0] = 0.0f;
                v.uv[1] = 0.0f;
            }
            
            // Set color to white for textured meshes (so texture * white = texture)
            v.color[0] = 1.0f;
            v.color[1] = 1.0f;
            v.color[2] = 1.0f;
            
            mesh.vertices.push_back(v);
        }
    }
    
    mesh.vertexCount = static_cast<uint32_t>(mesh.vertices.size());
    
    if (mesh.vertexCount == 0) {
        std::cerr << "No vertices loaded from model: " << filename << std::endl;
        return -1;
    }
    
    // Create vertex buffer
    VkDeviceSize bufferSize = sizeof(Vertex) * mesh.vertices.size();
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    void* data;
    vkMapMemory(g_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mesh.vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(g_device, stagingBufferMemory);
    
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.vertexBuffer, mesh.vertexMemory);
    
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = g_commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(g_device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    VkBufferCopy copyRegion = {};
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(commandBuffer, stagingBuffer, mesh.vertexBuffer, 1, &copyRegion);
    
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_graphicsQueue);
    
    vkFreeCommandBuffers(g_device, g_commandPool, 1, &commandBuffer);
    vkDestroyBuffer(g_device, stagingBuffer, nullptr);
    vkFreeMemory(g_device, stagingBufferMemory, nullptr);
    
    int meshId = g_nextMeshId++;
    g_meshes.push_back(mesh);
    
    std::cout << "Loaded mesh " << meshId << " with " << mesh.vertexCount << " vertices from " << filename << std::endl;
    return meshId;
}

extern "C" void heidic_draw_mesh(int mesh_id, float x, float y, float z, float rx, float ry, float rz) {
    if (mesh_id < 0 || mesh_id >= static_cast<int>(g_meshes.size())) {
        return;
    }
    
    Mesh& mesh = g_meshes[mesh_id];
    if (mesh.vertexCount == 0) return;
    
    // Construct Model Matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));
    model = glm::rotate(model, glm::radians(rx), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rz), glm::vec3(0.0f, 0.0f, 1.0f));
    
    PushConsts push = {model};
    VkCommandBuffer cb = g_commandBuffers[g_currentFrame];
    vkCmdPushConstants(cb, g_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConsts), &push);
    
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline);
    
    VkBuffer vertexBuffers[] = {mesh.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
    
    vkCmdDraw(cb, mesh.vertexCount, 1, 0, 0);
}

extern "C" void heidic_sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// ============================================================================
// RAYCASTING FUNCTIONS
// ============================================================================

// AABB structure for raycasting
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    
    AABB() : min(0.0f), max(0.0f) {}
    AABB(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}
};

// Get mouse position in screen coordinates (0,0 = top-left)
extern "C" float heidic_get_mouse_x(GLFWwindow* window) {
    double dx, dy;
    glfwGetCursorPos(window, &dx, &dy);
    return (float)dx;
}

extern "C" float heidic_get_mouse_y(GLFWwindow* window) {
    double dx, dy;
    glfwGetCursorPos(window, &dx, &dy);
    return (float)dy;
}

extern "C" float heidic_get_mouse_scroll_y(GLFWwindow* window) {
    ImGuiIO& io = ImGui::GetIO();
    // Return ImGui's scroll value - it will be 0 if ImGui consumed it
    // We'll use it for zoom even if ImGui might have used it for scrolling panels
    // The game logic can decide whether to use it based on context (e.g., only in top-down mode)
    return io.MouseWheel;
}

// Convert mouse screen position to normalized device coordinates (NDC)
// NDC: x,y in [-1, 1]
// Vulkan NDC: Y=1 is top, Y=-1 is bottom (Y points down)
// GLFW screen: Y=0 is top, Y=height is bottom
static glm::vec2 screenToNDC(float screenX, float screenY, int width, int height) {
    float ndcX = (2.0f * screenX / width) - 1.0f;
    // Map screen Y=0 (top) to NDC Y=-1 (top), screen Y=height (bottom) to NDC Y=1 (bottom)
    float ndcY = (2.0f * screenY / height) - 1.0f;
    return glm::vec2(ndcX, ndcY);
}

// Unproject: Convert NDC coordinates to world-space ray
// Returns ray origin and direction
// CORRECT VERSION: NDC Z is always [-1, 1] regardless of depth buffer format
// perspectiveRH_ZO affects depth buffer mapping, but NDC Z is still [-1, 1]
static void unproject(glm::vec2 ndc, glm::mat4 invProj, glm::mat4 invView, glm::vec3& rayOrigin, glm::vec3& rayDir) {
    // NDC clip space: Z = -1 (near), Z = +1 (far)
    // This is correct for both OpenGL and Vulkan NDC coordinates
    glm::vec4 clipNear = glm::vec4(ndc.x, ndc.y, -1.0f, 1.0f);  // Near plane Z = -1
    glm::vec4 clipFar = glm::vec4(ndc.x, ndc.y, 1.0f, 1.0f);    // Far plane Z = +1
    
    // Transform to eye space (view space)
    glm::vec4 eyeNear = invProj * clipNear;
    eyeNear /= eyeNear.w;  // Perspective divide
    
    glm::vec4 eyeFar = invProj * clipFar;
    eyeFar /= eyeFar.w;  // Perspective divide
    
    // Transform to world space
    glm::vec4 worldNear = invView * eyeNear;
    glm::vec4 worldFar = invView * eyeFar;
    
    glm::vec3 worldNearPoint = glm::vec3(worldNear);
    glm::vec3 worldFarPoint = glm::vec3(worldFar);
    
    // Ray origin is camera position (as per user's working code)
    rayOrigin = g_currentCamPos;
    
    // Ray direction: from near point to far point (normalized)
    // This gives us the correct direction regardless of distance
    glm::vec3 dirVec = worldFarPoint - worldNearPoint;
    float dirLen = glm::length(dirVec);
    if (dirLen > 0.0001f) {
        rayDir = dirVec / dirLen;
    } else {
        // Fallback: if near and far are too close, use direction from camera to far point
        rayDir = glm::normalize(worldFarPoint - g_currentCamPos);
    }
}

// Ray-AABB intersection using Möller-Trumbore slab method
// Returns true if ray hits AABB, and t (distance along ray) if hit
static bool rayAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const AABB& box, float& tMin, float& tMax) {
    // Ensure ray direction is normalized (should already be, but double-check for precision)
    glm::vec3 dir = glm::normalize(rayDir);
    
    // Handle division by zero by using a very small epsilon
    const float epsilon = 1e-6f;
    glm::vec3 invDir;
    invDir.x = (fabsf(dir.x) < epsilon) ? (dir.x >= 0.0f ? 1e6f : -1e6f) : (1.0f / dir.x);
    invDir.y = (fabsf(dir.y) < epsilon) ? (dir.y >= 0.0f ? 1e6f : -1e6f) : (1.0f / dir.y);
    invDir.z = (fabsf(dir.z) < epsilon) ? (dir.z >= 0.0f ? 1e6f : -1e6f) : (1.0f / dir.z);
    
    glm::vec3 t0 = (box.min - rayOrigin) * invDir;
    glm::vec3 t1 = (box.max - rayOrigin) * invDir;
    
    glm::vec3 tMinVec = glm::min(t0, t1);
    glm::vec3 tMaxVec = glm::max(t0, t1);
    
    tMin = glm::max(glm::max(tMinVec.x, tMinVec.y), tMinVec.z);
    tMax = glm::min(glm::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);
    
    // Ray hits if tMax >= tMin AND tMax >= 0 (intersection is in front of or at ray origin)
    // If tMin > tMax, the ray misses the AABB
    // If tMax < 0, the entire AABB is behind the ray origin
    // If tMin < 0 and tMax >= 0, the ray origin is inside the AABB (hit!)
    
    // DEBUG: print ray info if it hits this specific cube for debugging
    /*
    if (fabs(box.min.x - 2764.5f) < 1.0f) {
        printf("[RAYCAST DEBUG] Ray Origin: (%.2f, %.2f, %.2f) Dir: (%.3f, %.3f, %.3f)\n", 
               rayOrigin.x, rayOrigin.y, rayOrigin.z, dir.x, dir.y, dir.z);
        printf("[RAYCAST DEBUG] Cube Min: (%.1f, %.1f, %.1f) Max: (%.1f, %.1f, %.1f)\n", 
               box.min.x, box.min.y, box.min.z, box.max.x, box.max.y, box.max.z);
        printf("[RAYCAST DEBUG] tMin: %.2f, tMax: %.2f\n", tMin, tMax);
    }
    */
    
    return (tMax >= tMin) && (tMax >= 0.0f);
}

// Create AABB for a cube at position (x,y,z) with half-extents (sx/2, sy/2, sz/2)
static AABB createCubeAABB(float x, float y, float z, float sx, float sy, float sz) {
    float halfSx = sx * 0.5f;
    float halfSy = sy * 0.5f;
    float halfSz = sz * 0.5f;
    
    glm::vec3 min(x - halfSx, y - halfSy, z - halfSz);
    glm::vec3 max(x + halfSx, y + halfSy, z + halfSz);
    return AABB(min, max);
}

// Raycast from mouse position against a cube
// Returns 1 if hit, 0 if miss
extern "C" int heidic_raycast_cube_hit(GLFWwindow* window, float cubeX, float cubeY, float cubeZ, float cubeSx, float cubeSy, float cubeSz) {
    if (!window) return 0;
    
    // Get mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    // Get framebuffer size (more accurate than swapchain extent for mouse coordinates)
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    // Convert to NDC
    glm::vec2 ndc = screenToNDC((float)mouseX, (float)mouseY, fbWidth, fbHeight);
    
    // Unproject to get ray
    glm::mat4 invProj = glm::inverse(g_currentProj);
    glm::mat4 invView = glm::inverse(g_currentView);
    glm::vec3 rayOrigin, rayDir;
    unproject(ndc, invProj, invView, rayOrigin, rayDir);
    
    // Create AABB for cube
    AABB cubeBox = createCubeAABB(cubeX, cubeY, cubeZ, cubeSx, cubeSy, cubeSz);
    
    // Test intersection
    float tMin, tMax;
    bool hit = rayAABB(rayOrigin, rayDir, cubeBox, tMin, tMax);
    
    // DEBUG: Print raycast info when testing (with AABB bounds)
    /*
    printf("[RAYCAST DEBUG] Mouse: (%.1f, %.1f) | NDC: (%.3f, %.3f) | Ray Origin: (%.2f, %.2f, %.2f) | Ray Dir: (%.3f, %.3f, %.3f) | Cube: (%.1f, %.1f, %.1f) | AABB: min(%.1f,%.1f,%.1f) max(%.1f,%.1f,%.1f) | Hit: %s | tMin: %.2f, tMax: %.2f\n",
           mouseX, mouseY, ndc.x, ndc.y, 
           rayOrigin.x, rayOrigin.y, rayOrigin.z,
           rayDir.x, rayDir.y, rayDir.z,
           cubeX, cubeY, cubeZ,
           cubeBox.min.x, cubeBox.min.y, cubeBox.min.z,
           cubeBox.max.x, cubeBox.max.y, cubeBox.max.z,
           hit ? "YES" : "NO", tMin, tMax);
    */
    
    return hit ? 1 : 0;
}

// Get hit point from raycast (call after heidic_raycast_cube_hit returns 1)
// Returns world-space hit position
extern "C" Vec3 heidic_raycast_cube_hit_point(GLFWwindow* window, float cubeX, float cubeY, float cubeZ, float cubeSx, float cubeSy, float cubeSz) {
    Vec3 result = {0.0f, 0.0f, 0.0f};
    if (!window) return result;
    
    // Get mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    // Get framebuffer size
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    // Convert to NDC
    glm::vec2 ndc = screenToNDC((float)mouseX, (float)mouseY, fbWidth, fbHeight);
    
    // Unproject to get ray
    glm::mat4 invProj = glm::inverse(g_currentProj);
    glm::mat4 invView = glm::inverse(g_currentView);
    glm::vec3 rayOrigin, rayDir;
    unproject(ndc, invProj, invView, rayOrigin, rayDir);
    
    // Create AABB for cube
    AABB cubeBox = createCubeAABB(cubeX, cubeY, cubeZ, cubeSx, cubeSy, cubeSz);
    
    // Test intersection
    float tMin, tMax;
    if (rayAABB(rayOrigin, rayDir, cubeBox, tMin, tMax)) {
        // Calculate hit point (use tMin for closest intersection)
        glm::vec3 hit = rayOrigin + rayDir * tMin;
        result.x = hit.x;
        result.y = hit.y;
        result.z = hit.z;
    }
    
    return result;
}

// Get mouse ray origin in world space
extern "C" Vec3 heidic_get_mouse_ray_origin(GLFWwindow* window) {
    Vec3 result = {0.0f, 0.0f, 0.0f};
    if (!window) return result;
    
    // Get mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    // Get framebuffer size
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    // Convert to NDC
    glm::vec2 ndc = screenToNDC((float)mouseX, (float)mouseY, fbWidth, fbHeight);
    
    // Unproject to get ray
    glm::mat4 invProj = glm::inverse(g_currentProj);
    glm::mat4 invView = glm::inverse(g_currentView);
    glm::vec3 origin, dir;
    unproject(ndc, invProj, invView, origin, dir);
    
    result.x = origin.x;
    result.y = origin.y;
    result.z = origin.z;
    return result;
}

// Get mouse ray direction in world space
extern "C" Vec3 heidic_get_mouse_ray_dir(GLFWwindow* window) {
    Vec3 result = {0.0f, 0.0f, 0.0f};
    if (!window) return result;
    
    // Get mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    // Convert to NDC
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glm::vec2 ndc = screenToNDC((float)mouseX, (float)mouseY, fbWidth, fbHeight);
    
    // Unproject to get ray
    glm::mat4 invProj = glm::inverse(g_currentProj);
    glm::mat4 invView = glm::inverse(g_currentView);
    glm::vec3 origin, dir;
    unproject(ndc, invProj, invView, origin, dir);
    
    result.x = dir.x;
    result.y = dir.y;
    result.z = dir.z;
    return result;
}

// Draw a ground plane (large flat quad at specified y position)
extern "C" void heidic_draw_ground_plane(float size, float r, float g, float b) {
    // Draw a grid pattern on the ground plane
    // Ground plane is at y = -300 (3 meters below origin, since 1 unit = 1 cm)
    float groundY = -300.0f;
    float halfSize = size * 0.5f;
    int gridLines = 20; // Number of grid lines
    
    // Draw grid lines along X axis
    for (int i = 0; i <= gridLines; i++) {
        float z = -halfSize + (size / gridLines) * i;
        heidic_draw_line(-halfSize, groundY, z, halfSize, groundY, z, r, g, b);
    }
    
    // Draw grid lines along Z axis
    for (int i = 0; i <= gridLines; i++) {
        float x = -halfSize + (size / gridLines) * i;
        heidic_draw_line(x, groundY, -halfSize, x, groundY, halfSize, r, g, b);
    }
}

// Draw wireframe outline of a cube (12 edges)
extern "C" void heidic_draw_cube_wireframe(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz, float r, float g, float b) {
    // Build model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, z));
    model = glm::rotate(model, glm::radians(rx), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(ry), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rz), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Cube corners in local space (centered at origin, size 1x1x1)
    glm::vec3 corners[8] = {
        glm::vec3(-0.5f, -0.5f, -0.5f), // 0: left-bottom-back
        glm::vec3( 0.5f, -0.5f, -0.5f), // 1: right-bottom-back
        glm::vec3( 0.5f,  0.5f, -0.5f), // 2: right-top-back
        glm::vec3(-0.5f,  0.5f, -0.5f), // 3: left-top-back
        glm::vec3(-0.5f, -0.5f,  0.5f), // 4: left-bottom-front
        glm::vec3( 0.5f, -0.5f,  0.5f), // 5: right-bottom-front
        glm::vec3( 0.5f,  0.5f,  0.5f), // 6: right-top-front
        glm::vec3(-0.5f,  0.5f,  0.5f)  // 7: left-top-front
    };
    
    // Apply scale
    for (int i = 0; i < 8; i++) {
        corners[i] = glm::vec3(corners[i].x * sx, corners[i].y * sy, corners[i].z * sz);
    }
    
    // Transform to world space
    for (int i = 0; i < 8; i++) {
        glm::vec4 world = model * glm::vec4(corners[i], 1.0f);
        corners[i] = glm::vec3(world);
    }
    
    // Draw 12 edges of the cube
    // Back face (z = -0.5)
    heidic_draw_line(corners[0].x, corners[0].y, corners[0].z, corners[1].x, corners[1].y, corners[1].z, r, g, b); // bottom
    heidic_draw_line(corners[1].x, corners[1].y, corners[1].z, corners[2].x, corners[2].y, corners[2].z, r, g, b); // right
    heidic_draw_line(corners[2].x, corners[2].y, corners[2].z, corners[3].x, corners[3].y, corners[3].z, r, g, b); // top
    heidic_draw_line(corners[3].x, corners[3].y, corners[3].z, corners[0].x, corners[0].y, corners[0].z, r, g, b); // left
    
    // Front face (z = 0.5)
    heidic_draw_line(corners[4].x, corners[4].y, corners[4].z, corners[5].x, corners[5].y, corners[5].z, r, g, b); // bottom
    heidic_draw_line(corners[5].x, corners[5].y, corners[5].z, corners[6].x, corners[6].y, corners[6].z, r, g, b); // right
    heidic_draw_line(corners[6].x, corners[6].y, corners[6].z, corners[7].x, corners[7].y, corners[7].z, r, g, b); // top
    heidic_draw_line(corners[7].x, corners[7].y, corners[7].z, corners[4].x, corners[4].y, corners[4].z, r, g, b); // left
    
    // Connecting edges
    heidic_draw_line(corners[0].x, corners[0].y, corners[0].z, corners[4].x, corners[4].y, corners[4].z, r, g, b); // left-bottom
    heidic_draw_line(corners[1].x, corners[1].y, corners[1].z, corners[5].x, corners[5].y, corners[5].z, r, g, b); // right-bottom
    heidic_draw_line(corners[2].x, corners[2].y, corners[2].z, corners[6].x, corners[6].y, corners[6].z, r, g, b); // right-top
    heidic_draw_line(corners[3].x, corners[3].y, corners[3].z, corners[7].x, corners[7].y, corners[7].z, r, g, b); // left-top
}

// Ground detection: Raycast straight down from a position
// Returns 1 if ground is hit, 0 otherwise
extern "C" int heidic_raycast_ground_hit(float x, float y, float z, float maxDistance) {
    // Ray origin: at the position
    glm::vec3 rayOrigin(x, y, z);
    
    // Ray direction: straight down
    glm::vec3 rayDir(0.0f, -1.0f, 0.0f);
    
    // Test against a ground plane at y=-300 (3 meters below origin)
    float groundY = -300.0f;
    
    if (rayOrigin.y > groundY && rayOrigin.y - maxDistance <= groundY) {
        float t = rayOrigin.y - groundY; // Distance to ground (rayDir.y is -1)
        if (t >= 0.0f && t <= maxDistance) {
            return 1;
        }
    }
    
    return 0;
}

// Get ground hit point (call after heidic_raycast_ground_hit returns 1)
extern "C" Vec3 heidic_raycast_ground_hit_point(float x, float y, float z, float maxDistance) {
    Vec3 result = {x, -300.0f, z}; // Default to ground at y=-300 (3 meters below)
    
    glm::vec3 rayOrigin(x, y, z);
    glm::vec3 rayDir(0.0f, -1.0f, 0.0f);
    float groundY = -300.0f;
    
    if (rayOrigin.y > groundY && rayOrigin.y - maxDistance <= groundY) {
        float t = rayOrigin.y - groundY;
        if (t >= 0.0f && t <= maxDistance) {
            glm::vec3 hit = rayOrigin + rayDir * t;
            result.x = hit.x;
            result.y = hit.y;
            result.z = hit.z;
        }
    }
    
    return result;
}

// Debug: Print current mouse ray info to console
extern "C" void heidic_debug_print_ray(GLFWwindow* window) {
    if (!window) return;
    
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    glm::vec2 ndc = screenToNDC((float)mouseX, (float)mouseY, fbWidth, fbHeight);
    
    glm::mat4 invProj = glm::inverse(g_currentProj);
    glm::mat4 invView = glm::inverse(g_currentView);
    glm::vec3 rayOrigin, rayDir;
    unproject(ndc, invProj, invView, rayOrigin, rayDir);
    
    // Calculate a point 1000 units along the ray
    glm::vec3 rayEnd = rayOrigin + rayDir * 1000.0f;
    
    /*
    printf("=== RAYCAST DEBUG ===\n");
    printf("Mouse Screen: (%.1f, %.1f) | Framebuffer: (%d, %d)\n", mouseX, mouseY, fbWidth, fbHeight);
    printf("NDC: (%.4f, %.4f)\n", ndc.x, ndc.y);
    printf("Ray Origin: (%.2f, %.2f, %.2f)\n", rayOrigin.x, rayOrigin.y, rayOrigin.z);
    printf("Ray Direction: (%.4f, %.4f, %.4f) | Length: %.4f\n", rayDir.x, rayDir.y, rayDir.z, glm::length(rayDir));
    printf("Ray End (1000 units): (%.2f, %.2f, %.2f)\n", rayEnd.x, rayEnd.y, rayEnd.z);
    printf("Camera Pos: (%.2f, %.2f, %.2f)\n", g_currentCamPos.x, g_currentCamPos.y, g_currentCamPos.z);
    printf("====================\n");
    */
}

// Draw the mouse ray for visual debugging
extern "C" void heidic_draw_ray(GLFWwindow* window, float length, float r, float g, float b) {
    if (!window) return;
    
    // Get mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    // Get framebuffer size
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    
    // Convert to NDC
    glm::vec2 ndc = screenToNDC((float)mouseX, (float)mouseY, fbWidth, fbHeight);
    
    // Unproject to get ray
    glm::mat4 invProj = glm::inverse(g_currentProj);
    glm::mat4 invView = glm::inverse(g_currentView);
    glm::vec3 rayOrigin, rayDir;
    unproject(ndc, invProj, invView, rayOrigin, rayDir);
    
    // Calculate end point
    glm::vec3 rayEnd = rayOrigin + rayDir * length;
    
    // Draw the line
    // We can't access heidic_draw_line directly because it's extern "C" in this same file but not declared in a header this file includes in a way C++ likes for internal calls sometimes?
    // actually heidic_draw_line IS defined in this file. So we can just call it.
    heidic_draw_line(rayOrigin.x, rayOrigin.y, rayOrigin.z, rayEnd.x, rayEnd.y, rayEnd.z, r, g, b);
}
// Gizmo State
static int g_gizmoActiveAxis = 0; // 0=None, 1=X, 2=Y, 3=Z
static glm::vec3 g_gizmoInitialPos;
static float g_gizmoDragOffset = 0.0f; // Distance along axis from initial pos
static bool g_gizmoWasMouseDown = false;

// Helper: Distance between two lines
// Line 1: P1 + t1 * V1
// Line 2: P2 + t2 * V2
static float closestDistanceBetweenLines(glm::vec3 P1, glm::vec3 V1, glm::vec3 P2, glm::vec3 V2, float& t1, float& t2) {
    glm::vec3 P12 = P1 - P2;
    float d1343 = glm::dot(P12, V2);
    float d4321 = glm::dot(V2, V1);
    float d1321 = glm::dot(P12, V1);
    float d4343 = glm::dot(V2, V2);
    float d2121 = glm::dot(V1, V1);

    float denom = d2121 * d4343 - d4321 * d4321;
    float numer = d1343 * d4321 - d1321 * d4343;

    if (fabs(denom) < 1e-6f) {
        t1 = 0.0f;
        t2 = 0.0f; // Parallel
        return glm::length(P1 - P2); // Distance between origins roughly
    }

    t1 = numer / denom;
    t2 = (d1343 + d4321 * t1) / d4343;

    glm::vec3 Pa = P1 + t1 * V1;
    glm::vec3 Pb = P2 + t2 * V2;
    return glm::length(Pa - Pb);
}

// Draw translation gizmo and handle interaction
extern "C" Vec3 heidic_gizmo_translate(GLFWwindow* window, float x, float y, float z) {
    Vec3 result = {x, y, z};
    if (!window) return result;
    
    // Gizmo configuration
    float axisLen = 100.0f;
    float axisThick = 5.0f;
    
    // Mouse state
    bool mouseDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    bool justClicked = mouseDown && !g_gizmoWasMouseDown;
    
    // Get mouse ray
    Vec3 ro = heidic_get_mouse_ray_origin(window);
    Vec3 rd = heidic_get_mouse_ray_dir(window);
    glm::vec3 rayOrigin(ro.x, ro.y, ro.z);
    glm::vec3 rayDir(rd.x, rd.y, rd.z);
    glm::vec3 gizmoPos(x, y, z);
    
    // Axes directions
    glm::vec3 axes[] = {
        glm::vec3(1,0,0), // X
        glm::vec3(0,1,0), // Y
        glm::vec3(0,0,1)  // Z
    };
    
    // Colors
    float colors[3][3] = {
        {1,0,0}, {0,1,0}, {0,0,1} // R, G, B
    };
    
    // Hit testing (only if not dragging)
    int hoveredAxis = 0;
    
    // If not dragging, check for hover
    if (g_gizmoActiveAxis == 0) {
        float minDist = 1e9f;
        
        for (int i = 0; i < 3; i++) {
            // Calculate closest point on axis line to ray
            float tRay, tAxis;
            float dist = closestDistanceBetweenLines(rayOrigin, rayDir, gizmoPos, axes[i], tRay, tAxis);
            
            // Hit logic:
            // 1. Distance must be within "thickness" (cylindrical check)
            // 2. Point on axis must be positive (0 to axisLen)
            if (dist < axisThick * 2.0f && tAxis > 0.0f && tAxis < axisLen) {
                // Determine depth (distance along ray)
                if (tRay > 0.0f && tRay < minDist) {
                    minDist = tRay;
                    hoveredAxis = i + 1;
                }
            }
        }
    } else {
        hoveredAxis = g_gizmoActiveAxis;
    }
    
    // Drawing
    for (int i = 0; i < 3; i++) {
        float r = colors[i][0];
        float g = colors[i][1];
        float b = colors[i][2];
        
        // Highlight if hovered or active
        if (hoveredAxis == i + 1) {
            r = glm::min(r + 0.5f, 1.0f);
            g = glm::min(g + 0.5f, 1.0f);
            b = glm::min(b + 0.5f, 1.0f);
        }
        
        // Draw Axis (Wireframe Box)
        // Center position for the box
        float len = axisLen;
        float th = axisThick;
        
        float cx = x + axes[i].x * len * 0.5f;
        float cy = y + axes[i].y * len * 0.5f;
        float cz = z + axes[i].z * len * 0.5f;
        
        float sx = (i==0) ? len : th;
        float sy = (i==1) ? len : th;
        float sz = (i==2) ? len : th;
        
        heidic_draw_cube_wireframe(cx, cy, cz, 0, 0, 0, sx, sy, sz, r, g, b);
    }
    
    // Interaction Logic
    if (justClicked && hoveredAxis > 0) {
        g_gizmoActiveAxis = hoveredAxis;
        g_gizmoInitialPos = gizmoPos;
        
        // Calculate initial offset on axis
        float tRay, tAxis;
        closestDistanceBetweenLines(rayOrigin, rayDir, gizmoPos, axes[hoveredAxis-1], tRay, tAxis);
        g_gizmoDragOffset = tAxis;
    }
    else if (!mouseDown && g_gizmoActiveAxis > 0) {
        // Released
        g_gizmoActiveAxis = 0;
    }
    
    if (mouseDown && g_gizmoActiveAxis > 0) {
        // Dragging
        int axisIdx = g_gizmoActiveAxis - 1;
        float tRay, tAxis;
        // Project ray onto the line passing through INITIAL pos
        // Note: We use InitialPos as origin for the line to keep math stable
        closestDistanceBetweenLines(rayOrigin, rayDir, g_gizmoInitialPos, axes[axisIdx], tRay, tAxis);
        
        // Calculate movement delta
        // tAxis is the distance along the line from InitialPos
        // We want the new position to be such that the point under cursor (tAxis) matches the initial grab point (dragOffset)
        // Wait.
        // tAxis is "where on the line is closest to ray now".
        // g_gizmoDragOffset was "where on the line was closest to ray at start".
        // The shift is (tAxis - g_gizmoDragOffset).
        float delta = tAxis - g_gizmoDragOffset;
        
        result.x = g_gizmoInitialPos.x + axes[axisIdx].x * delta;
        result.y = g_gizmoInitialPos.y + axes[axisIdx].y * delta;
        result.z = g_gizmoInitialPos.z + axes[axisIdx].z * delta;
    }
    
    g_gizmoWasMouseDown = mouseDown;
    return result;
}

extern "C" int heidic_gizmo_is_interacting() {
    return (g_gizmoActiveAxis > 0);
}

// ============================================================================
// DYNAMIC CUBE STORAGE SYSTEM
// ============================================================================

struct CreatedCube {
    float x, y, z;
    float sx, sy, sz;  // size
    int active;  // 1 = exists, 0 = deleted
};

static std::vector<CreatedCube> g_createdCubes;

extern "C" int heidic_create_cube(float x, float y, float z, float sx, float sy, float sz) {
    CreatedCube cube;
    cube.x = x;
    cube.y = y;
    cube.z = z;
    cube.sx = sx;
    cube.sy = sy;
    cube.sz = sz;
    cube.active = 1;
    g_createdCubes.push_back(cube);
    return (int)(g_createdCubes.size() - 1);  // Return index
}

extern "C" int heidic_get_cube_count() {
    int count = 0;
    for (const auto& cube : g_createdCubes) {
        if (cube.active == 1) count++;
    }
    return count;
}

extern "C" int heidic_get_cube_total_count() {
    return (int)g_createdCubes.size();
}

extern "C" float heidic_get_cube_x(int index) {
    if (index < 0 || index >= (int)g_createdCubes.size()) return 0.0f;
    return g_createdCubes[index].x;
}

extern "C" float heidic_get_cube_y(int index) {
    if (index < 0 || index >= (int)g_createdCubes.size()) return 0.0f;
    return g_createdCubes[index].y;
}

extern "C" float heidic_get_cube_z(int index) {
    if (index < 0 || index >= (int)g_createdCubes.size()) return 0.0f;
    return g_createdCubes[index].z;
}

extern "C" float heidic_get_cube_sx(int index) {
    if (index < 0 || index >= (int)g_createdCubes.size()) return 200.0f;
    return g_createdCubes[index].sx;
}

extern "C" float heidic_get_cube_sy(int index) {
    if (index < 0 || index >= (int)g_createdCubes.size()) return 200.0f;
    return g_createdCubes[index].sy;
}

extern "C" float heidic_get_cube_sz(int index) {
    if (index < 0 || index >= (int)g_createdCubes.size()) return 200.0f;
    return g_createdCubes[index].sz;
}

extern "C" int heidic_get_cube_active(int index) {
    if (index < 0 || index >= (int)g_createdCubes.size()) return 0;
    return g_createdCubes[index].active;
}

extern "C" void heidic_set_cube_pos(int index, float x, float y, float z) {
    if (index < 0 || index >= (int)g_createdCubes.size()) return;
    g_createdCubes[index].x = x;
    g_createdCubes[index].y = y;
    g_createdCubes[index].z = z;
}

// Overload that accepts float index (for HEIDIC compatibility)
extern "C" void heidic_set_cube_pos_f(float index_f, float x, float y, float z) {
    int index = (int)index_f;
    if (index < 0 || index >= (int)g_createdCubes.size()) return;
    g_createdCubes[index].x = x;
    g_createdCubes[index].y = y;
    g_createdCubes[index].z = z;
}

extern "C" void heidic_delete_cube(int index) {
    if (index < 0 || index >= (int)g_createdCubes.size()) return;
    g_createdCubes[index].active = 0;
}

extern "C" int heidic_find_next_active_cube_index(int start_index) {
    // Find the next active cube starting from start_index
    for (int i = start_index; i < (int)g_createdCubes.size(); i++) {
        if (g_createdCubes[i].active == 1) {
            return i;
        }
    }
    return -1;  // No more active cubes
}

// Helper to convert i32 to f32 (for HEIDIC type system)
extern "C" float heidic_int_to_float(int value) {
    return (float)value;
}
