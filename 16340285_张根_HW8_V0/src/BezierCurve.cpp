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

// 鼠标点击坐标
float mouseX, mouseY;
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

// 鼠标点击生成的控制点

float controlPoints[1000] = { 0.0f };
int controlPointsNum = 0;

// 曲线经过的点

float curvePoints[1000000] = { 0.0f };
int curvePointsNum = 0;

// 线段顶点

float linePoints[5000] = { 0.0f };

// 创建顶点缓冲对象VBO

GLuint VBO = 0;

// 定义顶点数组对象VAO

GLuint curveVAO = 0;
GLuint pointsVAO = 0;
GLuint linesVAO = 0;

// 演示相关
// 是否演示

bool flag = false;

// 此时的t值

float tmp = 0;

// 定义窗口大小改变的回调函数

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
// 鼠标坐标回调函数

void CursorPosCallback(GLFWwindow* window, double x, double y) {
	mouseX = x;
	mouseY = y;
}

// 鼠标点击回调函数

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

	// 点击左键

	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
		
		float xpos = mouseX / SCREEN_WIDTH * 2 - 1;
		float ypos = -(mouseY / SCREEN_HEIGHT * 2 - 1);
		
		// 添加控制点

		controlPoints[controlPointsNum * 2] = xpos;
		controlPoints[controlPointsNum * 2 + 1] = ypos;
		controlPointsNum++;
		cout << xpos << ' ' << ypos << endl;
	}

	// 点击右键

	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
		
		// 删除最后添加的控制点

		if (controlPointsNum > 0) {
			controlPointsNum--;
		}
	}
}

// 计算伯恩斯坦基函数

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

// 贝塞尔曲线算法，计算贝塞尔曲线经过的所有点

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

// 获取线段端点

void Lines(float t){

	// 添加控制点

	for (int i = 0; i < controlPointsNum * 2; i++) {
		linePoints[i] = controlPoints[i];
	}

	// 插值

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
	// 实例化GLFW窗口

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建窗口对象，通知GLFW将窗口的上下文设置为当前线程的主上下文

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CG_HW8", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// 初始化GLAD

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 定义渲染窗口尺寸大小

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// 引入改变窗口大小的回调函数

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);

	// 启用深度测试

	glEnable(GL_DEPTH_TEST);

	// 顶点着色器源代码

	const GLchar* VertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"

		"void main() \n"
		"{\n"
			"gl_Position = vec4(vertex_position, 1.0);\n"
		"}\n\0";

	// 创建着色器

	GLint VertexShader;
	VertexShader = glCreateShader(GL_VERTEX_SHADER);

	// 编译顶点着色器代码，1指定了传递的源码字符串数量

	glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
	glCompileShader(VertexShader);

	// 片段着色器(计算像素最后的颜色输出)源代码

	const GLchar* FragmentShaderSource = "#version 330 core\n"
		"out vec4 fragment_color;\n"

		"void main()\n"
		"{\n"

			"fragment_color = vec4(1.0, 1.0, 1.0, 1.0);\n"

		"}\n\0";

	// 创建着色器

	GLint FragmentShader;
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 编译片段着色器的代码

	glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
	glCompileShader(FragmentShader);

	//这里检查编译是否成功

	GLint status;
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &status);

	//着色器编译调试

	if (status != GL_TRUE) {
		cout <<"深度贴图顶点着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(VertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "深度贴图片段着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(FragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// 创建着色器程序

	GLint shader_programme;
	shader_programme = glCreateProgram();

	// 附加上两个着色器，并链接起来

	glAttachShader(shader_programme, VertexShader);
	glAttachShader(shader_programme, FragmentShader);
	glLinkProgram(shader_programme);

	// 删除着色器对象

	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);


	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &curveVAO);
	glGenVertexArrays(1, &pointsVAO);
	glGenVertexArrays(1, &linesVAO);

	// 渲染循环

	while (!glfwWindowShouldClose(window))
	{
		// 处理输入
		processInput(window);

		// 清空屏幕
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(shader_programme);

		// 由于顶点数据会在渲染途中发生改变，所以需要写在渲染循环内部

		// 绑定顶点数组对象

		glBindVertexArray(pointsVAO);

		// 绑定顶点位置缓冲对象，GL_ARRAY_BUFFER是VBO的缓冲类型

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		// 将顶点数据复制到绑定当前绑定缓冲，GL_STATIC_DRAW定义显卡管理给定数据的方式，GL_STATIC_DRAW表示数据几乎不会改变

		glBufferData(GL_ARRAY_BUFFER, sizeof(controlPoints), controlPoints, GL_STATIC_DRAW);

		/*	设置顶点坐标属性
		第一个参数为索引，表示映射到顶点着色器中的下标（唯一）
		第二个参数为变量包含的组件数，这里因为是三个顶点，所以是3
		第三个参数为缓冲区数据类型
		第四个参数定义数据是否被标准化（指数值映射到0-1间）
		第五个参数为步长(Stride)，表明连续的顶点属性组之间的间隔，这里可以设为3 * sizeof(float)，由于在两个顶点属性之间没有空隙，所以可以设为0，让OpenGL自定
		第六个参数表示数据在缓冲中起始位置的偏移量(Offset)
		*/

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));

		// 启用定义的属性，使着色器知道在哪读数据

		glEnableVertexAttribArray(0);

		// 有控制点

		if (controlPointsNum > 0) {
			glPointSize(8);
			glDrawArrays(GL_POINTS, 0, controlPointsNum);
		}
		// 控制点大于2个
		if (controlPointsNum > 1) {
			glDrawArrays(GL_LINE_STRIP, 0, controlPointsNum);

			// 每次渲染都重新生成一次曲线，以便及时更新
			// 获得贝塞尔曲线经过的点

			BezierCurve();

			glBindVertexArray(curveVAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, sizeof(curvePoints), curvePoints, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));

			glEnableVertexAttribArray(0);

			glPointSize(1);
			glDrawArrays(GL_POINTS, 0, curvePointsNum);


		}

		// 随时间递增，用于画线演示
		if (flag) {
			tmp += 0.001;
			tmp = tmp > 1 ? 0 : tmp;
		}

		Lines(tmp);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 释放资源，退出

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}