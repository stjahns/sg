#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

using namespace glm;

class Camera
{
public:

	Camera() 
		: m_bMouseDrag(false)
		, m_MouseStartX(0.0)
		, m_MouseStartY(0.0)
		, m_Position(0.0f, 0.0f, 20.0f)
		, fCameraSpeed(1.0f)
		, fCameraRotateSpeed(1.0f)
	{
	}

	void OnMouseEvent(GLFWwindow* window, int button, int action, int mods)
	{
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT)
			{
				if (action == GLFW_PRESS)
				{
					glfwGetCursorPos(window, &m_MouseStartX, &m_MouseStartY);
					m_bMouseDrag = true;
					m_RotationStart = m_EulerRotation;
				}
				else if (action == GLFW_RELEASE)
				{
					m_bMouseDrag = false;
				}
			}
		}
	}

	void Widgets()
	{
		if (ImGui::CollapsingHeader("Camera"))
		{

			ImGui::DragFloat3("Position", (float *)&m_Position, 1.0, -10000, 10000);
			ImGui::DragFloat3("Rotation", (float *)&m_EulerRotation, 0.01, -3.14, 3.14);
			ImGui::DragFloat("Move Speed", &fCameraSpeed, 0.1, 0.0, 100.0);
			ImGui::DragFloat("Rotate Speed", &fCameraRotateSpeed, 0.1, 0.0, 100.0);
		}
	}

	// TODO -- use glfwSetScrollCallback(window, scroll_callback) on OSX
	// TODO use dT
	void Update(GLFWwindow* window)
	{

		const float fRotateRateScale = 0.003f;
		const float fTranslateRateScale = 0.500f;

		if (m_bMouseDrag)
		{

			double dMouseX;
			double dMouseY;
			glfwGetCursorPos(window, &dMouseX, &dMouseY);
			vec3 cameraDelta(m_MouseStartY - dMouseY, m_MouseStartX - dMouseX, 0.0f);
			m_EulerRotation = m_RotationStart + cameraDelta * fCameraRotateSpeed * fRotateRateScale;
		}

		mat4 camera = GetMatrix();

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			vec3 forward(camera[2]);
			m_Position -= forward * fCameraSpeed * fTranslateRateScale;
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			vec3 forward(camera[2]);
			m_Position += forward * fCameraSpeed * fTranslateRateScale;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			vec3 right(camera[0]);
			m_Position -= right * fCameraSpeed * fTranslateRateScale;
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			vec3 right(camera[0]);
			m_Position += right * fCameraSpeed * fTranslateRateScale;
		}

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			vec3 up(camera[1]);
			m_Position -= up * fCameraSpeed * fTranslateRateScale;
		}

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			vec3 up(camera[1]);
			m_Position += up * fCameraSpeed * fTranslateRateScale;
		}

	}

	mat4 GetMatrix()
	{
		mat4 m(1.0);
		m = translate(m, m_Position);
		m = rotate(m, m_EulerRotation.y, vec3(0.0f, 1.0f, 0.0f));
		m = rotate(m, m_EulerRotation.x, vec3(1.0f, 0.0f, 0.0f));
		m = rotate(m, m_EulerRotation.z, vec3(0.0f, 0.0f, 1.0f));
		return m;
	}

	mat4 GetViewMatrix()
	{
		// FIXME -- probably not the best way to do this
		return inverse(GetMatrix());
	}

	void SetPosition(vec3 p)
	{
		m_Position = p;
	}

	vec3 GetPosition()
	{
		return m_Position;
	}

	mat4 GetProjection()
	{
		return m_Projection;
	}

	void SetProjection(mat4 projection)
	{
		m_Projection = projection;
	}

private:

	vec3 m_Position;
	vec3 m_EulerRotation;

	vec3 m_RotationStart;
	mat4 m_Projection;

	bool m_bMouseDrag;

	double m_MouseStartX;
	double m_MouseStartY;

	float fCameraSpeed;
	float fCameraRotateSpeed; 
};
