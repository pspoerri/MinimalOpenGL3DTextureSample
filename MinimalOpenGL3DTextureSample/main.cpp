// GLFW Setup
#define GLFW_INCLUDE_GLCOREARB
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h> // Includes OpenGL
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <exception>

bool closeWindow = false;

void
glfwWindowCloseCallback(GLFWwindow* window)
{
    closeWindow = true;
}
void
glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        closeWindow = true;
    }
}
void
glCheckError()
{
    GLenum error = glGetError();
    if (error == GL_NO_ERROR)
    {
        return;
    }
    std::stringstream stream;
    const char* charstr = (const char*)glGetString(error);
    if (charstr != NULL)
    {
        stream << charstr;
    }
    else
    {
        stream << "Code " << std::to_string(error);
    }
    std::cerr << "Encountered an OpenGL Error: " << stream.str() << std::endl;
    throw std::exception();
}

void
initShader();
void
render();

GLuint m_shaderProgram;
GLuint m_vertexbufferID;
GLuint m_vertexArrayID;
GLuint m_textureBufferID;


std::vector<float> m_vertexBufferData = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f,  1.0f, 0.0f,
};

int
main()
{
    // GLFW Init and Core Profile Setup
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW. Aborting." << std::endl;
        return EXIT_FAILURE;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    auto width = 768;
    auto height = 768;
    // Create a window
    auto window = glfwCreateWindow(width, height, "Testapp", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cerr << "Failed to create window" << std::endl;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetWindowCloseCallback(window, glfwWindowCloseCallback);
    
    std::cout << "OpenGLVersion" << glGetString(GL_VERSION) << std::endl;

    initShader();
    while (!closeWindow)
    {
        // Render
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}


// Create some volume data
std::vector<float> m_volumeData(16*256*256,1.0);

void
initTexture()
{
    glGenTextures(1, &m_textureBufferID);
    glBindTexture(GL_TEXTURE_3D, m_textureBufferID);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D,
                 0,
                 GL_R32F,
                 16, 256, 256, 0,
                 GL_RED, GL_FLOAT, &m_volumeData[0]);
    glCheckError();
    
    // Set texture unit
    glUseProgram(m_shaderProgram);
    GLuint loc = glGetUniformLocation(m_shaderProgram, "uVolume");
    glActiveTexture(GL_TEXTURE0+0);
    glBindTexture(GL_TEXTURE_3D, m_textureBufferID);
    glUniform1i(loc, 0);
    glUseProgram(0);
    glCheckError();
}

void
initShader()
{
    GLint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    const std::string vertexShader =
        "#version 410 core\n"
        "in vec3 inPosition;\n"
        "void main(void)\n"
        "{\n"
        "   gl_Position = vec4(inPosition, 1);\n"
        "}\n";
    const std::string fragmentShader =
        "#version 410 core\n"
        "out vec4 color;\n"
        "uniform sampler3D uVolume;\n"
        "void main(void)\n"
        "{\n"
        "    vec3 sampleColor = vec3(1,1,1);\n"
        "    vec3 pos = vec3(0.5,0.5,0.5);\n"
        "    float intens = texture(uVolume, pos).x;\n"
        "    color = vec4(sampleColor*intens,1);\n"
//        "    color = vec4(sampleColor, 1);\n"
        "}\n";
    
    GLint status;
    
    const char* vShaderPtr = vertexShader.c_str();
    glShaderSource(vertexShaderID, 1, &vShaderPtr, NULL);
    glCompileShader(vertexShaderID);
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        throw std::exception();
    }
    glCheckError();

    const char* fShaderPtr = fragmentShader.c_str();
    glShaderSource(fragmentShaderID, 1, &fShaderPtr, NULL);
    glCompileShader(fragmentShaderID);
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        throw std::exception();
    }
    glCheckError();

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShaderID);
    glAttachShader(m_shaderProgram, fragmentShaderID);
    glLinkProgram(m_shaderProgram);
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        throw std::exception();
    }
    glCheckError();

    
    glGenVertexArrays(1, &m_vertexArrayID);
    glBindVertexArray(m_vertexArrayID);
    glCheckError();
    
    glGenBuffers(1, &m_vertexbufferID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexbufferID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexbufferID);
    glBufferData(GL_ARRAY_BUFFER, m_vertexBufferData.size()*sizeof(float), &m_vertexBufferData[0], GL_STATIC_DRAW);
    glCheckError();

    initTexture();
    
    glUseProgram(m_shaderProgram);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexbufferID);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glUseProgram(0);
    glCheckError();
}

void
render()
{
    glUseProgram(m_shaderProgram);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glCheckError();

    glUseProgram(0);
}