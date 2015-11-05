#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>

#include <glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <opengl_tutorials_org/shader.hpp>
#include <opengl_tutorials_org/texture.hpp>
#include <opengl_tutorials_org/objloader.hpp>
#include <opengl_tutorials_org/text2D.hpp>

// vertex shader params
static GLuint vertexPosition_ModelID;
static GLuint vertexNormal_ModelID;
static GLuint vertexColourID;
static GLuint vertexTextureUVID;
static GLuint mvpID;
static GLuint modelMatrixID;
static GLuint viewMatrixID;
static GLuint lightPosition_WorldID;

// fragment shader params
static GLuint fragmentIsTextureID;
static GLuint textureSamplerID;
static GLuint lightColourID;
static GLuint lightPowerID;

// transformations
glm::mat4 projection;
glm::mat4 viewMatrix;

int loadObjAndTexture(const char *objPath, const char *texturePath,
    GLuint &indiceBuffer, GLuint &vertexBuffer, GLuint &uvBuffer,
    GLuint &normalBuffer, GLuint &texture)
{
    // An array of 3 vectors which represents 3 vertices
    // Read our .obj file
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    bool res = loadAssImp(objPath, indices, indexed_vertices, indexed_uvs, indexed_normals);

    if (!res)
    {
        return 0;
    }

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);;

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

    glGenBuffers(1, &indiceBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
    texture = loadDDS(texturePath);
    if (texture == 0)
    {
        return 0;
    }

    return indices.size();
}

void drawTexturedObj(glm::mat4 modelMatrix, unsigned int indices,
    GLuint indiceBuffer, GLuint vertexBuffer, GLuint uvBuffer,
    GLuint normalBuffer, GLuint texture)
{
    glEnableVertexAttribArray(vertexTextureUVID);

    // mvp matrix = model -> homogenous
    glm::mat4 mvp = projection * viewMatrix * modelMatrix;
    glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(textureSamplerID, 0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(vertexPosition_ModelID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glVertexAttribPointer(vertexTextureUVID, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glVertexAttribPointer(vertexNormal_ModelID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glUniform1f(fragmentIsTextureID, 1.0f);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer);
    glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_SHORT, (void *)0);

    glDisableVertexAttribArray(vertexTextureUVID);
}

int main(void)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


    // Open a window and create its OpenGL context
    window = glfwCreateWindow(800, 600, "Playground", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    GLuint mainProgramID = LoadShaders("shaders/mainVertexShader.vertexshader", "shaders/mainFragmentShader.fragmentshader");
    if (mainProgramID == 0)
    {
        return -1;
    }
    initText2D("textures/compressed/Holstein.DDS");

    // projection matrix = camera -> homogenous (3d -> 2d)
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    projection = glm::perspective(45.0f, (float)width / height, 0.1f, 100.0f);

    // view matrix = world -> camera
    viewMatrix = glm::lookAt(glm::vec3(0, 0, 5),    // where the camera is in world co-ordinates
        glm::vec3(0, 0, 0),    // target (direction = target - location)
        glm::vec3(0, 1, 0));   // which way is up

                               // lighting
    glm::vec3 lightPosition_World = glm::vec3(0, 0, 0);
    glm::vec3 lightColour = glm::vec3(0.3, 0.3, 1);

    // Get main shader parameters
    // vertex params (variable)
    vertexPosition_ModelID = glGetAttribLocation(mainProgramID, "vertexPosition_Model");
    vertexNormal_ModelID = glGetAttribLocation(mainProgramID, "vertexNormal_Model");
    vertexColourID = glGetAttribLocation(mainProgramID, "vertexColour");
    vertexTextureUVID = glGetAttribLocation(mainProgramID, "vertexTextureUV");

    // vertex params (static)
    mvpID = glGetUniformLocation(mainProgramID, "MVP");
    modelMatrixID = glGetUniformLocation(mainProgramID, "ModelMatrix");
    viewMatrixID = glGetUniformLocation(mainProgramID, "ViewMatrix");
    lightPosition_WorldID = glGetUniformLocation(mainProgramID, "lightPosition_World");

    // fragment params
    fragmentIsTextureID = glGetUniformLocation(mainProgramID, "fragmentIsTexture");
    textureSamplerID = glGetUniformLocation(mainProgramID, "textureSampler");
    lightColourID = glGetUniformLocation(mainProgramID, "lightColour");
    lightPowerID = glGetUniformLocation(mainProgramID, "lightPower");

    GLuint tyreIndiceBuffer;
    GLuint tyreVertexBuffer;
    GLuint tyreUvBuffer;
    GLuint tyreNormalBuffer;
    GLuint tyreTexture;
    int tyreIndices = loadObjAndTexture("models/obj/tyre.obj", "textures/compressed/tyre.DDS", tyreIndiceBuffer, tyreVertexBuffer, tyreUvBuffer, tyreNormalBuffer, tyreTexture);
    if (tyreIndices <= 0)
    {
        printf("Failed to load tyre object / texture\n");
        return -1;
    }
    // model matrix = model -> world
    glm::mat4 tyre_model = //glm::translate(glm::vec3(1.0f, 0.5f, 0.0f)) *
                           glm::rotate(glm::radians(-60.0f), glm::vec3(0, 1, 0)) *
                           glm::rotate(glm::radians(90.0f), glm::vec3(1.0f,0.0f,0.0f)) *
                           glm::mat4(1.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Enable blending
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double lastTime = glfwGetTime();
    unsigned int frameCount = 0;
    unsigned int frameRate = 0;
    do
    {
        frameCount++;
        if (glfwGetTime() - lastTime >= 0.5)
        {
            lastTime = glfwGetTime();
            frameRate = frameCount * 2;
            frameCount = 0;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(mainProgramID);

        // parameters that don't change per mesh (can change per frame)
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMatrix[0][0]);
        glUniform3fv(lightPosition_WorldID, 1, &lightPosition_World[0]);
        glUniform3fv(lightColourID, 1, &lightColour[0]);
        glUniform1f(lightPowerID, 50.0f);

        glEnableVertexAttribArray(vertexPosition_ModelID);
        glEnableVertexAttribArray(vertexNormal_ModelID);
        //glEnableVertexAttribArray(vertexColourID);

        // draw tyre
        tyre_model *= glm::rotate(glm::radians(-0.1f), glm::vec3(0, 1, 0));
        drawTexturedObj(tyre_model,
            tyreIndices,
            tyreIndiceBuffer,
            tyreVertexBuffer,
            tyreUvBuffer,
            tyreNormalBuffer,
            tyreTexture);

        //glDisableVertexAttribArray(vertexColourID);
        glDisableVertexAttribArray(vertexNormal_ModelID);
        glDisableVertexAttribArray(vertexPosition_ModelID);

        char textBuff[16];
        snprintf(textBuff, 16, "FR: %d\n", frameRate);
        printText2D(textBuff, 10, 560, 35);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);


    // Cleanup VBO
    glDeleteBuffers(1, &tyreVertexBuffer);
    glDeleteBuffers(1, &tyreUvBuffer);
    glDeleteBuffers(1, &tyreNormalBuffer);
    glDeleteProgram(mainProgramID);
    glDeleteTextures(1, &textureSamplerID);
    cleanupText2D();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

