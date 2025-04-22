#include "pch.h"
#include "Game.h"
std::unique_ptr<Game> DGame = std::make_unique<Game>();
Game::Game()
    : window(initWindow()), context(window), swapChain(context)
{

}

Game::~Game()
{
}


void Game::Init()
{
    render.Init();
    input.Init();

    //TODO integrate with GUI when load GLTF
    resource.Init();

    auto tempScene = std::make_unique<DDing::Scene>();
    tempScene->LoadSceneFromGLTF(resource.gltfs[0]);
    tempScene->gltfDescriptor = *resource.gltfs[0].descriptorSet;
    scene.currentScene = tempScene.get();
    scene.AddScene(std::move(tempScene));
    
}

void Game::Run()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        Update();
        Render();
    }
    context.logical.waitIdle();

    glfwDestroyWindow(window);

    glfwTerminate();
}

void Game::Update()
{
    input.Update();
    scene.Update();
}

void Game::Render()
{
    render.DrawFrame(scene.scenes[0].get(), DDing::PassType::Default);
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
