#pragma once
#include <typeindex>
namespace DDing {
	class MonoBehaviour;
	class GameObject
	{
	public:
		GameObject();
		~GameObject();

		void Update();
		
		template<typename T, typename... Args>
		T* AddComponent(Args&&...args) {
			std::type_index index(typeid(T));
			auto component = std::make_unique<T>(std::forward<Args>(args)...);
			T* ptr = component.get();
			component->SetGameObject(this);
			components[index] = std::move(component);
			return ptr;
		}

		template<typename T>
		T* GetComponent() {
			std::type_index index(typeid(T));
			auto it = components.find(index);
			if (it != components.end()) {
				return dynamic_cast<T*>(it->second.get());
			}
			return nullptr;
		}

		void Draw(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);
		auto& GetComponents() { return components; }
		
		bool isActive = true;
		std::string name = "UNINITIALIZED";
	protected:
		std::unordered_map<std::type_index, std::unique_ptr<Component>> components;
		std::vector <std::unique_ptr<MonoBehaviour>> scripts;
	};
}

