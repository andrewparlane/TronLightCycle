#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <GL/glew.h>

#include <glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <objloader.hpp>
#include <object.hpp>
#include <world.hpp>
#include <shader.hpp>

#include <opengl_tutorials_org/texture.hpp>
#include <opengl_tutorials_org/text2D.hpp>

// colours
const glm::vec3 tronBlue = glm::vec3(0.184f, 1.0f, 1.0f);

Shader *setupMainShader()
{
    // first init main shader
    Shader *mainShader = new Shader("shaders/mainVertexShader.vertexshader", "shaders/mainFragmentShader.fragmentshader");
    if (!mainShader->compile())
    {
        printf("Failed to compile main shader\n");
        delete mainShader;
        return NULL;
    }
    else
    {
        // Get main shader parameters
        if (// vertex params (variable)
            !mainShader->addAttribID("vertexPosition_Model", SHADER_ATTRIB_VECTOR_POS) ||
            !mainShader->addAttribID("vertexNormal_Model", SHADER_ATTRIB_VECTOR_NORMAL) ||
            !mainShader->addAttribID("vertexTextureUV", SHADER_ATTRIB_VECTOR_UV) ||
            // vertex params (static)
            !mainShader->addUniformID("MVP", SHADER_UNIFORM_MVP) ||
            !mainShader->addUniformID("ModelMatrix", SHADER_UNIFORM_MODEL_MATRIX) ||
            !mainShader->addUniformID("ViewMatrix", SHADER_UNIFORM_VIEW_MATRIX) ||
            !mainShader->addUniformID("lightPosition_World", SHADER_UNIFORM_LIGHT_POS_WORLD) ||
            // fragment params
            !mainShader->addUniformID("lightColour", SHADER_UNIFORM_LIGHT_COLOUR) ||
            !mainShader->addUniformID("lightPower", SHADER_UNIFORM_LIGHT_POWER) ||
            !mainShader->addUniformID("fragmentIsTexture", SHADER_UNIFORM_IS_TEXTURE) ||
            !mainShader->addUniformID("textureSampler", SHADER_UNIFORM_TEXTURE_SAMPLER) ||
            !mainShader->addUniformID("fragmentColour", SHADER_UNIFORM_FRAGMENT_COLOUR))
        {
            printf("Error adding main shader IDs\n");
            delete mainShader;
            return NULL;
        }
    }

    return mainShader;
}

int main(void)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        system("pause");
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
        system("pause");
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        system("pause");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    std::shared_ptr<const Shader> mainShader;
    mainShader.reset(setupMainShader());
    if (!mainShader)
    {
        system("pause");
        return -1;
    }
    if (!initText2D("textures/compressed/Holstein.DDS"))
    {
        system("pause");
        return -1;
    }

    // projection matrix = camera -> homogenous (3d -> 2d)
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    std::shared_ptr<World> world;
    world.reset(new World);
    world->setProjection(glm::perspective(45.0f, (float)width / height, 0.1f, 100.0f));

    // view matrix = world -> camera
    world->setCamera(glm::lookAt(glm::vec3(0, 0, 5),    // where the camera is in world co-ordinates
                                 glm::vec3(0, 0, 0),    // target (direction = target - location)
                                 glm::vec3(0, 1, 0)));  // which way is up

    // lighting
    world->setLight(glm::vec3(0, 2, 4),             // position
                    glm::vec3(0.6f, 0.6f, 1.0f),    // colour
                    50.0f);                         // power

    

    std::shared_ptr<ObjLoader> bikeLoader(new ObjLoader("models/obj/bike.obj"));
    if (!bikeLoader->loadObj())
    {
        printf("Failed to load object / texture\n");
        system("pause");
        return -1;
    }

    // model matrix = model -> world
    glm::mat4 bike_model = glm::translate(glm::vec3(0.0f, 0.0f, -5.0f)) *
        //glm::rotate(glm::radians(-60.0f), glm::vec3(0, 1, 0)) *
        glm::rotate(glm::radians(90.0f), glm::vec3(0.0f,1.0f,0.0f)) *
        glm::mat4(1.0f);

    Object bike(bikeLoader, world, mainShader, bike_model);
    bike.setDefaultColour(tronBlue);

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
        
        // draw bike
        //bike_model *= glm::rotate(glm::radians(-0.1f), glm::vec3(0, 1, 0));
        bike.drawAll();

        char textBuff[16];
        snprintf(textBuff, 16, "FR: %d\n", frameRate);
        printText2D(textBuff, 10, 560, 35);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);


    // Cleanup    
    cleanupText2D();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

