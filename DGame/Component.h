#pragma once

namespace DDing {
	class GameObject;
	class Component
	{
	public:
		Component();
		virtual ~Component();

		GameObject* GetGameObject() { return gameObject; }

		virtual void Update() {};
	private:
		friend class GameObject;
		void SetGameObject(GameObject* gameObject) { this->gameObject = gameObject; }

	protected:
		GameObject* gameObject;
	};
}

