#include "en_gl_camera.h"
#include <glm/gtx/quaternion.hpp>
#include "pe_physic_engine.h"

const glm::vec3 worldUp = glm::vec3(0, 1, 0);

CFreeCamera::CFreeCamera()
{
}
CFreeCamera::~CFreeCamera()
{
};
void CFreeCamera::initialize(glm::vec3 cameraOrigin)
{
	glm::vec3 orig{cameraOrigin};

	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDirection = glm::normalize(orig - cameraTarget);

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

	// glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

	m_front = glm::normalize(cameraTarget - orig);

	glm::vec3 right = normalize(cross(m_front, worldUp));
	m_up = normalize(cross(right, m_front));


	glm::mat4 view;
	view = glm::lookAt(orig, orig + m_front, m_up);
	view = view;// glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	updateProject();
}

void CFreeCamera::updateProject()
{
	m_projectRt = glm::perspective(glm::radians(95.0f), m_width / (float)m_height, 0.1f, 200.0f);
	m_project = m_projectRt;
	m_project[1][1] *= -1;
	//m_ubo.proj = glm::ortho(0.f, (float)m_width, 0.f, (float)m_height, -100.f, 100.f);// , m_width / (float)m_height, 0.1f, 100.0f);
	//
}

void CFreeCamera::updateAngle(glm::vec3 deltaOrient, bool bUpdate)
{
	if(bUpdate)
	{
		m_orientation += deltaOrient;
	}
}

constexpr float cameraDistance = 3.5f;
constexpr float cameraHeight = 2.0f;

void CFreeCamera::resize(float width, float height)
{
	m_width = width;
	m_height = height;
	updateProject();
}

glm::mat4 CFreeCamera::view() const
{
	return m_view;
}
glm::mat4 CFreeCamera::project() const
{
	return m_project;
}

glm::mat4 CFreeCamera::viewRt()
{
	return m_viewRt;
}
glm::mat4 CFreeCamera::projectRt()
{
	return m_projectRt;
}

glm::vec3 CFreeCamera::front()
{
	return m_front;
}

glm::vec3 CFreeCamera::orientation()
{
	return m_orientation;
}

glm::vec3 CFreeCamera::origin()
{
	return m_origin;
}

glm::vec3 CFreeCamera::up()
{
	return m_up;
}

void CFreeCamera::updatePos(glm::vec3 origin)
{
	m_origin = origin;

	if(m_orientation.y > 89.0f) m_orientation.y = 89.0f;
	if(m_orientation.y < -89.0f) m_orientation.y = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(m_orientation.y)) * cos(glm::radians(m_orientation.x));
	front.y = sin(glm::radians(m_orientation.y));
	front.z = cos(glm::radians(m_orientation.y)) * sin(glm::radians(m_orientation.x));
	m_front = normalize(front);

	glm::vec3 worldUp = glm::vec3(0, 1, 0);
	glm::vec3 right = normalize(cross(m_front, worldUp));
	m_up = normalize(cross(right, m_front));

	glm::vec3 delta{cameraDistance * m_front.x, -cameraHeight, cameraDistance * m_front.z};
	m_view = glm::lookAt(m_origin - delta, m_origin + m_front - delta, m_up);
	m_viewRt = glm::lookAt(m_origin, m_origin + m_front, m_up);
}

void COrbitCamera::rotate(float deltaX, float deltaY)
{
	m_yaw += deltaX * m_rotateSpeed;
	m_pitch += deltaY * m_rotateSpeed;

	if(m_pitch > 89.0f)  
		m_pitch = 89.0f;
	if(m_pitch < -89.0f) 
		m_pitch = -89.0f;

	updateView();
}

void COrbitCamera::zoom(float offset)
{
	m_distance -= offset * m_zoomSpeed;

	if(m_distance < 0.1f)
		m_distance = 0.1f;

	updateView();
}

void COrbitCamera::pan(float deltaX, float deltaY)
{
	glm::vec3 forward = glm::normalize(m_target - m_position);
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	m_target += -right * deltaX * m_panSpeed * m_distance;
	m_target += up * deltaY * m_panSpeed * m_distance;

	updateView();
}

void COrbitCamera::updateView()
{
	glm::vec3 position;

	position.x = m_target.x + m_distance * cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
	position.y = m_target.y + m_distance * sin(glm::radians(m_pitch));
	position.z = m_target.z + m_distance * cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));

	m_position = position;

	m_view = glm::lookAt(
		m_position,
		m_target,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
}
void COrbitCamera::resize(float width, float height)
{
	m_width = width;
	m_height = height;

	m_projection = glm::perspective(
		glm::radians(60.0f),
		m_width / m_height,
		0.1f,
		500.0f
	);

	m_projection[1][1] *= -1; //

	updateView();
}

