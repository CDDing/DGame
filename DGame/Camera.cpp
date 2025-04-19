#include "pch.h"
#include "Camera.h"
#include "Transform.h"
glm::mat4 DDing::Camera::View = glm::mat4();
glm::mat4 DDing::Camera::Projection = glm::mat4();
void DDing::Camera::Update()
{
	UpdateMatrix();
}

void DDing::Camera::UpdateMatrix()
{
	auto transform = gameObject->GetComponent<DDing::Transform>();
	auto eyePosition = transform->GetPosition();
	auto focusDirection = eyePosition + transform->GetLook();
	auto upDirection = transform->GetUp();

	View = glm::lookAtLH(eyePosition, focusDirection, upDirection);

	//if needed, Orthogonal add
	Projection = glm::perspectiveFovLH(glm::radians(70.0f), static_cast<float>(DGame->swapChain.extent.width), static_cast<float>(DGame->swapChain.extent.height), 0.1f, 100.0f);
}
