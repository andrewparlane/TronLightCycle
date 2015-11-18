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
#include <bike.hpp>

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
            !mainShader->addUniformID("lightAmbientColour", SHADER_UNIFORM_LIGHT_AMBIENT_COLOUR) ||
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

ObjData *createArena()
{
    ObjData *arenaObjData = new ObjData();
    MeshData md;
    md.name = "arena";
    md.hasTexture = true;
    md.texturePath = "arena.DDS";

    // it's the floor, so all normals are the same
    // we can use indexing so only need to specify unique vertices
    // we want the texture to repeat per square, so just use vertex numbers for uvs

    const unsigned int STRETCH_FACTOR = 50;
    const unsigned int NUM_X = 11;
    const unsigned int NUM_Z = 11;

    for (int x = 0; x < NUM_X; x++)
    {
        for (int z = 0; z < NUM_Z; z++)
        {
            md.vertices.push_back(glm::vec3((x - ((NUM_X - 1.0f) / 2.0f)) * STRETCH_FACTOR,
                                            0.0f,
                                            (z - ((NUM_Z - 1.0f) / 2.0f)) * STRETCH_FACTOR));
            md.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
            md.uvs.push_back(glm::vec2(x, z));
        }
    }

    // now to create indices
    // we have (NUM_X - 1) * (NUM_Z - 1) squares
    // go through each square
    for (unsigned short x = 0; x < (NUM_X - 1); x++)
    {
        for (unsigned short z = 0; z < (NUM_Z - 1); z++)
        {
            // 1---2
            // |   |
            // 0---3
            unsigned short corners[4] = { (x * NUM_X) + z,
                                          (x * NUM_X) + z + 1,
                                          ((x + 1) * NUM_X) + z + 1,
                                          ((x + 1) * NUM_X) + z };

            // each square consists of two triangular faces:
            // 0,1,3 and 1,2,3
            md.indices.push_back(corners[0]);
            md.indices.push_back(corners[1]);
            md.indices.push_back(corners[3]);

            md.indices.push_back(corners[1]);
            md.indices.push_back(corners[2]);
            md.indices.push_back(corners[3]);
        }
    }

    arenaObjData->addMesh(md);
    if (!arenaObjData->createBuffers())
    {
        delete arenaObjData;
        arenaObjData = NULL;
    }
    return arenaObjData;
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
    world->setProjection(glm::perspective(45.0f, (float)width / height, 0.1f, 1000.0f));

    // view matrix = world -> camera
    const float distanceBetweenBikeAndCamera = 18.0f;
    world->setCamera(glm::lookAt(glm::vec3(0, 8, distanceBetweenBikeAndCamera),    // where the camera is in world co-ordinates
                                 glm::vec3(0, 6, 0),    // target (direction = target - location)
                                 glm::vec3(0, 1, 0)));  // which way is up

    // lighting
    world->setLight(glm::vec3(0, 2, 4),             // position
                    glm::vec3(0.6f, 0.6f, 1.0f),    // colour
                    50.0f,                          // power
                    glm::vec3(0.6f, 0.6f, 0.6f));   // ambient

    

    std::shared_ptr<ObjLoader> bikeLoader(new ObjLoader("models/obj/bike.obj", "models/obj/bike.tex"));
    if (!bikeLoader->loadTextureMap() ||
        !bikeLoader->loadObj() ||
        !bikeLoader->createBuffers())
    {
        printf("Failed to load object / texture\n");
        system("pause");
        return -1;
    }

    // get the lowest point of the bike, so we can move it so the wheels rest on the floor
    BoundingBox bikeBB = bikeLoader->getBoundingBox();
    float lowest = FLT_MAX;
    for (unsigned int i = 0; i < 8; i++)
    {
        if (bikeBB.vertices[i].y < lowest) lowest = bikeBB.vertices[i].y;
    }

    // model matrix = model -> world
    glm::mat4 bike_model = glm::translate(glm::vec3(0.0f, -lowest, 0.0f)) *
        //glm::rotate(glm::radians(-60.0f), glm::vec3(0, 1, 0)) *
        //glm::rotate(glm::radians(90.0f), glm::vec3(0.0f,1.0f,0.0f)) *
        glm::mat4(1.0f);

    Bike bike(bikeLoader, world, mainShader, bike_model);
    bike.setDefaultColour(tronBlue);

    std::shared_ptr<ObjData> arenaObjData(createArena());
    if (!arenaObjData)
    {
        printf("Failed to create arena obj data\n");
        system("pause");
        return -1;
    }
    Object arena(arenaObjData, world, mainShader, glm::mat4(1.0f));

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

        // always moving forwards
        bike.translate(glm::vec3(0,0,-0.1f));

        // deal with keyboard input
        if (glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS)
        {
            bike.rotate(glm::radians(-0.5f), glm::vec3(0,1,0));
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS)
        {
            bike.rotate(glm::radians(0.5f), glm::vec3(0,1,0));
        }

        // update camera location
        glm::vec3 bikeLocation = bike.getWorldLocation();
        float bikeAngleRads = bike.getRotationRads();
        glm::vec3 cameraPosition = glm::vec3(bikeLocation.x + distanceBetweenBikeAndCamera * sin(bikeAngleRads),
                                             8,
                                             bikeLocation.z + distanceBetweenBikeAndCamera * cos(bikeAngleRads));
        glm::vec3 cameraDirection = glm::vec3(bikeLocation.x, 6, bikeLocation.z);

        world->setCamera(glm::lookAt(cameraPosition,       // where the camera is in world co-ordinates
                                     cameraDirection,      // target (direction = target - location)
                                     glm::vec3(0, 1, 0)));  // which way is up
        
        // draw bike
        bike.drawAll();

        // draw arena
        arena.drawAll();

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

