#pragma once
namespace DDing {

	class Transform : public Component
	{
	public:
		Transform();
		~Transform();

		glm::vec3 GetLocalScale() const { return localScale; }
		void SetLocalScale(const glm::vec3& localScale) { this->localScale = localScale; MakeDirty(); }
		glm::quat GetLocalRotation() const { return localRotation; }
		void SetLocalRotation(const glm::quat& localRotation) { this->localRotation = localRotation; MakeDirty(); }
		glm::vec3 GetLocalPosition() const { return localPosition; }
		void SetLocalPosition(const glm::vec3& localPosition) { this->localPosition = localPosition; MakeDirty(); }
		glm::mat4 GetLocalMatrix() const { return cachedLocalMatrix; }
		
		
		auto GetWorldPosition() {
			if (isDirty)
				RecalculateMatrices();
			return worldPosition;
		}

		glm::vec3 GetWorldEulerAngle() {
			if (isDirty)
				RecalculateMatrices();
			return glm::eulerAngles(worldRotation);
		}

		auto GetWorldRotation() {
			if (isDirty)
				RecalculateMatrices();
			return worldRotation;
		}
		auto GetWorldScale() {
			if (isDirty)
				RecalculateMatrices();
			return worldScale;
		}
		
		glm::mat4 GetWorldMatrix() { 
			if (isDirty)
				RecalculateMatrices();
			return cachedWorldMatrix; 
		}

		glm::vec3 GetRight() { 
			if (isDirty)
				RecalculateMatrices();
			return glm::normalize(worldRotation * glm::vec3(-1, 0, 0)); }
		glm::vec3 GetUp() { 
			if (isDirty)
				RecalculateMatrices();
			return glm::normalize(worldRotation * glm::vec3(0, 1, 0)); }
		glm::vec3 GetLook() { 
			if (isDirty)
				RecalculateMatrices();
			return glm::normalize(worldRotation * glm::vec3(0,0,-1)); }

		virtual void Update() override;

		bool HasParent() { return parent != nullptr; }
		/*void SetParent(Transform* parent) { 
			this->parent = parent; 
		}*/
		void AddChild(Transform* child) { 
			children.push_back(child);
			child->parent = this;
		}
		void DrawUI() override;

		void RemoveChild(Transform* child) {
			children.erase(std::remove(children.begin(), children.end(), child), children.end());
			child->parent = nullptr;
		}
		Transform* GetParent() { return parent; }
		const std::vector<Transform*>& GetChildren() { return children; }
	private:
		bool DecomposeMatrix(const glm::mat4& matrix, glm::vec3& position, glm::quat& rotation, glm::vec3& scale);

		glm::vec3 localPosition = glm::vec3(0.0f);
		glm::quat localRotation = { 1.0f,0.0f,0.0f,0.0f };
		glm::vec3 localEulerAngle = {};
		glm::vec3 localScale = glm::vec3(1.0f);

		glm::vec3 worldPosition = {};
		glm::quat worldRotation = {};
		glm::vec3 worldScale = {};

		glm::mat4 cachedLocalMatrix = { 1.0f };
		glm::mat4 cachedWorldMatrix = { 1.0f };
		bool isDirty = true;

		void MakeDirty();
		void RecalculateMatrices();



		Transform* parent = nullptr;
		std::vector<Transform*> children;
	};
}
