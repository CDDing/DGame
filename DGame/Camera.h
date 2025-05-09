#pragma once
#include "Component.h"
namespace DDing {
	class Camera : public Component
	{
	public:
		static glm::mat4 View;
		static glm::mat4 Projection;
		
		virtual void Update() override;

		void DrawUI() override;
	private:
		void UpdateMatrix();
		void UpdatePosition();
		void UpdateTransform();
		void UpdateRotation();
	};
}

