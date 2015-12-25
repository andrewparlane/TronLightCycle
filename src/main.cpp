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
#include <bike_movements.hpp>
#include <two_dimensional.hpp>

#include <opengl_tutorials_org/texture.hpp>

#define SPEED_BAR_START_X 602.0f
#define SPEED_BAR_END_X 778.0f

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
            !mainShader->addAttribID("vertexPosition_Model", SHADER_ATTRIB_VERTEX_POS) ||
            !mainShader->addAttribID("vertexNormal_Model", SHADER_ATTRIB_VERTEX_NORMAL) ||
            !mainShader->addAttribID("vertexTextureUV", SHADER_ATTRIB_VERTEX_UV) ||
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

Shader *setup2DShader()
{
    // initialise text shader
    Shader *shader2D = new Shader( "shaders/2DVertexShader.vertexshader", "shaders/2DFragmentShader.fragmentshader" );
    if (!shader2D->compile())
    {
        printf("Failed to compile 2D shader\n");
        return NULL;
    }
    else
    {
        // Get a handle for our buffers
        if (!shader2D->addAttribID("vertexPosition_screenspace", SHADER_ATTRIB_VERTEX_POS_SCREEN) ||
            !shader2D->addAttribID("vertexUV", SHADER_ATTRIB_VERTEX_UV) ||
            !shader2D->addAttribID("vertexColour", SHADER_ATTRIB_VERTEX_COLOUR) ||
            !shader2D->addUniformID("myTextureSampler", SHADER_UNIFORM_TEXTURE_SAMPLER) ||
            !shader2D->addUniformID("fragmentIsTexture", SHADER_UNIFORM_IS_TEXTURE))
        {
            printf("Failed to add 2D shader IDs\n");
            return NULL;
        }
    }

    return shader2D;
}

ObjData3D *createArena()
{
    ObjData3D *arenaObjData = new ObjData3D();
    MeshData<glm::vec3> md;
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

    if (!arenaObjData->addMesh(md))
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
    std::shared_ptr<const Shader> shader2D;
    shader2D.reset(setup2DShader());
    if (!shader2D)
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
    float distanceBetweenBikeAndCamera = 18.0f;
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
        !bikeLoader->loadObj())
    {
        printf("Failed to load object / texture\n");
        system("pause");
        return -1;
    }

    // get the lowest point of the bike, so we can move it so the wheels rest on the floor
    // and the most forward point, so we can use it for collision detection
    BoundingBox<glm::vec3> bikeBB = bikeLoader->getBoundingBox();
    float bike_lowest = FLT_MAX;
    float bike_most_forward = FLT_MAX;
    for (unsigned int i = 0; i < 8; i++)
    {
        if (bikeBB.vertices[i].y < bike_lowest)       bike_lowest =       bikeBB.vertices[i].y;
        if (bikeBB.vertices[i].z < bike_most_forward) bike_most_forward = bikeBB.vertices[i].z;
    }

    // model matrix = model -> world
    glm::mat4 bike_model = glm::translate(glm::vec3(0.0f, -bike_lowest, 0.0f));

    // Load bike
    Bike bike(bikeLoader, world, mainShader, bike_model, tronBlue);

    // create arena
    std::shared_ptr<ObjData3D> arenaObjData(createArena());
    if (!arenaObjData)
    {
        printf("Failed to create arena obj data\n");
        system("pause");
        return -1;
    }
    Object arena(arenaObjData, world, mainShader, glm::mat4(1.0f));

    // load default font
    unsigned int defaultFont = loadDDS(std::string("textures/compressed/Holstein.DDS"));
    if (defaultFont == 0)
    {
        printf("Couldn't loadDDS font texture\n");
        system("pause");
        return false;
    }

    // create debug text object
    Text text(shader2D);

    // create speed bar
    Shape2D speedBar(shader2D, glm::vec3(1.0f, 1.0f, 1.0f));
    speedBar.addRect(glm::vec2(SPEED_BAR_START_X-2.0f,580), glm::vec2(SPEED_BAR_END_X+2,580), glm::vec2(SPEED_BAR_END_X+2,560), glm::vec2(SPEED_BAR_START_X-2.0f,560),
                     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    speedBar.addRect(glm::vec2(SPEED_BAR_START_X,578), glm::vec2(SPEED_BAR_END_X,578), glm::vec2(SPEED_BAR_END_X,562), glm::vec2(SPEED_BAR_START_X,562),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));

    Text speedBarText(shader2D);
    speedBarText.addText2D("speed", (unsigned int)(SPEED_BAR_START_X - 156), 560, 26, defaultFont);

    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Enable blending
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // frame rate reporting
    double lastFrameCountUpdateTime = glfwGetTime();
    unsigned int frameCount = 0;
    unsigned int frameRate = 0;
    double timeSpentBusy = 0;
    unsigned int displayedMaxPossibleFrameRate = 0;

    // frame rate limiting
    const double maxFrameRatelimit = 60.0;
    double frameRateLimit = maxFrameRatelimit;
    double minTimeBetweenFrames = 1.0/frameRateLimit;
    double lastFrameStartTime = glfwGetTime();

    // camera rotation
    float cameraRotationDegrees = 0.0f;
    bool cameraRotating = false;

    // key flags
    bool cKeyPressed = false;           // camera rotation
    bool sKeyPressed = false;           // stop moving the bike
    bool spaceKeyPressed = false;       // toggle light trail
