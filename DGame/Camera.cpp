#include "pch.h"
#include "Camera.h"
#include "Transform.h"
glm::mat4 DDing::Camera::View = glm::mat4();
glm::mat4 DDing::Camera::Projection = glm::mat4();
void DDing::Camera::Update()
{
	UpdatePosition();
	UpdateMatrix();
	UploadToStaging();
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

void DDing::Camera::UploadToStaging()
{
	FrameData& frame = DGame->render.frameDatas[DGame->render.currentFrame];
	auto eyePosition = gameObject->GetComponent<DDing::Transform>()->GetPosition();
	void* mappedData = frame.globalStagingBuffer.GetMappedPtr();
	memcpy((char*)mappedData + offsetof(GlobalBuffer, view), &View, sizeof(glm::mat4));
	memcpy((char*)mappedData + offsetof(GlobalBuffer, projection), &Projection, sizeof(glm::mat4));
	memcpy((char*)mappedData + offsetof(GlobalBuffer, cameraPosition), &eyePosition, sizeof(glm::vec3));
	
}

void DDing::Camera::UpdatePosition()
{
	bool* keyPressed = DGame->input.keyPressed;
	auto transform = gameObject->GetComponent<DDing::Transform>();
	auto forward = glm::normalize(transform->GetLook());
	auto right = glm::normalize(transform->GetRight());
	auto up = glm::normalize(transform->GetUp());
	glm::vec3 move(0.0f);
	
	if (keyPressed[GLFW_KEY_W]) move += forward;
	if (keyPressed[GLFW_KEY_S]) move -= forward;
	if (keyPressed[GLFW_KEY_A]) move -= right;
	if (keyPressed[GLFW_KEY_D]) move += right;
	if (keyPressed[GLFW_KEY_E]) move += up;
	if (keyPressed[GLFW_KEY_Q]) move -= up;

	if (glm::length(move) > 0.0f) {
		auto movedPosition = transform->GetPosition() + glm::normalize(move) * 0.01f;
		transform->SetPosition(movedPosition);
	}
}
