#pragma once
#include "Context.h"
#include "SwapChain.h"
#include "RenderManager.h"
class Game
{
public:
	Game();
	~Game();

	void Run();
	void Update();
	void Render();


private:
	GLFWwindow* initWindow();
	GLFWwindow* window;
	DDing::Context context;
	DDing::SwapChain swapChain;
	RenderManager render;
};

