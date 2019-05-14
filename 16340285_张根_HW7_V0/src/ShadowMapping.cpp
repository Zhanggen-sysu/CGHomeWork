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

	// 启用深度测试

	glEnable(GL_DEPTH_TEST);

	// 深度贴图顶点着色器源代码

	const GLchar* DepthMapVertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"
		"layout(location = 1) in vec2 texture_coord;\n"
		"out vec2 TextureCoord;\n"

		"void main() \n"
		"{\n"
			"TextureCoord = texture_coord;\n"
			"gl_Position = vec4(vertex_position, 1.0);\n"
		"}\n\0";

	// 创建着色器

	GLint DepthMapVertexShader;
	DepthMapVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// 编译顶点着色器代码，1指定了传递的源码字符串数量

	glShaderSource(DepthMapVertexShader, 1, &DepthMapVertexShaderSource, NULL);
	glCompileShader(DepthMapVertexShader);

	// 深度贴图片段着色器(计算像素最后的颜色输出)源代码

	const GLchar* DepthMapFragmentShaderSource = "#version 330 core\n"
		"out vec4 fragment_color;\n"
		"in vec2 TextureCoord;\n"

		"uniform sampler2D depthMap;\n"
		"uniform float near_plane;\n"
		"uniform float far_plane;\n"
		"uniform bool flag;\n"

		"void main()\n"
		"{\n"
			"float depthValue = texture(depthMap, TextureCoord).r;\n"
			"if(!flag){\n"
				"float z = depthValue * 2.0 - 1.0;\n"
				"float coord =  (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));\n"
				"fragment_color = vec4(vec3(coord / far_plane), 1.0);\n"
			"}\n"
			"else{\n"
				"fragment_color = vec4(vec3(depthValue), 1.0);\n"
			"}\n"
		"}\n\0";

	// 创建着色器

	GLint DepthMapFragmentShader;
	DepthMapFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 编译片段着色器的代码

	glShaderSource(DepthMapFragmentShader, 1, &DepthMapFragmentShaderSource, NULL);
	glCompileShader(DepthMapFragmentShader);

	//这里检查编译是否成功

	GLint status;
	glGetShaderiv(DepthMapVertexShader, GL_COMPILE_STATUS, &status);

	//着色器编译调试

	if (status != GL_TRUE) {
		cout <<"深度贴图顶点着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(DepthMapVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(DepthMapFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "深度贴图片段着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(DepthMapFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// 创建着色器程序

	GLint depthMap_shader_programme;
	depthMap_shader_programme = glCreateProgram();

	// 附加上两个着色器，并链接起来

	glAttachShader(depthMap_shader_programme, DepthMapVertexShader);
	glAttachShader(depthMap_shader_programme, DepthMapFragmentShader);
	glLinkProgram(depthMap_shader_programme);

	// 删除着色器对象

	glDeleteShader(DepthMapVertexShader);
	glDeleteShader(DepthMapFragmentShader);


	// 光空间顶点着色器源代码

	const GLchar* LightSpaceVertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"

		"uniform mat4 lightSpaceMat;\n"
		"uniform mat4 model;\n"

		"void main() \n"
		"{\n"
			"gl_Position = lightSpaceMat * model * vec4(vertex_position, 1.0);\n"
		"}\n\0";

	// 创建着色器

	GLint LightSpaceVertexShader;
	LightSpaceVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// 编译顶点着色器代码，1指定了传递的源码字符串数量

	glShaderSource(LightSpaceVertexShader, 1, &LightSpaceVertexShaderSource, NULL);
	glCompileShader(LightSpaceVertexShader);

	// 光空间片段着色器(计算像素最后的颜色输出)源代码

	const GLchar* LightSpaceFragmentShaderSource = "#version 330 core\n"
		"void main()\n"
		"{\n"
			
		"}\n\0";

	// 创建着色器

	GLint LightSpaceFragmentShader;
	LightSpaceFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 编译片段着色器的代码

	glShaderSource(LightSpaceFragmentShader, 1, &LightSpaceFragmentShaderSource, NULL);
	glCompileShader(LightSpaceFragmentShader);

	//这里检查编译是否成功

	glGetShaderiv(LightSpaceVertexShader, GL_COMPILE_STATUS, &status);

	//着色器编译调试

	if (status != GL_TRUE) {
		cout << "光空间顶点着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(LightSpaceVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(LightSpaceFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "光空间片段着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(LightSpaceFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// 创建着色器程序

	GLint lightSpace_shader_programme;
	lightSpace_shader_programme = glCreateProgram();

	// 附加上两个着色器，并链接起来

	glAttachShader(lightSpace_shader_programme, LightSpaceVertexShader);
	glAttachShader(lightSpace_shader_programme, LightSpaceFragmentShader);
	glLinkProgram(lightSpace_shader_programme);

	// 删除着色器对象

	glDeleteShader(LightSpaceVertexShader);
	glDeleteShader(LightSpaceFragmentShader);

	// 阴影映射顶点着色器源代码

	const GLchar* ShadowMappingVertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"
		"layout (location = 1) in vec3 vertex_normal;\n"
		"layout (location = 2) in vec2 texture_coord;\n"

		"out vec3 FragPos;\n"
		"out vec3 Normal;\n"
		"out vec2 TextureCoord;\n"
		"out vec4 FragPosLightSpace;\n"

		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 lightSpaceMatrix;"

		"void main() \n"
		"{\n"
			"FragPos = vec3(model * vec4(vertex_position, 1.0));\n"
			"Normal = transpose(inverse(mat3(model))) * vertex_normal;\n"
			"TextureCoord = texture_coord;\n"
			"FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);\n"
			"gl_Position = projection * view * model * vec4(vertex_position, 1.0);\n"
		"}\n\0";

	// 创建着色器

	GLint ShadowMappingVertexShader;
	ShadowMappingVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// 编译顶点着色器代码，1指定了传递的源码字符串数量

	glShaderSource(ShadowMappingVertexShader, 1, &ShadowMappingVertexShaderSource, NULL);
	glCompileShader(ShadowMappingVertexShader);

	// 阴影映射片段着色器(计算像素最后的颜色输出)源代码

	const GLchar* ShadowMappingFragmentShaderSource = "#version 330 core\n"
		"in vec3 FragPos;\n"
		"in vec3 Normal;\n"
		"in vec2 TextureCoord;\n"
		"in vec4 FragPosLightSpace;\n"
		"out vec4 fragment_color;\n"

		"uniform sampler2D diffuseTexture;\n"
		"uniform sampler2D shadowMap;\n"
		"uniform vec3 lightPos;\n"
		"uniform vec3 viewPos;\n"

		"void main()\n"
		"{\n"
			"vec3 color = texture(diffuseTexture, TextureCoord).rgb;\n"
			"vec3 normal = normalize(Normal); \n"
			"vec3 lightColor = vec3(0.3); \n"

			"vec3 ambient = 0.3 * color; \n"

			"vec3 lightDir = normalize(lightPos - FragPos); \n"
			"float diff = max(dot(lightDir, normal), 0.0); \n"
			"vec3 diffuse = diff * lightColor; \n"

			"vec3 viewDir = normalize(viewPos - FragPos); \n"
			"vec3 reflectDir = reflect(-lightDir, normal); \n"
			"float spec = 0.0;\n"
			"vec3 halfwayDir = normalize(lightDir + viewDir); \n"
			"spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0); \n"
			"vec3 specular = spec * lightColor; \n"

			"vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w; \n"
			"projCoords = projCoords * 0.5 + 0.5; \n"
			"float closestDepth = texture(shadowMap, projCoords.xy).r; \n"

			"float currentDepth = projCoords.z; \n"
			"float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); \n"

			"float shadow = 0.0; \n"
			"vec2 texelSize = 1.0 / textureSize(shadowMap, 0); \n"
			"for (int x = -1; x <= 1; ++x)\n"
			"{\n"
				"for (int y = -1; y <= 1; ++y)\n"
				"{\n"
					"float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; \n"
					"shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0; \n"
				"}\n"
			"}\n"
			"shadow /= 9.0; \n"

			"if (projCoords.z > 1.0)shadow = 0.0; \n"

			"vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color; \n"

			"fragment_color = vec4(lighting, 1.0); \n"
		"}\n\0";

	// 创建着色器

	GLint ShadowMappingFragmentShader;
	ShadowMappingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// 编译片段着色器的代码

	glShaderSource(ShadowMappingFragmentShader, 1, &ShadowMappingFragmentShaderSource, NULL);
	glCompileShader(ShadowMappingFragmentShader);

	//这里检查编译是否成功

	glGetShaderiv(ShadowMappingVertexShader, GL_COMPILE_STATUS, &status);

	//着色器编译调试

	if (status != GL_TRUE) {
		cout << "阴影映射顶点着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(ShadowMappingVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(ShadowMappingFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "阴影映射片段着色器编译失败\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//实际错误日志长度
		glGetShaderInfoLog(ShadowMappingFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// 创建着色器程序

	GLint shadowMapping_shader_programme;
	shadowMapping_shader_programme = glCreateProgram();

	// 附加上两个着色器，并链接起来

	glAttachShader(shadowMapping_shader_programme, ShadowMappingVertexShader);
	glAttachShader(shadowMapping_shader_programme, ShadowMappingFragmentShader);
	glLinkProgram(shadowMapping_shader_programme);

	// 删除着色器对象

	glDeleteShader(ShadowMappingVertexShader);
	glDeleteShader(ShadowMappingFragmentShader);

	float cubeVertexData[] = {
		// 位置坐标			// 法向量				// 纹理坐标
		// 后
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // 左下
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // 右上
		1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // 右下         
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // 右上
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // 左下
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // 左上
		// 前
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // 左下
		1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // 右下
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // 右上
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // 右上
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // 左上
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // 左下
		// 左
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // 右上
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // 左上
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // 左下
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // 左下
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // 右下
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // 右上
		// 右
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // 左上
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // 右下
		1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // 右上         
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // 右下
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // 左上
		1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // 左下     
		// 下
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // 右上
		1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // 左上
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // 左下
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // 左下
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // 右下
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // 右上
		// 上
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // 左上
		1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // 右下
		1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // 右上     
		1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // 右下
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // 左上
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // 左下        
	};
	// 创建顶点缓冲对象VBO

	GLuint cubeVBO = 0;
	glGenBuffers(1, &cubeVBO);

	// 定义顶点数组对象VAO

	GLuint cubeVAO = 0;
	glGenVertexArrays(1, &cubeVAO);

	// 绑定顶点数组对象

	glBindVertexArray(cubeVAO);

	// 绑定顶点位置缓冲对象，GL_ARRAY_BUFFER是VBO的缓冲类型

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

	// 将顶点数据复制到绑定当前绑定缓冲，GL_STATIC_DRAW定义显卡管理给定数据的方式，GL_STATIC_DRAW表示数据几乎不会改变

	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexData), cubeVertexData, GL_STATIC_DRAW);

	/*	设置顶点坐标属性
	第一个参数为索引，表示映射到顶点着色器中的下标（唯一）
	第二个参数为变量包含的组件数，这里因为是三个顶点，所以是3
	第三个参数为缓冲区数据类型
	第四个参数定义数据是否被标准化（指数值映射到0-1间）
	第五个参数为步长(Stride)，表明连续的顶点属性组之间的间隔，这里可以设为3 * sizeof(float)，由于在两个顶点属性之间没有空隙，所以可以设为0，让OpenGL自定
	第六个参数表示数据在缓冲中起始位置的偏移量(Offset)
	*/

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));

	// 启用定义的属性，使着色器知道在哪读数据

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// 平面顶点（两个三角形组合）

	float planeVertexData[] = {
		// 位置坐标				// 法向量	        // 纹理坐标
		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};

	// 创建顶点缓冲对象VBO

	GLuint planeVBO = 0;
	glGenBuffers(1, &planeVBO);

	// 定义顶点数组对象VAO

	GLuint planeVAO = 0;
	glGenVertexArrays(1, &planeVAO);

	// 绑定顶点数组对象

	glBindVertexArray(planeVAO);

	// 绑定顶点位置缓冲对象，GL_ARRAY_BUFFER是VBO的缓冲类型

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

	// 将顶点数据复制到绑定当前绑定缓冲，GL_STATIC_DRAW定义显卡管理给定数据的方式，GL_STATIC_DRAW表示数据几乎不会改变

	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertexData), planeVertexData, GL_STATIC_DRAW);

	/*	设置顶点坐标属性
	第一个参数为索引，表示映射到顶点着色器中的下标（唯一）
	第二个参数为变量包含的组件数，这里因为是三个顶点，所以是3
	第三个参数为缓冲区数据类型
	第四个参数定义数据是否被标准化（指数值映射到0-1间）
	第五个参数为步长(Stride)，表明连续的顶点属性组之间的间隔，这里可以设为3 * sizeof(float)，由于在两个顶点属性之间没有空隙，所以可以设为0，让OpenGL自定
	第六个参数表示数据在缓冲中起始位置的偏移量(Offset)
	*/

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));
	
	// 启用定义的属性，使着色器知道在哪读数据

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	// 生成纹理

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//纹理目标为2D，指定S轴T轴为纹理轴，环绕方式为重复纹理图像

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// 设置放大，缩小时纹理过滤方式为线性过滤

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 加载图片

	int width, height, nrChannels;

	// 告诉 stb_image.h 在纵轴翻转加载纹理.

	stbi_set_flip_vertically_on_load(true);

	unsigned char *data = stbi_load("floor.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// 为渲染的深度贴图创建帧缓冲对象

	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// 创建2D纹理，提供给帧缓冲的深度缓冲使用

	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// 纹理边界颜色

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// 把生成的深度纹理作为帧缓冲的深度缓冲

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(shadowMapping_shader_programme);
	glUniform1i(glGetUniformLocation(shadowMapping_shader_programme, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shadowMapping_shader_programme, "shadowMap"), 1);

	glUseProgram(depthMap_shader_programme);
	glUniform1i(glGetUniformLocation(depthMap_shader_programme, "depthMap"), 0);

	glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

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
	static bool flag = true;
	// 渲染循环

	while (!glfwWindowShouldClose(window))
	{
		// 处理输入
		processInput(window);

		// 清空屏幕
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 把顶点变换到光空间

		glm::mat4 lightProjection, lightView, lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 7.5f;

		if (flag) {
			lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		}
		else {
			lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane);
		}
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		glUseProgram(lightSpace_shader_programme);
		glUniformMatrix4fv(glGetUniformLocation(lightSpace_shader_programme, "lightSpaceMat") , 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		// 渲染深度贴图
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		// 地面深度贴图
		glm::mat4 model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(lightSpace_shader_programme, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// 方块深度贴图
		model = glm::scale(model, glm::vec3(0.5f));
		glUniformMatrix4fv(glGetUniformLocation(lightSpace_shader_programme, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 渲染场景
		glViewport(0, 0, 800, 800);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shadowMapping_shader_programme);
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)800.0 / (float)800.0, 0.1f, 100.0f);
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		glUniformMatrix4fv(glGetUniformLocation(shadowMapping_shader_programme, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shadowMapping_shader_programme, "view"), 1, GL_FALSE, glm::value_ptr(view));
		
		glUniform3fv(glGetUniformLocation(shadowMapping_shader_programme, "viewPos"), 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 3.0f)));
		glUniform3fv(glGetUniformLocation(shadowMapping_shader_programme, "lightPos"), 1, glm::value_ptr(lightPos));
		glUniformMatrix4fv(glGetUniformLocation(shadowMapping_shader_programme, "lightSpaceMatrix"),  1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		// 地面场景贴图

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shadowMapping_shader_programme, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// 方块场景贴图
		model = glm::scale(model, glm::vec3(0.5f));
		glUniformMatrix4fv(glGetUniformLocation(shadowMapping_shader_programme, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// 创建GUI帧

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Setting");

		// 设置GUI大小

		ImGui::SetWindowSize(ImVec2(300, 170));

		ImGui::SliderFloat("ambient", &ambientStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("diffuse", &diffuseStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("specular", &specularStrength, 0.0f, 1.0f);
		ImGui::SliderInt("shininess", &shininess, 1, 8);

		ImGui::RadioButton("Phong Shading", &s, 0);
		ImGui::RadioButton("LightSpace Shading", &s, 1);
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