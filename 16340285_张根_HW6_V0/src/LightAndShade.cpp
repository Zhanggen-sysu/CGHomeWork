#include <glad/glad.h> 
#include <iostream>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "camera.h"
#define PI 3.1415926

using namespace std;

// ���崰�ڴ�С�ı�Ļص�����

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

int main()
{
	// ʵ����GLFW����

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// �������ڶ���֪ͨGLFW�����ڵ�����������Ϊ��ǰ�̵߳���������

	GLFWwindow* window = glfwCreateWindow(800, 800, "CG_HW5", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// ��ʼ��GLAD

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// ������Ⱦ���ڳߴ��С

	glViewport(0, 0, 800, 800);

	// ����ı䴰�ڴ�С�Ļص�����

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// ���϶�����ɫ��Դ����

	const GLchar* PhongVertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"
		"layout(location = 1) in vec3 vertex_normal;\n"
		"out vec3 fragment_position;\n"
		"out vec3 fragment_normal;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"

		"void main() \n"
		"{\n"
			"fragment_position = vec3(model * vec4(vertex_position, 1.0));\n"
			"fragment_normal = mat3(transpose(inverse(model))) * vertex_normal; \n"
			"gl_Position = projection * view * vec4(fragment_position, 1.0);\n"
		"}\n\0";

	// ������ɫ��

	GLint PhongVertexShader;
	PhongVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// ���붥����ɫ�����룬1ָ���˴��ݵ�Դ���ַ�������

	glShaderSource(PhongVertexShader, 1, &PhongVertexShaderSource, NULL);
	glCompileShader(PhongVertexShader);

	// ����Ƭ����ɫ��(��������������ɫ���)Դ����

	const GLchar* PhongFragmentShaderSource = "#version 330 core\n"
		"out vec4 fragment_color;\n"
		"in vec3 fragment_position;\n"
		"in vec3 fragment_normal;\n"

		"uniform vec3 light_position;\n"
		"uniform vec3 view_position;\n"

		"uniform vec3 light_color;\n"
		"uniform vec3 object_color;\n"

		"uniform float ambientStrength;\n"
		"uniform float diffuseStrength;\n"
		"uniform float specularStrength;\n"
		"uniform int shininess;\n"

		"void main()\n"
		"{\n"
			
			"vec3 ambient = ambientStrength * light_color;"

			"vec3 norm = normalize(fragment_normal);\n"
			"vec3 light_direction = normalize(light_position - fragment_position);\n"
			"float diff = max(dot(norm, light_direction), 0.0);\n"
			"vec3 diffuse = diffuseStrength * diff * light_color;\n"

			"vec3 view_direction = normalize(view_position - fragment_position);\n"
			"vec3 reflect_direction = reflect(-light_direction, norm); \n"
			"float spec = pow(max(dot(view_direction, reflect_direction), 0.0), shininess);\n"
			"vec3 specular = specularStrength * spec * light_color;\n"
			"vec3 result = (ambient + diffuse + specular) * object_color;\n"
			"fragment_color = vec4(result, 1.0);\n"
		"}\n\0";

	// ������ɫ��

	GLint PhongFragmentShader;
	PhongFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// ����Ƭ����ɫ���Ĵ���

	glShaderSource(PhongFragmentShader, 1, &PhongFragmentShaderSource, NULL);
	glCompileShader(PhongFragmentShader);

	//����������Ƿ�ɹ�

	GLint status;
	glGetShaderiv(PhongVertexShader, GL_COMPILE_STATUS, &status);

	//��ɫ���������

	if (status != GL_TRUE) {
		cout <<"���϶�����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(PhongVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(PhongFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "����Ƭ����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(PhongFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// ������ɫ������

	GLint phong_shader_programme;
	phong_shader_programme = glCreateProgram();

	// ������������ɫ��������������

	glAttachShader(phong_shader_programme, PhongVertexShader);
	glAttachShader(phong_shader_programme, PhongFragmentShader);
	glLinkProgram(phong_shader_programme);

	// ɾ����ɫ������

	glDeleteShader(PhongVertexShader);
	glDeleteShader(PhongFragmentShader);


	// ����¶�����ɫ��Դ����

	const GLchar* GouraudVertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"
		"layout(location = 1) in vec3 vertex_normal;\n"

		"out vec3 vertex_color;\n"

		"uniform vec3 light_position;\n"
		"uniform vec3 view_position;\n"

		"uniform vec3 light_color;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"

		"uniform float ambientStrength;\n"
		"uniform float diffuseStrength;\n"
		"uniform float specularStrength;\n"
		"uniform int shininess;\n"

		"void main() \n"
		"{\n"
			"gl_Position = projection * view * model * vec4(vertex_position, 1.0);\n"
			
			"vec3 fragment_position = vec3(model * vec4(vertex_position, 1.0));\n"
			"vec3 fragment_normal = mat3(transpose(inverse(model))) * vertex_normal;\n"
			
			"vec3 ambient = ambientStrength * light_color;"

			"vec3 norm = normalize(fragment_normal);\n"
			"vec3 light_direction = normalize(light_position - fragment_position);\n"
			"float diff = max(dot(norm, light_direction), 0.0);\n"
			"vec3 diffuse = diffuseStrength * diff * light_color;\n"

			"vec3 view_direction = normalize(view_position - fragment_position);\n"
			"vec3 reflect_direction = reflect(-light_direction, norm); \n"
			"float spec = pow(max(dot(view_direction, reflect_direction), 0.0), shininess);\n"
			"vec3 specular = specularStrength * spec * light_color;\n"
			
			"vertex_color =  ambient + diffuse + specular;\n"
			
		"}\n\0";

	// ������ɫ��

	GLint GouraudVertexShader;
	GouraudVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// ���붥����ɫ�����룬1ָ���˴��ݵ�Դ���ַ�������

	glShaderSource(GouraudVertexShader, 1, &GouraudVertexShaderSource, NULL);
	glCompileShader(GouraudVertexShader);

	// �����Ƭ����ɫ��(��������������ɫ���)Դ����

	const GLchar* GouraudFragmentShaderSource = "#version 330 core\n"
		"out vec4 fragment_color;\n"

		"in vec3 vertex_color;\n"

		"uniform vec3 object_color;\n"

		"void main()\n"
		"{\n"
			"fragment_color = vec4(vertex_color * object_color, 1.0);\n"
		"}\n\0";

	// ������ɫ��

	GLint GouraudFragmentShader;
	GouraudFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// ����Ƭ����ɫ���Ĵ���

	glShaderSource(GouraudFragmentShader, 1, &GouraudFragmentShaderSource, NULL);
	glCompileShader(GouraudFragmentShader);

	//����������Ƿ�ɹ�

	glGetShaderiv(GouraudVertexShader, GL_COMPILE_STATUS, &status);

	//��ɫ���������

	if (status != GL_TRUE) {
		cout << "����¶�����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(GouraudVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(GouraudFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "�����Ƭ����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(GouraudFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// ������ɫ������

	GLint gouraud_shader_programme;
	gouraud_shader_programme = glCreateProgram();

	// ������������ɫ��������������

	glAttachShader(gouraud_shader_programme, GouraudVertexShader);
	glAttachShader(gouraud_shader_programme, GouraudFragmentShader);
	glLinkProgram(gouraud_shader_programme);

	// ɾ����ɫ������

	glDeleteShader(GouraudVertexShader);
	glDeleteShader(GouraudFragmentShader);

	// ��Դ������ɫ��Դ����

	const GLchar* LightSourceVertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"

		"void main() \n"
		"{\n"
			"gl_Position = projection * view * model * vec4(vertex_position, 1.0);\n"
		"}\n\0";

	// ������ɫ��

	GLint LightSourceVertexShader;
	LightSourceVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// ���붥����ɫ�����룬1ָ���˴��ݵ�Դ���ַ�������

	glShaderSource(LightSourceVertexShader, 1, &LightSourceVertexShaderSource, NULL);
	glCompileShader(LightSourceVertexShader);

	// ��ԴƬ����ɫ��(��������������ɫ���)Դ����

	const GLchar* LightSourceFragmentShaderSource = "#version 330 core\n"
		"out vec4 fragment_color;\n"

		"void main()\n"
		"{\n"
			"fragment_color = vec4(1.0);\n"
		"}\n\0";

	// ������ɫ��

	GLint LightSourceFragmentShader;
	LightSourceFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// ����Ƭ����ɫ���Ĵ���

	glShaderSource(LightSourceFragmentShader, 1, &LightSourceFragmentShaderSource, NULL);
	glCompileShader(LightSourceFragmentShader);

	//����������Ƿ�ɹ�

	glGetShaderiv(LightSourceVertexShader, GL_COMPILE_STATUS, &status);

	//��ɫ���������

	if (status != GL_TRUE) {
		cout << "��Դ������ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(LightSourceVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(LightSourceFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "��ԴƬ����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(LightSourceFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// ������ɫ������

	GLint lightsource_shader_programme;
	lightsource_shader_programme = glCreateProgram();

	// ������������ɫ��������������

	glAttachShader(lightsource_shader_programme, LightSourceVertexShader);
	glAttachShader(lightsource_shader_programme, LightSourceFragmentShader);
	glLinkProgram(lightsource_shader_programme);

	// ɾ����ɫ������

	glDeleteShader(LightSourceVertexShader);
	glDeleteShader(LightSourceFragmentShader);


	// �����嶥�㣨�����������ϣ�

	float points[] = {

		//�����꣬           //��Ӧ������

		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f

	};

	// �������㻺�����VBO

	GLuint points_vbo = 0;
	glGenBuffers(1, &points_vbo);

	// ���嶥���������VAO

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);

	// �󶨶����������

	glBindVertexArray(vao);

	// �󶨶���λ�û������GL_ARRAY_BUFFER��VBO�Ļ�������

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);

	// ���������ݸ��Ƶ��󶨵�ǰ�󶨻��壬GL_STATIC_DRAW�����Կ�����������ݵķ�ʽ��GL_STATIC_DRAW��ʾ���ݼ�������ı�

	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	/*	���ö�����������
	��һ������Ϊ��������ʾӳ�䵽������ɫ���е��±꣨Ψһ��
	�ڶ�������Ϊ�����������������������Ϊ���������㣬������3
	����������Ϊ��������������
	���ĸ��������������Ƿ񱻱�׼����ָ��ֵӳ�䵽0-1�䣩
	���������Ϊ����(Stride)�����������Ķ���������֮��ļ�������������Ϊ3 * sizeof(float)��������������������֮��û�п�϶�����Կ�����Ϊ0����OpenGL�Զ�
	������������ʾ�����ڻ�������ʼλ�õ�ƫ����(Offset)
	*/

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
	
	// ���ö�������ԣ�ʹ��ɫ��֪�����Ķ�����

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// ��ԴҲ�Ǹ����飬��Ҫ�õ�VAO

	GLuint light_vao;
	glGenVertexArrays(1, &light_vao);
	glBindVertexArray(light_vao);

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	// ����GUI

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// ��ʼ��GUI

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	//��Ȳ���
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);

	// GUI��ɫ����

	ImGui::StyleColorsDark();

	static int s = 0;
	static float ambientStrength = 0.1;
	static float diffuseStrength = 0.5;
	static float specularStrength = 1.0;
	static int shininess = 5;

	// ��Ⱦѭ��

	while (!glfwWindowShouldClose(window))
	{
		// ��������
		processInput(window);

		// �����Ļ
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)800.0 / (float)800.0, 0.1f, 100.0f);
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0, 1.0f, 0.5f));

		glm::vec3 view_position(0.0f, 0.0f, 3.0f);
		glm::vec3 light_position(0.7*cos((float)glfwGetTime()), 0.7*sin((float)glfwGetTime()), 1.0f);
		glm::vec3 object_color(0.0f, 0.0f, 1.0f);
		glm::vec3 light_color(1.0, 1.0, 1.0);

		if (s == 0) {

			// ������ɫ������
			glUseProgram(phong_shader_programme);
			GLuint projectionLoc = glGetUniformLocation(phong_shader_programme, "projection");
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			GLuint viewLoc = glGetUniformLocation(phong_shader_programme, "view");
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			GLuint modelLoc = glGetUniformLocation(phong_shader_programme, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			GLuint viewPosLoc = glGetUniformLocation(phong_shader_programme, "view_position");
			glUniform3fv(viewPosLoc, 1, glm::value_ptr(view_position));
			GLuint lightPosLoc = glGetUniformLocation(phong_shader_programme, "light_position");
			glUniform3fv(lightPosLoc, 1, glm::value_ptr(light_position));
			GLuint objColorLoc = glGetUniformLocation(phong_shader_programme, "object_color");
			glUniform3fv(objColorLoc, 1, glm::value_ptr(object_color));
			GLuint lightColorLoc = glGetUniformLocation(phong_shader_programme, "light_color");
			glUniform3fv(lightColorLoc, 1, glm::value_ptr(light_color));

			glUniform1f(glGetUniformLocation(phong_shader_programme, "ambientStrength"), ambientStrength);
			glUniform1f(glGetUniformLocation(phong_shader_programme, "diffuseStrength"), diffuseStrength);
			glUniform1f(glGetUniformLocation(phong_shader_programme, "specularStrength"), specularStrength);
			glUniform1i(glGetUniformLocation(phong_shader_programme, "shininess"), pow(2, shininess));

		}
		else {

			// ������ɫ������
			glUseProgram(gouraud_shader_programme);
			GLuint projectionLoc = glGetUniformLocation(gouraud_shader_programme, "projection");
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			GLuint viewLoc = glGetUniformLocation(gouraud_shader_programme, "view");
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			GLuint modelLoc = glGetUniformLocation(gouraud_shader_programme, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			GLuint viewPosLoc = glGetUniformLocation(gouraud_shader_programme, "view_position");
			glUniform3fv(viewPosLoc, 1, glm::value_ptr(view_position));
			GLuint lightPosLoc = glGetUniformLocation(gouraud_shader_programme, "light_position");
			glUniform3fv(lightPosLoc, 1, glm::value_ptr(light_position));
			GLuint objColorLoc = glGetUniformLocation(gouraud_shader_programme, "object_color");
			glUniform3fv(objColorLoc, 1, glm::value_ptr(object_color));
			GLuint lightColorLoc = glGetUniformLocation(gouraud_shader_programme, "light_color");
			glUniform3fv(lightColorLoc, 1, glm::value_ptr(light_color));

			glUniform1f(glGetUniformLocation(gouraud_shader_programme, "ambientStrength"), ambientStrength);
			glUniform1f(glGetUniformLocation(gouraud_shader_programme, "diffuseStrength"), diffuseStrength);
			glUniform1f(glGetUniformLocation(gouraud_shader_programme, "specularStrength"), specularStrength);
			glUniform1i(glGetUniformLocation(gouraud_shader_programme, "shininess"), pow(2, shininess));

		}
		// ����������
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glUseProgram(lightsource_shader_programme);
		GLuint projectionLoc = glGetUniformLocation(lightsource_shader_programme, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		GLuint viewLoc = glGetUniformLocation(lightsource_shader_programme, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		model = glm::mat4(1.0f);
		model = glm::translate(model, light_position);
		model = glm::scale(model, glm::vec3(0.02f));
		GLuint modelLoc = glGetUniformLocation(lightsource_shader_programme, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(light_vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ����GUI֡

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Setting");

		// ����GUI��С

		ImGui::SetWindowSize(ImVec2(300, 300));

		ImGui::SliderFloat("ambient", &ambientStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("diffuse", &diffuseStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("specular", &specularStrength, 0.0f, 1.0f);
		ImGui::SliderInt("shininess", &shininess, 1, 8);

		ImGui::RadioButton("Phong Shading", &s, 0);
		ImGui::RadioButton("Gouraud Shading", &s, 1);
		ImGui::End();

		// ��Ⱦ

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// �ͷ���Դ���˳�

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}