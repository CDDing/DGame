#pragma once
#include "RenderPass.h"
namespace DDing
{
	class DeferredPass :public RenderPass
	{
	public:
		DeferredPass();
		virtual void Render(vk::CommandBuffer commandBuffer);
	};
};

