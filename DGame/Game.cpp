#include "pch.h"
#include "Game.h"

Game::Game()
    : window(initWindow()), context(window), swapChain(context), render(context,swapChain)
{

}

Game::~Game()
{
}


void Game::Run()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        Update();
        Render();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
}

void Game::Update()
{
}

void Game::Render()
{
    //render.DrawFrame();
}

GLFWwindow* Game::initWindow()
{
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Don't create OpenGL context
    GLFWwindow* initializedWindow = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);
    if (!initializedWindow) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    return initializedWindow;
}
