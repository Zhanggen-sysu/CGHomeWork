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

// ���������
float mouseX, mouseY;
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

// ��������ɵĿ��Ƶ�

float controlPoints[1000] = { 0.0f };
int controlPointsNum = 0;

// ���߾����ĵ�

float curvePoints[1000000] = { 0.0f };
int curvePointsNum = 0;

// �߶ζ���

float linePoints[5000] = { 0.0f };

// �������㻺�����VBO

GLuint VBO = 0;

// ���嶥���������VAO

GLuint curveVAO = 0;
GLuint pointsVAO = 0;
GLuint linesVAO = 0;

// ��ʾ���
// �Ƿ���ʾ

bool flag = false;

// ��ʱ��tֵ

float tmp = 0;

// ���崰�ڴ�С�ı�Ļص�����

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
// �������ص�����

void CursorPosCallback(GLFWwindow* window, double x, double y) {
	mouseX = x;
	mouseY = y;
}

// ������ص�����

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

	// ������

	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
		
		float xpos = mouseX / SCREEN_WIDTH * 2 - 1;
		float ypos = -(mouseY / SCREEN_HEIGHT * 2 - 1);
		
		// ��ӿ��Ƶ�

		controlPoints[controlPointsNum * 2] = xpos;
		controlPoints[controlPointsNum * 2 + 1] = ypos;
		controlPointsNum++;
		cout << xpos << ' ' << ypos << endl;
	}

	// ����Ҽ�

	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
		
		// ɾ�������ӵĿ��Ƶ�

		if (controlPointsNum > 0) {
			controlPointsNum--;
		}
	}
}

// ���㲮��˹̹������

float Bernstein(int i, int n, float t) {

	double Nn = 1, Ni = 1, Nni = 1;
	for (int k = 2; k <= i; k++) {
		Ni *= k;
	}

	for (int k = 2; k <= n - i; k++) {
		Nni *= k;
	}

	for (int k = 2; k <= n; k++) {
		Nn *= k;
	}

	return Nn * pow(t, i) * pow(1 - t, n - i) / (Ni * Nni);
}

// �����������㷨�����㱴�������߾��������е�

void BezierCurve() {
	curvePointsNum = 0;
	for (float t = 0.0; t < 1.0; t += 0.001) {

		float tmpX = 0.0f;
		float tmpY = 0.0f;

		for (int i = 0; i <= controlPointsNum - 1; i++) {
			float bernstein = Bernstein(i, controlPointsNum - 1, t);
			tmpX += controlPoints[2 * i] * bernstein;
			tmpY += controlPoints[2 * i + 1] * bernstein;
		}
		curvePoints[curvePointsNum * 2] = tmpX;
		curvePoints[curvePointsNum * 2 + 1] = tmpY;
		curvePointsNum++;
	}
}

// ��ȡ�߶ζ˵�

void Lines(float t){

	// ��ӿ��Ƶ�

	for (int i = 0; i < controlPointsNum * 2; i++) {
		linePoints[i] = controlPoints[i];
	}

	// ��ֵ

	for (int i = controlPointsNum; i > 1; i--) {
		for (int j = 0; j < i - 1; j++) {
			linePoints[j * 2] = linePoints[j * 2] * (1 - t) + linePoints[(j + 1) * 2] * t;
			linePoints[j * 2 + 1] = linePoints[j * 2 + 1] * (1 - t) + linePoints[(j + 1) * 2 + 1] * t;
		}
		glBindVertexArray(linesVAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(linePoints), linePoints, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));

		glEnableVertexAttribArray(0);

		glPointSize(8);
		glDrawArrays(GL_POINTS, 0, i - 1);
		glDrawArrays(GL_LINE_STRIP, 0, i - 1);
	}
}


void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		flag = !flag;
}

