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

// 定义窗口大小改变的回调函数

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
	// 实例化GLFW窗口

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建窗口对象，通知GLFW将窗口的上下文设置为当前线程的主上下文

	GLFWwindow* window = glfwCreateWindow(800, 800, "CG_HW5", NULL, NULL);
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

	glViewport(0, 0, 800, 800);

	// 引入改变窗口大小的回调函数

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 冯氏顶点着色器源代码

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

	// 创建着色器

	GLint PhongVertexShader;
	PhongVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// 编译顶点着色器代码，1指定了传递的源码字符串数量

	glShaderSource(PhongVertexShader, 1, &PhongVertexShaderSource, NULL);
	glCompileShader(PhongVertexShader);

	// 冯氏片段着色器(计算像素最后的颜色输出)源代码

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

	// 创建着色器

	GLint PhongFragmentShader;
	PhongFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 编译片段着色器的代码

	glShaderSource(PhongFragmentShader, 1, &PhongFragmentShaderSource, NULL);
	glCompileShader(PhongFragmentShader);

	//这里检查编译是否成功

	GLint status;
	glGetShaderiv(PhongVertexShader, GL_COMPILE_STATUS, &status);

	//着色器编译调试

	if (status != GL_TRUE) {
		cout <<"冯氏顶点着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(PhongVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(PhongFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "冯氏片段着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(PhongFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// 创建着色器程序

	GLint phong_shader_programme;
	phong_shader_programme = glCreateProgram();

	// 附加上两个着色器，并链接起来

	glAttachShader(phong_shader_programme, PhongVertexShader);
	glAttachShader(phong_shader_programme, PhongFragmentShader);
	glLinkProgram(phong_shader_programme);

	// 删除着色器对象

	glDeleteShader(PhongVertexShader);
	glDeleteShader(PhongFragmentShader);


	// 高洛德顶点着色器源代码

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

	// 创建着色器

	GLint GouraudVertexShader;
	GouraudVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// 编译顶点着色器代码，1指定了传递的源码字符串数量

	glShaderSource(GouraudVertexShader, 1, &GouraudVertexShaderSource, NULL);
	glCompileShader(GouraudVertexShader);

	// 高洛德片段着色器(计算像素最后的颜色输出)源代码

	const GLchar* GouraudFragmentShaderSource = "#version 330 core\n"
		"out vec4 fragment_color;\n"

		"in vec3 vertex_color;\n"

		"uniform vec3 object_color;\n"

		"void main()\n"
		"{\n"
			"fragment_color = vec4(vertex_color * object_color, 1.0);\n"
		"}\n\0";

	// 创建着色器

	GLint GouraudFragmentShader;
	GouraudFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 编译片段着色器的代码

	glShaderSource(GouraudFragmentShader, 1, &GouraudFragmentShaderSource, NULL);
	glCompileShader(GouraudFragmentShader);

	//这里检查编译是否成功

	glGetShaderiv(GouraudVertexShader, GL_COMPILE_STATUS, &status);

	//着色器编译调试

	if (status != GL_TRUE) {
		cout << "高洛德顶点着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(GouraudVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(GouraudFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "高洛德片段着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(GouraudFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// 创建着色器程序

	GLint gouraud_shader_programme;
	gouraud_shader_programme = glCreateProgram();

	// 附加上两个着色器，并链接起来

	glAttachShader(gouraud_shader_programme, GouraudVertexShader);
	glAttachShader(gouraud_shader_programme, GouraudFragmentShader);
	glLinkProgram(gouraud_shader_programme);

	// 删除着色器对象

	glDeleteShader(GouraudVertexShader);
	glDeleteShader(GouraudFragmentShader);

	// 光源顶点着色器源代码

	const GLchar* LightSourceVertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"

		"void main() \n"
		"{\n"
			"gl_Position = projection * view * model * vec4(vertex_position, 1.0);\n"
		"}\n\0";

	// 创建着色器

	GLint LightSourceVertexShader;
	LightSourceVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// 编译顶点着色器代码，1指定了传递的源码字符串数量

	glShaderSource(LightSourceVertexShader, 1, &LightSourceVertexShaderSource, NULL);
	glCompileShader(LightSourceVertexShader);

	// 光源片段着色器(计算像素最后的颜色输出)源代码

	const GLchar* LightSourceFragmentShaderSource = "#version 330 core\n"
		"out vec4 fragment_color;\n"

		"void main()\n"
		"{\n"
			"fragment_color = vec4(1.0);\n"
		"}\n\0";

	// 创建着色器

	GLint LightSourceFragmentShader;
	LightSourceFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 编译片段着色器的代码

	glShaderSource(LightSourceFragmentShader, 1, &LightSourceFragmentShaderSource, NULL);
	glCompileShader(LightSourceFragmentShader);

	//这里检查编译是否成功

	glGetShaderiv(LightSourceVertexShader, GL_COMPILE_STATUS, &status);

	//着色器编译调试

	if (status != GL_TRUE) {
		cout << "光源顶点着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(LightSourceVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(LightSourceFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "光源片段着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(LightSourceFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// 创建着色器程序

	GLint lightsource_shader_programme;
	lightsource_shader_programme = glCreateProgram();

	// 附加上两个着色器，并链接起来

	glAttachShader(lightsource_shader_programme, LightSourceVertexShader);
	glAttachShader(lightsource_shader_programme, LightSourceFragmentShader);
	glLinkProgram(lightsource_shader_programme);

	// 删除着色器对象

	glDeleteShader(LightSourceVertexShader);
	glDeleteShader(LightSourceFragmentShader);


	// 立方体顶点（多个三角形组合）

	float points[] = {

		//点坐标，           //对应法向量

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

	// 创建顶点缓冲对象VBO

	GLuint points_vbo = 0;
	glGenBuffers(1, &points_vbo);

	// 定义顶点数组对象VAO

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);

	// 绑定顶点数组对象

	glBindVertexArray(vao);

	// 绑定顶点位置缓冲对象，GL_ARRAY_BUFFER是VBO的缓冲类型

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);

	// 将顶点数据复制到绑定当前绑定缓冲，GL_STATIC_DRAW定义显卡管理给定数据的方式，GL_STATIC_DRAW表示数据几乎不会改变

	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	/*	设置顶点坐标属性
	第一个参数为索引，表示映射到顶点着色器中的下标（唯一）
	第二个参数为变量包含的组件数，这里因为是三个顶点，所以是3
	第三个参数为缓冲区数据类型
	第四个参数定义数据是否被标准化（指数值映射到0-1间）
	第五个参数为步长(Stride)，表明连续的顶点属性组之间的间隔，这里可以设为3 * sizeof(float)，由于在两个顶点属性之间没有空隙，所以可以设为0，让OpenGL自定
	第六个参数表示数据在缓冲中起始位置的偏移量(Offset)
	*/

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(0));
	
	// 启用定义的属性，使着色器知道在哪读数据

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// 光源也是个方块，需要用到VAO

	GLuint light_vao;
	glGenVertexArrays(1, &light_vao);
	glBindVertexArray(light_vao);

	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	// 创建GUI

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// 初始化GUI

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	//深度测试
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);

	// GUI颜色主题

	ImGui::StyleColorsDark();

	static int s = 0;
	static float ambientStrength = 0.1;
	static float diffuseStrength = 0.5;
	static float specularStrength = 1.0;
	static int shininess = 5;

	// 渲染循环

	while (!glfwWindowShouldClose(window))
	{
		// 处理输入
		processInput(window);

		// 清空屏幕
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

			// 激活着色器程序
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

			// 激活着色器程序
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
		// 绘制三角形
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

		// 创建GUI帧

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Setting");

		// 设置GUI大小

		ImGui::SetWindowSize(ImVec2(300, 300));

		ImGui::SliderFloat("ambient", &ambientStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("diffuse", &diffuseStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("specular", &specularStrength, 0.0f, 1.0f);
		ImGui::SliderInt("shininess", &shininess, 1, 8);

		ImGui::RadioButton("Phong Shading", &s, 0);
		ImGui::RadioButton("Gouraud Shading", &s, 1);
		ImGui::End();

		// 渲染

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 释放资源，退出

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}