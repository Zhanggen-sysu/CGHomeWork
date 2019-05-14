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

	// ������Ȳ���

	glEnable(GL_DEPTH_TEST);

	// �����ͼ������ɫ��Դ����

	const GLchar* DepthMapVertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"
		"layout(location = 1) in vec2 texture_coord;\n"
		"out vec2 TextureCoord;\n"

		"void main() \n"
		"{\n"
			"TextureCoord = texture_coord;\n"
			"gl_Position = vec4(vertex_position, 1.0);\n"
		"}\n\0";

	// ������ɫ��

	GLint DepthMapVertexShader;
	DepthMapVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// ���붥����ɫ�����룬1ָ���˴��ݵ�Դ���ַ�������

	glShaderSource(DepthMapVertexShader, 1, &DepthMapVertexShaderSource, NULL);
	glCompileShader(DepthMapVertexShader);

	// �����ͼƬ����ɫ��(��������������ɫ���)Դ����

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

	// ������ɫ��

	GLint DepthMapFragmentShader;
	DepthMapFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// ����Ƭ����ɫ���Ĵ���

	glShaderSource(DepthMapFragmentShader, 1, &DepthMapFragmentShaderSource, NULL);
	glCompileShader(DepthMapFragmentShader);

	//����������Ƿ�ɹ�

	GLint status;
	glGetShaderiv(DepthMapVertexShader, GL_COMPILE_STATUS, &status);

	//��ɫ���������

	if (status != GL_TRUE) {
		cout <<"�����ͼ������ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(DepthMapVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(DepthMapFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "�����ͼƬ����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(DepthMapFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// ������ɫ������

	GLint depthMap_shader_programme;
	depthMap_shader_programme = glCreateProgram();

	// ������������ɫ��������������

	glAttachShader(depthMap_shader_programme, DepthMapVertexShader);
	glAttachShader(depthMap_shader_programme, DepthMapFragmentShader);
	glLinkProgram(depthMap_shader_programme);

	// ɾ����ɫ������

	glDeleteShader(DepthMapVertexShader);
	glDeleteShader(DepthMapFragmentShader);


	// ��ռ䶥����ɫ��Դ����

	const GLchar* LightSpaceVertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 vertex_position;\n"

		"uniform mat4 lightSpaceMat;\n"
		"uniform mat4 model;\n"

		"void main() \n"
		"{\n"
			"gl_Position = lightSpaceMat * model * vec4(vertex_position, 1.0);\n"
		"}\n\0";

	// ������ɫ��

	GLint LightSpaceVertexShader;
	LightSpaceVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// ���붥����ɫ�����룬1ָ���˴��ݵ�Դ���ַ�������

	glShaderSource(LightSpaceVertexShader, 1, &LightSpaceVertexShaderSource, NULL);
	glCompileShader(LightSpaceVertexShader);

	// ��ռ�Ƭ����ɫ��(��������������ɫ���)Դ����

	const GLchar* LightSpaceFragmentShaderSource = "#version 330 core\n"
		"void main()\n"
		"{\n"
			
		"}\n\0";

	// ������ɫ��

	GLint LightSpaceFragmentShader;
	LightSpaceFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// ����Ƭ����ɫ���Ĵ���

	glShaderSource(LightSpaceFragmentShader, 1, &LightSpaceFragmentShaderSource, NULL);
	glCompileShader(LightSpaceFragmentShader);

	//����������Ƿ�ɹ�

	glGetShaderiv(LightSpaceVertexShader, GL_COMPILE_STATUS, &status);

	//��ɫ���������

	if (status != GL_TRUE) {
		cout << "��ռ䶥����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(LightSpaceVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(LightSpaceFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "��ռ�Ƭ����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(LightSpaceFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// ������ɫ������

	GLint lightSpace_shader_programme;
	lightSpace_shader_programme = glCreateProgram();

	// ������������ɫ��������������

	glAttachShader(lightSpace_shader_programme, LightSpaceVertexShader);
	glAttachShader(lightSpace_shader_programme, LightSpaceFragmentShader);
	glLinkProgram(lightSpace_shader_programme);

	// ɾ����ɫ������

	glDeleteShader(LightSpaceVertexShader);
	glDeleteShader(LightSpaceFragmentShader);

	// ��Ӱӳ�䶥����ɫ��Դ����

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

	// ������ɫ��

	GLint ShadowMappingVertexShader;
	ShadowMappingVertexShader = glCreateShader(GL_VERTEX_SHADER);

	// ���붥����ɫ�����룬1ָ���˴��ݵ�Դ���ַ�������

	glShaderSource(ShadowMappingVertexShader, 1, &ShadowMappingVertexShaderSource, NULL);
	glCompileShader(ShadowMappingVertexShader);

	// ��Ӱӳ��Ƭ����ɫ��(��������������ɫ���)Դ����

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

	// ������ɫ��

	GLint ShadowMappingFragmentShader;
	ShadowMappingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// ����Ƭ����ɫ���Ĵ���

	glShaderSource(ShadowMappingFragmentShader, 1, &ShadowMappingFragmentShaderSource, NULL);
	glCompileShader(ShadowMappingFragmentShader);

	//����������Ƿ�ɹ�

	glGetShaderiv(ShadowMappingVertexShader, GL_COMPILE_STATUS, &status);

	//��ɫ���������

	if (status != GL_TRUE) {
		cout << "��Ӱӳ�䶥����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(ShadowMappingVertexShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}
	glGetShaderiv(ShadowMappingFragmentShader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		cout << "��Ӱӳ��Ƭ����ɫ������ʧ��\n";
		char szLog[1024] = { 0 };
		GLsizei logLen = 0;//ʵ�ʴ�����־����
		glGetShaderInfoLog(ShadowMappingFragmentShader, 1024, &logLen, szLog);
		printf("%s", szLog);
		//exit(EXIT_FAILURE);
	}

	// ������ɫ������

	GLint shadowMapping_shader_programme;
	shadowMapping_shader_programme = glCreateProgram();

	// ������������ɫ��������������

	glAttachShader(shadowMapping_shader_programme, ShadowMappingVertexShader);
	glAttachShader(shadowMapping_shader_programme, ShadowMappingFragmentShader);
	glLinkProgram(shadowMapping_shader_programme);

	// ɾ����ɫ������

	glDeleteShader(ShadowMappingVertexShader);
	glDeleteShader(ShadowMappingFragmentShader);

	float cubeVertexData[] = {
		// λ������			// ������				// ��������
		// ��
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // ����
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // ����
		1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // ����         
		1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // ����
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // ����
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // ����
		// ǰ
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // ����
		1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // ����
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // ����
		1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // ����
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // ����
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // ����
		// ��
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // ����
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // ����
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // ����
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // ����
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // ����
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // ����
		// ��
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // ����
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // ����
		1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // ����         
		1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // ����
		1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // ����
		1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // ����     
		// ��
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // ����
		1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // ����
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // ����
		1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // ����
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // ����
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // ����
		// ��
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // ����
		1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // ����
		1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // ����     
		1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // ����
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // ����
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // ����        
	};
	// �������㻺�����VBO

	GLuint cubeVBO = 0;
	glGenBuffers(1, &cubeVBO);

	// ���嶥���������VAO

	GLuint cubeVAO = 0;
	glGenVertexArrays(1, &cubeVAO);

	// �󶨶����������

	glBindVertexArray(cubeVAO);

	// �󶨶���λ�û������GL_ARRAY_BUFFER��VBO�Ļ�������

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);

	// ���������ݸ��Ƶ��󶨵�ǰ�󶨻��壬GL_STATIC_DRAW�����Կ�����������ݵķ�ʽ��GL_STATIC_DRAW��ʾ���ݼ�������ı�

	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertexData), cubeVertexData, GL_STATIC_DRAW);

	/*	���ö�����������
	��һ������Ϊ��������ʾӳ�䵽������ɫ���е��±꣨Ψһ��
	�ڶ�������Ϊ�����������������������Ϊ���������㣬������3
	����������Ϊ��������������
	���ĸ��������������Ƿ񱻱�׼����ָ��ֵӳ�䵽0-1�䣩
	���������Ϊ����(Stride)�����������Ķ���������֮��ļ�������������Ϊ3 * sizeof(float)��������������������֮��û�п�϶�����Կ�����Ϊ0����OpenGL�Զ�
	������������ʾ�����ڻ�������ʼλ�õ�ƫ����(Offset)
	*/

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));

	// ���ö�������ԣ�ʹ��ɫ��֪�����Ķ�����

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// ƽ�涥�㣨������������ϣ�

	float planeVertexData[] = {
		// λ������				// ������	        // ��������
		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};

	// �������㻺�����VBO

	GLuint planeVBO = 0;
	glGenBuffers(1, &planeVBO);

	// ���嶥���������VAO

	GLuint planeVAO = 0;
	glGenVertexArrays(1, &planeVAO);

	// �󶨶����������

	glBindVertexArray(planeVAO);

	// �󶨶���λ�û������GL_ARRAY_BUFFER��VBO�Ļ�������

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);

	// ���������ݸ��Ƶ��󶨵�ǰ�󶨻��壬GL_STATIC_DRAW�����Կ�����������ݵķ�ʽ��GL_STATIC_DRAW��ʾ���ݼ�������ı�

	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertexData), planeVertexData, GL_STATIC_DRAW);

	/*	���ö�����������
	��һ������Ϊ��������ʾӳ�䵽������ɫ���е��±꣨Ψһ��
	�ڶ�������Ϊ�����������������������Ϊ���������㣬������3
	����������Ϊ��������������
	���ĸ��������������Ƿ񱻱�׼����ָ��ֵӳ�䵽0-1�䣩
	���������Ϊ����(Stride)�����������Ķ���������֮��ļ�������������Ϊ3 * sizeof(float)��������������������֮��û�п�϶�����Կ�����Ϊ0����OpenGL�Զ�
	������������ʾ�����ڻ�������ʼλ�õ�ƫ����(Offset)
	*/

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0));
	
	// ���ö�������ԣ�ʹ��ɫ��֪�����Ķ�����

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	
	// ��������

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	//����Ŀ��Ϊ2D��ָ��S��T��Ϊ�����ᣬ���Ʒ�ʽΪ�ظ�����ͼ��

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// ���÷Ŵ���Сʱ������˷�ʽΪ���Թ���

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// ����ͼƬ

	int width, height, nrChannels;

	// ���� stb_image.h �����ᷭת��������.

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

	// Ϊ��Ⱦ�������ͼ����֡�������

	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// ����2D�����ṩ��֡�������Ȼ���ʹ��

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

	// ����߽���ɫ

	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// �����ɵ����������Ϊ֡�������Ȼ���

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
	static bool flag = true;
	// ��Ⱦѭ��

	while (!glfwWindowShouldClose(window))
	{
		// ��������
		processInput(window);

		// �����Ļ
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// �Ѷ���任����ռ�

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

		// ��Ⱦ�����ͼ
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		// ���������ͼ
		glm::mat4 model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(lightSpace_shader_programme, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// ���������ͼ
		model = glm::scale(model, glm::vec3(0.5f));
		glUniformMatrix4fv(glGetUniformLocation(lightSpace_shader_programme, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// ��Ⱦ����
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

		// ���泡����ͼ

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(shadowMapping_shader_programme, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// ���鳡����ͼ
		model = glm::scale(model, glm::vec3(0.5f));
		glUniformMatrix4fv(glGetUniformLocation(shadowMapping_shader_programme, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// ����GUI֡

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Setting");

		// ����GUI��С

		ImGui::SetWindowSize(ImVec2(300, 170));

		ImGui::SliderFloat("ambient", &ambientStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("diffuse", &diffuseStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("specular", &specularStrength, 0.0f, 1.0f);
		ImGui::SliderInt("shininess", &shininess, 1, 8);

		ImGui::RadioButton("Phong Shading", &s, 0);
		ImGui::RadioButton("LightSpace Shading", &s, 1);
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