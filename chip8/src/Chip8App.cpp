#include "Chip8App.hpp"

#include <chrono>
#include <string>

Chip8App::Chip8App() {
    loadPlane();
    m_chip8.LoadROM(std::string(CHIP8_ROM_DIR) + "/test_opcode.ch8");

    for (size_t i = 0; i < Chip8::VIDEO_WIDTH * Chip8::VIDEO_HEIGHT; i++) {
        m_pixelBuffer[i * 4 + 3] = 255;
    }
}

void Chip8App::loadPlane() {
    MAGE::Builder planeBuilder {};
    planeBuilder.vertices = {
        {{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{-1.0f,  1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}}
    };

    planeBuilder.indices = {0, 1, 2, 0, 2, 3};

    std::shared_ptr<MAGE::Model> planeModel = std::make_shared<MAGE::Model>(m_engine.getDevice(), planeBuilder);
    MAGE::GameObject plane = MAGE::GameObject::createGameObject();
    plane.m_model = planeModel;
    m_gameObjects.push_back(std::move(plane));
}

void Chip8App::updateAspect(float aspect) {
    constexpr float chip8Aspect = static_cast<float>(Chip8::VIDEO_WIDTH) / static_cast<float>(Chip8::VIDEO_HEIGHT);

    if (aspect > chip8Aspect) { 
        m_left = -aspect / chip8Aspect;
        m_right = aspect / chip8Aspect;

        m_top = -1.0f;
        m_bottom = 1.0f;
    }
    else {
        m_left = -1.0f;
        m_right = 1.0f;

        m_top = -chip8Aspect / aspect;
        m_bottom = chip8Aspect / aspect;
    }

    m_camera.setOrtohraphicProjection(m_left, m_right, m_top, m_bottom, -1.0f, 1.0f);
}

void Chip8App::pollChip8Keys() {
    GLFWwindow* window = m_engine.getWindow().getGLFWWindow();

    constexpr std::pair<int, uint8_t> keymap[] = {
        {GLFW_KEY_1, 0x1}, {GLFW_KEY_2, 0x2}, {GLFW_KEY_3, 0x3}, {GLFW_KEY_4, 0xC},
        {GLFW_KEY_Q, 0x4}, {GLFW_KEY_W, 0x5}, {GLFW_KEY_E, 0x6}, {GLFW_KEY_R, 0xD},
        {GLFW_KEY_A, 0x7}, {GLFW_KEY_S, 0x8}, {GLFW_KEY_D, 0x9}, {GLFW_KEY_F, 0xE},
        {GLFW_KEY_Z, 0xA}, {GLFW_KEY_X, 0x0}, {GLFW_KEY_C, 0xB}, {GLFW_KEY_V, 0xF},
    };

    for (auto [glfwKey, chip8Key] : keymap) {
        m_chip8.m_keypad[chip8Key] = glfwGetKey(window, glfwKey) == GLFW_PRESS ? 1 : 0;
    }
}

void Chip8App::run() {
    // UBO buffers
    std::vector<std::unique_ptr<MAGE::Buffer>> uboBuffers(MAGE::SwapChain::MAX_FRAMES_IN_FLIGHT);
    
    for (size_t i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<MAGE::Buffer>(
            m_engine.getDevice(),
            sizeof(MAGE::GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_engine.getDevice().properties.limits.minUniformBufferOffsetAlignment);
        uboBuffers[i]->map();
    }

    // Global + texture descriptor set layouts
    auto globalSetLayout = MAGE::DescriptorSetLayout::Builder(m_engine.getDevice())
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .build();

    auto textureSetLayout = MAGE::DescriptorSetLayout::Builder(m_engine.getDevice())
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    std::vector<VkDescriptorSet> globalDescriptorSets(MAGE::SwapChain::MAX_FRAMES_IN_FLIGHT);
    
    for (size_t i = 0; i < globalDescriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();
        MAGE::DescriptorWriter(*globalSetLayout, *m_engine.getGlobalPool())
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    // Chip8 display texture + descriptor
    m_displayTexture = std::make_unique<MAGE::Texture>(m_engine.getDevice(), m_pixelBuffer.data(), Chip8::VIDEO_WIDTH, Chip8::VIDEO_HEIGHT);
    VkDescriptorImageInfo displayInfo = m_displayTexture->descriptorInfo();
    VkDescriptorSet displayDescriptorSet;

    MAGE::DescriptorWriter(*textureSetLayout, *m_engine.getGlobalPool())
        .writeImage(0, &displayInfo)
        .build(displayDescriptorSet);
    m_gameObjects[0].m_textureDescriptorSet = displayDescriptorSet;

    MAGE::RenderSystem renderSystem{
        m_engine.getDevice(), m_engine.getRenderer().getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout()};

    float aspect = m_engine.getRenderer().getAspectRatio();
    updateAspect(aspect);

    MAGE::KeyboardController cameraController{};
    MAGE::GameObject viewObject = MAGE::GameObject::createGameObject();

    float cycleAccumulator = 0.0f, timerAccumulator = 0.0f;
    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!m_engine.getWindow().shouldClose()) {
        glfwPollEvents();
        auto newTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        dt = std::min(dt, 0.1f);
        currentTime = newTime;

        if (aspect != m_engine.getRenderer().getAspectRatio()) {
            aspect = m_engine.getRenderer().getAspectRatio();
            updateAspect(aspect);
        }

        pollChip8Keys();

        cycleAccumulator += dt;
        while (cycleAccumulator >= m_chip8.CYCLE_PERIOD) {
            m_chip8.Cycle();
            cycleAccumulator -= m_chip8.CYCLE_PERIOD;
        }

        timerAccumulator += dt;
        while (timerAccumulator >= m_chip8.TIMER_PERIOD) {
            m_chip8.TickTimers();
            timerAccumulator -= m_chip8.TIMER_PERIOD;
        }

        m_chip8.getPixelsRGBA(m_pixelBuffer.data());
        m_displayTexture->update(m_pixelBuffer.data());

        cameraController.otherKeys(m_engine.getWindow().getGLFWWindow(), dt, viewObject);

        if (VkCommandBuffer cb = m_engine.getRenderer().beginFrame())
        {
            int frameIndex = m_engine.getRenderer().getFrameIndex();
            MAGE::FrameInfo frame{frameIndex, dt, cb, m_camera, globalDescriptorSets[frameIndex]};

            MAGE::GlobalUbo ubo{};
            ubo.projectionView = m_camera.getProjection() * m_camera.getView();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);

            m_engine.getRenderer().beginSwapChainRenderPass(cb);
            renderSystem.renderGameObject(frame, m_gameObjects);
            m_engine.getRenderer().endSwapChainRenderPass(cb);
            m_engine.getRenderer().endFrame();
        }
    }

    vkDeviceWaitIdle(m_engine.getDevice().getDevice());
}