#pragma once

#include "Engine.hpp"
#include "Buffer.hpp"
#include "Camera.hpp"
#include "GameObject.hpp"
#include "KeyboardController.hpp"
#include "Model.hpp"
#include "RenderSystem.hpp"
#include "Texture.hpp"
#include "Util.hpp"

#include "Chip8.hpp"

#include <array>
#include <memory>
#include <vector>

class Chip8App {
public:
    Chip8App();
    ~Chip8App() = default;

    void run();

private:
    void loadPlane();
    void updateAspect(float aspect);
    void pollChip8Keys();

    MAGE::Engine m_engine;
    Chip8 m_chip8;

    std::vector<MAGE::GameObject> m_gameObjects;
    std::unique_ptr<MAGE::Texture> m_displayTexture;
    std::array<uint8_t, Chip8::VIDEO_WIDTH * Chip8::VIDEO_HEIGHT * 4> m_pixelBuffer {};

    MAGE::Camera m_camera {};
    float m_top {}, m_bottom {}, m_left {}, m_right {};
};