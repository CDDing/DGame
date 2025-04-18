#pragma once
#include "Context.h"
#include "SwapChain.h"
#include "RenderManager.h"
#include "SceneManager.h"
class Game
{
private:
	GLFWwindow* window;
public:
	Game();
	~Game();

	void Init();
	void Run();
	void Update();
	void Render();

	DDing::Context context;
	DDing::SwapChain swapChain;
	RenderManager render;
	SceneManager scene;
private:
	GLFWwindow* initWindow();
};
extern std::unique_ptr<Game> DGame;