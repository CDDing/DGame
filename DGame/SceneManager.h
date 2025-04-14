#pragma once
//#include "Scene.h"
class SceneManager
{
public:
	SceneManager(DDing::Context& context);
private:

	//Scene activeScene; //TODO : convert to Vector if add scenes.
	DDing::Context* context;

};