int main()
{
	// ʵ����GLFW����

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// �������ڶ���֪ͨGLFW�����ڵ�����������Ϊ��ǰ�̵߳���������

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CG_HW8", NULL, NULL);
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

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// ����ı䴰�ڴ�С�Ļص�����

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);

	// ������Ȳ���

	glEnable(GL_DEPTH_TEST);

	// ������ɫ��Դ����

	const GLchar* VertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"

		"void main() \n"
		"{\n"
			"gl_Position = vec4(vertex_position, 1.0);\n"
		"}\n\0";

	// ������ɫ��

	GLint VertexShader;
	VertexShader = glCreateShader(GL_VERTEX_SHADER);

	// ���붥����ɫ�����룬1ָ���˴��ݵ�Դ���ַ�������

	glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
	glCompileShader(VertexShader);

	// Ƭ����ɫ��(��������������ɫ���)Դ����

	const GLchar* FragmentShaderSource = "#version 330 core\n"
		"out vec4 fragment_color;\n"

		"void main()\n"
		"{\n"

			"fragment_color = vec4(1.0, 1.0, 1.0, 1.0);\n"

		"}\n\0";

	// ������ɫ��

	GLint FragmentShader;
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// ����Ƭ����ɫ���Ĵ���

	glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
	glCompileShader(FragmentShader);

	//����������Ƿ�ɹ�

	GLint status;
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &status);

	//��ɫ���������

	if (status != GL_TRUE) {
		cout <<"�����ͼ������ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(VertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "�����ͼƬ����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(FragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// ������ɫ������

	GLint shader_programme;
	shader_programme = glCreateProgram();

	// ������������ɫ��������������

	glAttachShader(shader_programme, VertexShader);
	glAttachShader(shader_programme, FragmentShader);
	glLinkProgram(shader_programme);

	// ɾ����ɫ������

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);


	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &curveVAO);
	glGenVertexArrays(1, &pointsVAO);
	glGenVertexArrays(1, &linesVAO);

	// ��Ⱦѭ��

	while (!glfwWindowShouldClose(window))
	{
		// ��������
		processInput(window);

		// �����Ļ
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader_programme);

		// ���ڶ������ݻ�����Ⱦ;�з����ı䣬������Ҫд����Ⱦѭ���ڲ�

		// �󶨶����������

		glBindVertexArray(pointsVAO);

		// �󶨶���λ�û������GL_ARRAY_BUFFER��VBO�Ļ�������

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// ���������ݸ��Ƶ��󶨵�ǰ�󶨻��壬GL_STATIC_DRAW�����Կ�����������ݵķ�ʽ��GL_STATIC_DRAW��ʾ���ݼ�������ı�

		glBufferData(GL_ARRAY_BUFFER, sizeof(controlPoints), controlPoints, GL_STATIC_DRAW);

		/*	���ö�����������
		��һ������Ϊ��������ʾӳ�䵽������ɫ���е��±꣨Ψһ��
		�ڶ�������Ϊ�����������������������Ϊ���������㣬������3
		����������Ϊ��������������
		���ĸ��������������Ƿ񱻱�׼����ָ��ֵӳ�䵽0-1�䣩
		���������Ϊ����(Stride)�����������Ķ���������֮��ļ�������������Ϊ3 * sizeof(float)��������������������֮��û�п�϶�����Կ�����Ϊ0����OpenGL�Զ�
		������������ʾ�����ڻ�������ʼλ�õ�ƫ����(Offset)
		*/

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));

		// ���ö�������ԣ�ʹ��ɫ��֪�����Ķ�����

		glEnableVertexAttribArray(0);

		// �п��Ƶ�

		if (controlPointsNum > 0) {
			glPointSize(8);
			glDrawArrays(GL_POINTS, 0, controlPointsNum);
		}
		// ���Ƶ����2��
		if (controlPointsNum > 1) {
			glDrawArrays(GL_LINE_STRIP, 0, controlPointsNum);

			// ÿ����Ⱦ����������һ�����ߣ��Ա㼰ʱ����
			// ��ñ��������߾����ĵ�

			BezierCurve();

			glBindVertexArray(curveVAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, sizeof(curvePoints), curvePoints, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));

			glEnableVertexAttribArray(0);

			glPointSize(1);
			glDrawArrays(GL_POINTS, 0, curvePointsNum);


		}

		// ��ʱ����������ڻ�����ʾ
		if (flag) {
			tmp += 0.001;
			tmp = tmp > 1 ? 0 : tmp;
		}

		Lines(tmp);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// �ͷ���Դ���˳�

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}