#pragma once
#include "Scene.h"
class SceneManager
{
public:
	SceneManager() {};
	DDing::Scene& GetCurrentScene() { return scene; }
private:
	DDing::Scene scene;
	

};

