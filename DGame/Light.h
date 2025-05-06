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
		LightType type;

		glm::vec3 color = glm::vec3(1.0f);

		void DrawUI() override;
		void Update() override;
		float intensity = 1.0f;
	};
}

