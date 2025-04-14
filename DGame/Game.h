#pragma once
class SceneManager;
class ResourceManager;
#include "Context.h"
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
};

