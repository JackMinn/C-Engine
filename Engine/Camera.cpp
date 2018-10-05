#include "stdafx.h"
#include "Camera.h"
#include "Input.h"

Camera::Camera(const uint32_t& screenWidth, const uint32_t& screenHeight) : m_Transform(Transform(DirectX::SimpleMath::Vector3(0.0f, 5.0f, -20.0f), DirectX::SimpleMath::Quaternion::Identity,
													DirectX::SimpleMath::Vector3::One)), m_ClearColor { 0.0f, 0.0f, 0.0f, 1.0f }
{
	DebugLog("s", "Camera constructor");
	m_EulerAngles = m_Transform.GetEulerAngles();

	m_NearClip = 0.3f;
	m_FarClip = 1000.0f;
	m_FieldOfView = DirectX::XM_2PI / 6.0f;
	m_ScreenAspect = (float)screenWidth / (float)screenHeight;
	m_ViewPort.Width = (float)screenWidth;
	m_ViewPort.Height = (float)screenHeight;
	m_ViewPort.MinDepth = 0.0f;
	m_ViewPort.MaxDepth = 1.0f;
	m_ViewPort.TopLeftX = 0.0f;
	m_ViewPort.TopLeftY = 0.0f;

	Camera::BuildProjectionMatrix();
	Camera::BuildViewMatrix();
}

Camera::Camera(const Camera&)
{
}

Camera::~Camera() 
{
}

void Camera::Update() 
{
	using namespace DirectX::SimpleMath;

	const float moveSpeed = (1.0f / 60.0f);
	const float rotateSpeed = 0.1f;

	Vector3 movementVector = Vector3{ 0.0f, 0.0f, 0.0f };

	if (Input::IsKeyDown(VK_UP)) {
		movementVector.z += moveSpeed;
	}
	if (Input::IsKeyDown(VK_DOWN)) {
		movementVector.z -= moveSpeed;
	}
	if (Input::IsKeyDown(VK_RIGHT)) {
		movementVector.x += moveSpeed;
	}
	if (Input::IsKeyDown(VK_LEFT)) {
		movementVector.x -= moveSpeed;
	}

	m_EulerAngles.x += Input::GetMouseDelta().y * rotateSpeed;
	m_EulerAngles.y += Input::GetMouseDelta().x * rotateSpeed;
	m_Transform.SetRotation(Quaternion::CreateFromPitchYawRoll(m_EulerAngles.x, m_EulerAngles.y, 0.0f));

	Vector3 deltaPosition = Vector3::Transform(movementVector, m_Transform.GetRotation());
	m_Transform.SetPosition(deltaPosition + m_Transform.GetPosition());
}

void Camera::BuildProjectionMatrix()
{
	using namespace DirectX::SimpleMath;

	m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_FieldOfView, m_ScreenAspect, m_NearClip, m_FarClip);
	m_InvProjectionMatrix = m_ProjectionMatrix.Invert();
	m_ProjectionMatrixGPU = DirectX::XMMatrixTranspose(m_ProjectionMatrix);
	m_InvProjectionMatrixGPU = DirectX::XMMatrixTranspose(m_InvProjectionMatrix);
}

void Camera::BuildViewMatrix()
{
	using namespace DirectX::SimpleMath;

	Vector3 lookAtPoint = { 0, 0, 1 };
	Vector3 upVector = { 0, 1, 0 };

	//rotate the point and vector by the camera's rotation quaternion
	//lookAtPoint = DirectX::XMVector3Rotate(lookAtPoint, m_Rotation);
	//upVector = DirectX::XMVector3Rotate(upVector, m_Rotation);
	lookAtPoint = DirectX::XMVector3Rotate(lookAtPoint, m_Transform.GetRotation());
	upVector = DirectX::XMVector3Rotate(upVector, m_Transform.GetRotation());


	//translate the look at point so that it is relative to the cameras actual position
	//lookAtPoint = DirectX::XMVectorAdd(m_Position, lookAtPoint);
	lookAtPoint = DirectX::XMVectorAdd(m_Transform.GetPosition(), lookAtPoint);

	//create a left handed look at matrix
	//m_ViewMatrix = Matrix::CreateLookAt(m_Position, lookAtPoint, upVector);
	m_ViewMatrix = Matrix::CreateLookAt(m_Transform.GetPosition(), lookAtPoint, upVector);
	m_ViewMatrixGPU = DirectX::XMMatrixTranspose(m_ViewMatrix);
}
