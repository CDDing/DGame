#include "pch.h"
#include "Camera.h"
#include "Transform.h"
glm::mat4 DDing::Camera::View = glm::mat4();
glm::mat4 DDing::Camera::Projection = glm::mat4();
void DDing::Camera::Update()
{
	UpdateTransform();
	UpdateMatrix();
	UploadToStaging();
}

void DDing::Camera::UpdateMatrix()
{
	auto transform = gameObject->GetComponent<DDing::Transform>();
	auto eyePosition = transform->GetWorldPosition();
	auto focusDirection = eyePosition + transform->GetLook();
	auto upDirection = transform->GetUp();

	View = glm::lookAtLH(eyePosition, focusDirection, upDirection);

	//if needed, Orthogonal add
	Projection = glm::perspectiveFovLH(glm::radians(70.0f), static_cast<float>(DGame->swapChain.extent.width), static_cast<float>(DGame->swapChain.extent.height), 0.1f, 10000.0f);

	Projection[1][1] *= -1;
}

void DDing::Camera::UploadToStaging()
{
	FrameData& frame = DGame->render.frameDatas[DGame->render.currentFrame];
	auto eyePosition = gameObject->GetComponent<DDing::Transform>()->GetWorldPosition();
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
		auto movedPosition = transform->GetLocalPosition() + glm::normalize(move) * 0.1f;
		transform->SetLocalPosition(movedPosition);
	}
}

void DDing::Camera::UpdateTransform()
{
	UpdatePosition();
	UpdateRotation();
}

void DDing::Camera::UpdateRotation()
{
	static bool wasRightMouseDown = false;
	static glm::vec2 lastMouse = { InputManager::mouseX, InputManager::mouseY };

	bool rightMouseDown = InputManager::mouseButtons[GLFW_MOUSE_BUTTON_RIGHT];

	if (!rightMouseDown)
	{
		wasRightMouseDown = false;
		return;
	}

	glm::vec2 currentMouse = { InputManager::mouseX, InputManager::mouseY };

	if (!wasRightMouseDown)
	{
		// 첫 클릭 시점에서 lastMouse 초기화
		lastMouse = currentMouse;
		wasRightMouseDown = true;
		return;
	}

	glm::vec2 delta = currentMouse - lastMouse;
	float sensitivity = 0.1f;
	float yawDelta = delta.x * sensitivity;
	float pitchDelta = -delta.y * sensitivity;

	auto transform = gameObject->GetComponent<DDing::Transform>();
	glm::quat quat = transform->GetLocalRotation();
	
	glm::quat pitchQuat = glm::angleAxis(glm::radians(pitchDelta), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::quat yawQuat = glm::angleAxis(glm::radians(yawDelta), glm::vec3(0.0f, 1.0f, 0.0f));

	quat = yawQuat * quat;
	quat = quat * pitchQuat;

	transform->SetLocalRotation(glm::quat(quat));

	lastMouse = currentMouse;
}
