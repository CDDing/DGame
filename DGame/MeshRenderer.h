#pragma once
#include "Component.h"
namespace DDing {
	class MeshRenderer : public Component
	{
	public:

		void SetMesh(Mesh* mesh) { this->mesh = mesh; }
		auto GetMesh() { return mesh; }
	private:

		Mesh* mesh;
	};
}

