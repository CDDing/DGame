#pragma once
namespace DDing {

	class Transform : public Component
	{
	public:
		Transform();
		~Transform();

		glm::vec3 GetScale() { return scale; }
		void SetScale(const glm::vec3& worldScale) { scale = worldScale; }
		glm::quat GetRotation() { return rotation; }
		void SetRotation(const glm::quat& worldRotation) { rotation = worldRotation; }
		glm::vec3 GetPosition() { return position; }
		void SetPosition(const glm::vec3& worldPosition) { position = worldPosition; }
		glm::mat4 GetTransformMatrix() { return localTransform; }

		glm::vec3 GetRight() { return glm::normalize(worldMatrix[0]); }
		glm::vec3 GetUp() { return glm::normalize(worldMatrix[1]); }
		glm::vec3 GetLook() { return glm::normalize(worldMatrix[2]); }

		virtual void Update() override;

		bool HasParent() { return parent != nullptr; }
		/*void SetParent(Transform* parent) { 
			this->parent = parent; 
		}*/
		void AddChild(Transform* child) { 
			children.push_back(child);
			child->parent = this;
		}
		Transform* GetParent() { return parent; }
		const std::vector<Transform*>& GetChildren() { return children; }
	private:

		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;

		glm::mat4 worldMatrix;

		void UpdateLocalTransform();
		glm::mat4 localTransform;

		Transform* parent = nullptr;
		std::vector<Transform*> children;
	};
}
