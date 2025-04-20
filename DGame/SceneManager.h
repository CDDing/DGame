#pragma once
#include "Scene.h"
class SceneManager
{
public:
	SceneManager() {};

	void AddScene(std::unique_ptr<DDing::Scene> scene) { scenes.push_back(std::move(scene)); }

	//TODO add Getter and move this private
	std::vector<std::unique_ptr<DDing::Scene>> scenes;
private:
	

};

