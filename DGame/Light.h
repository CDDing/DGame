#pragma once
enum class LightType {
	eDirectional,
	ePoint,
	eSpot,
};
namespace DDing {
	class Light : public Component
	{
	public:
		LightType type = LightType::eDirectional;

		glm::vec3 color = glm::vec3(1.0f);

		void DrawUI() override;
		void Update() override;
		float intensity = 1.0f;

		float innerCone = 15.0f, outerCone = 25.0f;
	};
}

