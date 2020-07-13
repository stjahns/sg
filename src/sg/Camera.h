#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "GlfwWindow.h"

using namespace glm;

class Camera
{
public:

	Camera() 
		: m_bMouseDrag(false)
		, m_MouseStartX(0.0)
		, m_MouseStartY(0.0)
		, m_Position(0.0f, 0.0f, 20.0f)
		, fCameraSpeed(10.0f)
		, fCameraRotateSpeed(1.0f)
	{
	}

	void OnMouseEvent(MouseEvent e)
	{
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			if (e.button == GLFW_MOUSE_BUTTON_LEFT)
			{
				if (e.action == GLFW_PRESS)
				{
					m_MouseStartX = e.mouseX;
					m_MouseStartY = e.mouseY;
					m_bMouseDrag = true;
					m_RotationStart = m_EulerRotation;
				}
				else if (e.action == GLFW_RELEASE)
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
	void Update(GLFWWindow& window)
	{
		const float fRotateRateScale = 0.003f;
		const float fTranslateRateScale = 1.500f;

		if (m_bMouseDrag)
		{
			double dMouseX;
			double dMouseY;
			window.GetCursorPos(dMouseX, dMouseY);
			vec3 cameraDelta(m_MouseStartY - dMouseY, m_MouseStartX - dMouseX, 0.0f);
			m_EulerRotation = m_RotationStart + cameraDelta * fCameraRotateSpeed * fRotateRateScale;
		}

		mat4 camera = GetMatrix();

		if (window.GetKey(GLFW_KEY_W) == GLFW_PRESS)
		{
			vec3 forward(camera[2]);
			m_Position -= forward * fCameraSpeed * fTranslateRateScale;
		}

		if (window.GetKey(GLFW_KEY_S) == GLFW_PRESS)
		{
			vec3 forward(camera[2]);
			m_Position += forward * fCameraSpeed * fTranslateRateScale;
		}

		if (window.GetKey(GLFW_KEY_A) == GLFW_PRESS)
		{
			vec3 right(camera[0]);
			m_Position -= right * fCameraSpeed * fTranslateRateScale;
		}

		if (window.GetKey(GLFW_KEY_D) == GLFW_PRESS)
		{
			vec3 right(camera[0]);
			m_Position += right * fCameraSpeed * fTranslateRateScale;
		}

		if (window.GetKey(GLFW_KEY_Q) == GLFW_PRESS)
		{
			vec3 up(camera[1]);
			m_Position -= up * fCameraSpeed * fTranslateRateScale;
		}

		if (window.GetKey(GLFW_KEY_E) == GLFW_PRESS)
		{
			vec3 up(camera[1]);
			m_Position += up * fCameraSpeed * fTranslateRateScale;
		}

		m_View = ComputeViewMatrix();
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
		return m_View;
	}

	mat4 ComputeViewMatrix()
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

	mat4 m_View;

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
