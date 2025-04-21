#pragma once
namespace DDing {

	class Transform : public Component
	{
	public:
		Transform();
		~Transform();

		glm::vec3 GetScale() { return scale; }
		void SetScale(const glm::vec3& worldScale) { scale = worldScale; UpdateLocalTransform(); }
		glm::quat GetRotation() { return rotation; }
		void SetRotation(const glm::quat& worldRotation) { rotation = worldRotation; UpdateLocalTransform(); }
		glm::vec3 GetPosition() { return position; }
		void SetPosition(const glm::vec3& worldPosition) { position = worldPosition; UpdateLocalTransform(); }
		glm::mat4 GetTransformMatrix() { return localTransform; }

		glm::vec3 GetRight() { return glm::normalize(rotation * glm::vec3(-1, 0, 0)); }
		glm::vec3 GetUp() { return glm::normalize(rotation * glm::vec3(0, 1, 0)); }
		glm::vec3 GetLook() { return glm::normalize(rotation * glm::vec3(0,0,-1)); }

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


		void UpdateLocalTransform();
		glm::mat4 localTransform;

		Transform* parent = nullptr;
		std::vector<Transform*> children;
	};
}