#ifdef DEBUG
    bool f5KeyPressed = false;          // quick save
    bool f9KeyPressed = false;          // quick load
#endif

    // misc
    float lastSpeed = 0.0f;

    // debug stuff
    bool stop = false;                          // stop moving the bike with the 's' key
#ifdef DEBUG
    bool stateIsSaved = false;
    float savedCameraRotationDegrees;
    bool savedCameraRotating;
    bool savedStop;
#endif
    do
    {
        // frame rate limiting =====================================================
        while ((glfwGetTime() - lastFrameStartTime) < minTimeBetweenFrames);
        lastFrameStartTime = glfwGetTime();

        // frame rate reporting ====================================================
        frameCount++;
        // update displayed frame rate evry second
        if ((glfwGetTime() - lastFrameCountUpdateTime) >= 1.0)
        {
            lastFrameCountUpdateTime = glfwGetTime();
            frameRate = frameCount;

            // in the last second we spent "timeSpentBusy" seconds actually donig stuff
            // to display "frameCount" frames.
            // so we spent timeSpentBusy / frameCount seconds per frame (on average)
            // so max possible frame rate would be 1 / that, so just swap the order
            displayedMaxPossibleFrameRate = (unsigned int)(frameCount / timeSpentBusy);

            timeSpentBusy = 0;
            frameCount = 0;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // deal with keyboard input =====================================

        TurnDirection turnDirection = NO_TURN;
        Accelerating accelerating = SPEED_NORMAL;

        // bike turning right
        if (glfwGetKey(window, GLFW_KEY_RIGHT ))
        {
            turnDirection = TURN_RIGHT;
        }
        // bike turning left
        else if (glfwGetKey(window, GLFW_KEY_LEFT ))
        {
            turnDirection = TURN_LEFT;
        }

        // Light trail toggle
        if (glfwGetKey(window, GLFW_KEY_SPACE ))
        {
            spaceKeyPressed = 1;
        }
        if (spaceKeyPressed && !glfwGetKey(window, GLFW_KEY_SPACE ))
        {
            spaceKeyPressed = 0;
            bike.toggleLightTrail();
        }

        // camera rotating?
        // first detect c pressed, then wait for release
        if (glfwGetKey(window, GLFW_KEY_C ))
        {
            cKeyPressed = 1;
        }
        if (cKeyPressed && !glfwGetKey(window, GLFW_KEY_C ))
        {
            cKeyPressed = 0;
            cameraRotating = !cameraRotating;
        }

        // zoom camera in or out using left shift and control keys
        // TODO: Add mouse wheel support
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
        {
            if (distanceBetweenBikeAndCamera > 8.0f)
            {
                distanceBetweenBikeAndCamera -= 0.2f;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL))
        {
            if (distanceBetweenBikeAndCamera < 30.0f)
            {
                distanceBetweenBikeAndCamera += 0.2f;
            }
        }

        // bike speed using up and down arrow keys
        if (glfwGetKey(window, GLFW_KEY_UP))
        {
            accelerating = SPEED_ACCELERATE;
        }
        else if (glfwGetKey(window, GLFW_KEY_DOWN))
        {
            accelerating = SPEED_BRAKE;
        }

#ifdef DEBUG
        // Stop moving the bike for debug purposes
        if (glfwGetKey(window, GLFW_KEY_S ))
        {
            sKeyPressed = 1;
        }
        if (sKeyPressed && !glfwGetKey(window, GLFW_KEY_S ))
        {
            sKeyPressed = 0;
            stop = !stop;
        }

        // change frame rate for debug purposes
        if (glfwGetKey(window, GLFW_KEY_EQUAL))
        {
            frameRateLimit++;
            if (frameRateLimit > maxFrameRatelimit)
            {
                frameRateLimit = maxFrameRatelimit;
            }
            minTimeBetweenFrames = 1.0/frameRateLimit;
        }
        else if (glfwGetKey(window, GLFW_KEY_MINUS))
        {
            frameRateLimit--;
            if (frameRateLimit < 1)
            {
                frameRateLimit = 1;
            }
            minTimeBetweenFrames = 1.0/frameRateLimit;
        }

        // quick save and quick load of bike position
        // TODO: expand to cover light trails too
        if (glfwGetKey(window, GLFW_KEY_F5 ))
        {
            f5KeyPressed = 1;
        }
        if (f5KeyPressed && !glfwGetKey(window, GLFW_KEY_F5 ))
        {
            f5KeyPressed = 0;
            bike.saveBikeState();
            stateIsSaved = true;
            savedCameraRotationDegrees = cameraRotationDegrees;
            savedCameraRotating = cameraRotating;
            savedStop = stop;
        }
        if (glfwGetKey(window, GLFW_KEY_F9 ))
        {
            f9KeyPressed = 1;
        }
        if (f9KeyPressed && !glfwGetKey(window, GLFW_KEY_F9 ))
        {
            f9KeyPressed = 0;
            bike.restoreBikeState();
            if (stateIsSaved)
            {
                cameraRotationDegrees = savedCameraRotationDegrees;
                cameraRotating = savedCameraRotating;
                stop = savedStop;
            }
        }
#endif

        // update the bike and light trails
        bike.update(turnDirection, accelerating, stop);

        // check for collisions
        // only with it's own trail ATM, as there are no more

        // transform co-ords of front of bike to world co-ords.
        glm::vec3 bikeFrontLocation = bike.applyModelMatrx(glm::vec3(0,0,bike_most_forward));

        const LightTrailManager &tm = bike.getTrailManager();
        if (tm.collides(glm::vec2(bikeFrontLocation.x, bikeFrontLocation.z)) ||
            bike.checkSelfCollision())
        {
            // TODO indicate collision occured
            stop = true;
            cameraRotating = true;
        }

        // update camera location =============================================
        // transform origin of bike to world co-ords.
        glm::vec3 bikeLocation = bike.applyModelMatrx(glm::vec3(0,0,bike_most_forward));

        // we want the camera to be distanceBetweenBikeAndCamera from the bike.
        // by default it should be directly behind the bike so we can see where we are going
        // however I also want a mode where the camera rotates around so you can see the bike
        // so take a vector where the camera is directly behind the bike, in bike model space
        // and rotate it, this rotates the point around the bike.
        glm::vec3 cameraOffsetFromBike = glm::vec3(glm::vec4(0,0,distanceBetweenBikeAndCamera,1) *
                                                   glm::rotate(glm::radians(cameraRotationDegrees), glm::vec3(0,1,0)));
        // transform the camera position into world co-ordinates
        // note: this applies rotations, translations and scaling + any other transform
        //       so it won't work correctly if you scale your model in the Z direction
        //       your camera position will be scaled too
        glm::vec3 cameraPosition = bike.applyModelMatrx(cameraOffsetFromBike);
        // We want the y co-ord to be a bit above the bike
        cameraPosition.y = 8.0f;
        // point the camera at the bike, but adjust the y so we aren't looking too much down
        glm::vec3 cameraDirection = glm::vec3(bikeLocation.x, 6, bikeLocation.z);

        world->setCamera(glm::lookAt(cameraPosition,       // where the camera is in world co-ordinates
                                     cameraDirection,      // target (direction = target - location)
                                     glm::vec3(0, 1, 0)));  // which way is up
        
        // if the camera is rotating around the bike, then update the angle
        if (cameraRotating)
        {
            cameraRotationDegrees += 0.4f;
        }

        // Draw 3D objects draw only what's in front ================================
        glEnable(GL_DEPTH_TEST);

        // draw bike
        bike.drawAll();

        // draw arena
        arena.drawAll();

        // Draw 2D objects in order they are listed =================================
        glDisable(GL_DEPTH_TEST);

        // draw speed bar, only change the bar if the value has changed
        float speedPercent = bike.getSpeedPercent();
        if (abs(lastSpeed - speedPercent) > 0.01f)
        {
            lastSpeed = speedPercent;
            float end_x = SPEED_BAR_START_X + (SPEED_BAR_END_X - SPEED_BAR_START_X) * speedPercent;
            speedBar.deleteObjData("speed_bar");
            speedBar.addRect(glm::vec2(SPEED_BAR_START_X,578), glm::vec2(end_x,578), glm::vec2(end_x,562), glm::vec2(SPEED_BAR_START_X,562),
                             glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(speedPercent, 1.0f - speedPercent, 0.0f), glm::vec3(speedPercent, 1.0f - speedPercent, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                             "speed_bar");
        }
        speedBar.drawAll();
        speedBarText.drawAll();

        // output debug stats
        if (frameCount == 0)
        {
            char textBuff[32];
            text.deleteAllObjData();

            snprintf(textBuff, 32, "FR (Actual): %d\n", frameRate);
            text.addText2D(textBuff, 10, 560, 26, defaultFont);

            snprintf(textBuff, 32, "      (MAX): %d\n", displayedMaxPossibleFrameRate);
            text.addText2D(textBuff, 10, 530, 26, defaultFont);
        }
        text.drawAll();

        // Swap buffers ========================================================
        glfwSwapBuffers(window);

        // Check for key presses ===============================================
        glfwPollEvents();

        // finished this frame, update timeSpentBusy ===========================
        timeSpentBusy += glfwGetTime() - lastFrameStartTime;

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);


    // Cleanup
    // delete default font
    glDeleteTextures(1, &defaultFont);
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

