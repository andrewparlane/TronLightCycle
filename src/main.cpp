#include "objloader.hpp"
#include "object.hpp"
#include "world.hpp"
#include "shader.hpp"
#include "bike.hpp"
#include "bike_movements.hpp"
#include "two_dimensional.hpp"
#include "progress_bar.hpp"
#include "texture.hpp"
#include "light_trail_manager.hpp"
#include "render_pipeline.hpp"

#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
#include "light_trail_segment.hpp"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <limits.h>

#include <GL/glew.h>

#include <glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

#define SPEED_BAR_START_X 602.0f
#define SPEED_BAR_END_X 778.0f

#define ARENA_STRETCH_FACTOR 25
#define ARENA_NUM_X 11
#define ARENA_NUM_Z 11

// colours
const glm::vec3 tronBlue = glm::vec3(0.184f, 1.0f, 1.0f);

std::shared_ptr<ObjData3D> createArena()
{
    std::shared_ptr<ObjData3D> arenaObjData = std::make_shared<ObjData3D>();
    MeshData<glm::vec3> md;
    md.name = "arena";
    md.hasTexture = true;
    md.texturePath = "arena.DDS";

    // it's the floor, so all normals are the same
    // we can use indexing so only need to specify unique vertices
    // we want the texture to repeat per square, so just use vertex numbers for uvs

    for (int x = 0; x < ARENA_NUM_X; x++)
    {
        for (int z = 0; z < ARENA_NUM_Z; z++)
        {
            md.vertices.push_back(glm::vec3((x - ((ARENA_NUM_X - 1.0f) / 2.0f)) * ARENA_STRETCH_FACTOR,
                                            0.0f,
                                            -(z * ARENA_STRETCH_FACTOR)));
            md.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
            md.uvs.push_back(glm::vec2(x, z));
        }
    }

    // now to create indices
    // we have (ARENA_NUM_X - 1) * (ARENA_NUM_Z - 1) squares
    // go through each square
    for (unsigned short x = 0; x < (ARENA_NUM_X - 1); x++)
    {
        for (unsigned short z = 0; z < (ARENA_NUM_Z - 1); z++)
        {
            // 1---2
            // |   |
            // 0---3
            unsigned short corners[4] = { (unsigned short)((x * ARENA_NUM_X) + z),
                                          (unsigned short)((x * ARENA_NUM_X) + z + 1),
                                          (unsigned short)(((x + 1) * ARENA_NUM_X) + z + 1),
                                          (unsigned short)(((x + 1) * ARENA_NUM_X) + z )};

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
        arenaObjData = NULL;
    }

    return arenaObjData;
}

std::shared_ptr<ObjData3D> createLamp()
{
    std::shared_ptr<ObjData3D> objData = std::make_shared<ObjData3D>();
    MeshData<glm::vec3> md;
    md.name = "lamp";
    md.hasTexture = false;

    md.vertices.push_back(glm::vec3(-1.0f, -1.0f,  1.0f));  // front bottom left
    md.vertices.push_back(glm::vec3(-1.0f,  1.0f,  1.0f));  // front top left
    md.vertices.push_back(glm::vec3( 1.0f,  1.0f,  1.0f));  // front top right
    md.vertices.push_back(glm::vec3( 1.0f, -1.0f,  1.0f));  // front bottom right

    md.vertices.push_back(glm::vec3(-1.0f, -1.0f, -1.0f));  // back bottom left
    md.vertices.push_back(glm::vec3(-1.0f,  1.0f, -1.0f));  // back top left
    md.vertices.push_back(glm::vec3( 1.0f,  1.0f, -1.0f));  // back top right
    md.vertices.push_back(glm::vec3( 1.0f, -1.0f, -1.0f));  // back bottom right

    // front
    md.indices.push_back(0); md.indices.push_back(1); md.indices.push_back(3);
    md.indices.push_back(1); md.indices.push_back(2); md.indices.push_back(3);

    // left
    md.indices.push_back(4); md.indices.push_back(5); md.indices.push_back(0);
    md.indices.push_back(5); md.indices.push_back(1); md.indices.push_back(0);

    // right
    md.indices.push_back(3); md.indices.push_back(2); md.indices.push_back(7);
    md.indices.push_back(2); md.indices.push_back(6); md.indices.push_back(7);

    // back
    md.indices.push_back(7); md.indices.push_back(6); md.indices.push_back(4);
    md.indices.push_back(6); md.indices.push_back(5); md.indices.push_back(4);

    // bottom
    md.indices.push_back(4); md.indices.push_back(0); md.indices.push_back(7);
    md.indices.push_back(0); md.indices.push_back(3); md.indices.push_back(7);

    // top
    md.indices.push_back(1); md.indices.push_back(5); md.indices.push_back(2);
    md.indices.push_back(5); md.indices.push_back(6); md.indices.push_back(2);

    if (!objData->addMesh(md))
    {
        objData = NULL;
    }
    return objData;
}

bool setupArenaLighting(std::shared_ptr<World> world)
{
    std::shared_ptr<const Shader> shader = Shader::getShader(SHADER_TYPE_LAMP);

    // lighting
    std::shared_ptr<ObjData3D> lampObjData(createLamp());
    if (!lampObjData)
    {
        printf("Failed to create lamp obj data\n");
        return false;
    }

    std::shared_ptr<ObjLoader> deferredShadingObj = std::make_shared<ObjLoader>("models/obj/sphere.obj", "");
    if (!deferredShadingObj ||
        !deferredShadingObj->loadObj())
    {
        printf("Error loading sphere\n");
        return false;
    }

    const float lightRadius = 15.0f;
    const float lightAmbient = 0.2f;
    const float lightDiffuse = 0.5f;
    const float lightSpecular = 1.0f;
    const float lampHeight = 20.0f;

    glm::mat4 lamp_model_without_position = glm::scale(glm::vec3(5.0f, 0.2f, 0.2f));

    float zPosFurtherst = -((ARENA_NUM_Z - 1.0f) * ARENA_STRETCH_FACTOR);
    for (unsigned int x = 0; x < (ARENA_NUM_X - 1.0f) / 2.0f; x++)
    {
        float xPos = ((x - ((ARENA_NUM_X - 1.0f) / 4.0f)) * ARENA_STRETCH_FACTOR * 2.0f) + ARENA_STRETCH_FACTOR;

        world->addLamp(lampObjData, deferredShadingObj, shader, lamp_model_without_position,
                       glm::vec3(xPos, lampHeight, 0),                  // position
                       lightRadius,                                     // radius
                       glm::vec3(0.6f, 0.6f, 1.0f),                     // colour
                       lightAmbient,                                    // ambient
                       lightDiffuse,                                    // diffuse
                       lightSpecular);                                  // specular

        world->addLamp(lampObjData, deferredShadingObj, shader, lamp_model_without_position,
                       glm::vec3(xPos, lampHeight, zPosFurtherst),      // position
                       lightRadius,                                     // radius
                       glm::vec3(0.6f, 0.6f, 1.0f),                     // colour
                       lightAmbient,                                    // ambient
                       lightDiffuse,                                    // diffuse
                       lightSpecular);                                  // specular
    }

    // rotate lamp by 90 degrees and do other sides
    lamp_model_without_position = glm::rotate(glm::radians(90.0f), glm::vec3(0,1,0)) * lamp_model_without_position;

    float leftXPos = -((ARENA_NUM_X - 1.0f) / 2.0f) * ARENA_STRETCH_FACTOR;
    float rightXPos = ((ARENA_NUM_X - 1.0f) / 2.0f) * ARENA_STRETCH_FACTOR;
    for (unsigned int z = 0; z < (ARENA_NUM_Z - 1.0f) / 2.0f; z++)
    {
        float zPos = -(ARENA_STRETCH_FACTOR + (z * 2.0f * ARENA_STRETCH_FACTOR));

        world->addLamp(lampObjData, deferredShadingObj, shader, lamp_model_without_position,
                       glm::vec3(leftXPos, lampHeight, zPos),   // position
                       lightRadius,                             // radius
                       glm::vec3(0.6f, 0.6f, 1.0f),             // colour
                       lightAmbient,                            // ambient
                       lightDiffuse,                            // diffuse
                       lightSpecular);                          // specular

        world->addLamp(lampObjData, deferredShadingObj, shader, lamp_model_without_position,
                       glm::vec3(rightXPos, lampHeight, zPos),  // position
                       lightRadius,                             // radius
                       glm::vec3(0.6f, 0.6f, 1.0f),             // colour
                       lightAmbient,                            // ambient
                       lightDiffuse,                            // diffuse
                       lightSpecular);                          // specular
    }

    world->addLamp(lampObjData, deferredShadingObj, shader, lamp_model_without_position,
        glm::vec3(0, lampHeight, zPosFurtherst/2.0f),    // position
        lightRadius,                                     // radius
        glm::vec3(0.6f, 0.6f, 1.0f),                     // colour
        lightAmbient,                                    // ambient
        lightDiffuse,                                    // diffuse
        lightSpecular);                                  // specular


    return true;
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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


    // Open a window and create its OpenGL context
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tron", NULL, NULL);
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

    // can only use black with deffered shading
    // background colour is set in the lighting_pass fragment shader
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // set up our shaders
    if (!Shader::setupShaders())
    {
        system("pause");
        return -1;
    }

    // load default font
    std::shared_ptr<Texture> defaultFont = Texture::getOrCreate(std::string("textures/compressed/Holstein.DDS"));
    if (!defaultFont)
    {
        printf("Couldn't loadDDS font texture\n");
        system("pause");
        return false;
    }

    // create progress bar
    ProgressBar progressBar({{ProgressBar::PROGRESS_TYPE_LOAD_BIKE,100}}, window, Shader::getShader(SHADER_TYPE_2D), defaultFont);

    // projection matrix = camera -> homogenous (3d -> 2d)
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    std::shared_ptr<World> world = std::make_shared<World>();
    world->setProjection(glm::perspective(45.0f, (float)width / height, 0.1f, 1000.0f));

    // view matrix = world -> camera
    float distanceBetweenBikeAndCamera = 9.0f;
    const float CAMERA_Y_POS = 4.0f;
    const float CAMERA_Y_LOOKAT_POS = 2.0f;
    world->setCamera(glm::lookAt(glm::vec3(0, 0, distanceBetweenBikeAndCamera),    // where the camera is in world co-ordinates
                                 glm::vec3(0, CAMERA_Y_LOOKAT_POS, 0),    // target (direction = target - location)
                                 glm::vec3(0, 1, 0)));  // which way is up

    // setup Render Pipeline
    RenderPipeline renderPipeline(world, SCR_WIDTH, SCR_HEIGHT);
    if (!renderPipeline.initialise())
    {
        printf("Failed to initialise render pipeline\n");
        system("pause");
        return false;
    }

    std::shared_ptr<ObjLoader> bikeLoader = std::make_shared<ObjLoader>("models/obj/bike.obj", "models/obj/bike.tex", &progressBar, ProgressBar::PROGRESS_TYPE_LOAD_BIKE);
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

    // we are scaling the bike by 1/2, so update bounding box
    // TODO add support to getBoundingBox to do this?
    const float BIKE_SCALE_FACTOR = 0.5f;
    bike_lowest *= BIKE_SCALE_FACTOR;
    bike_most_forward *= BIKE_SCALE_FACTOR;

    // model matrix = model -> world
    glm::mat4 bike_model = glm::translate(glm::vec3(0.0f, -bike_lowest, 0.0f)) *
                           glm::scale(glm::vec3(BIKE_SCALE_FACTOR, BIKE_SCALE_FACTOR, BIKE_SCALE_FACTOR));

    // Load bike
    std::shared_ptr<Bike> bike = std::make_shared<Bike>(bikeLoader, world, Shader::getShader(SHADER_TYPE_MAIN_GEOMETRY_PASS), Shader::getShader(SHADER_TYPE_EXPLODE_GEOMETRY_PASS), bike_model, tronBlue);
    renderPipeline.add3DObject(bike);

    // create arena
    std::shared_ptr<ObjData3D> arenaObjData(createArena());
    if (!arenaObjData)
    {
        printf("Failed to create arena obj data\n");
        system("pause");
        return -1;
    }
    std::shared_ptr<Object> arena = std::make_shared<Object>(arenaObjData, world, Shader::getShader(SHADER_TYPE_MAIN_GEOMETRY_PASS), glm::mat4(1.0f));
    renderPipeline.add3DObject(arena);

    if (!setupArenaLighting(world))
    {
        printf("Failed to set up arena lighting\n");
        system("pause");
        return -1;
    }

    // create debug text object
    std::shared_ptr<Text> text = std::make_shared<Text>(Shader::getShader(SHADER_TYPE_2D));
    renderPipeline.add2DObject(text);
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
    std::shared_ptr<Text> activeSegmentText = std::make_shared<Text>(Shader::getShader(SHADER_TYPE_2D));
    renderPipeline.add2DObject(activeSegmentText);
#endif

    // create speed bar
    std::shared_ptr<Shape2D> speedBar = std::make_shared<Shape2D>(Shader::getShader(SHADER_TYPE_2D));
    speedBar->addRect(glm::vec2(SPEED_BAR_START_X-2.0f,580), glm::vec2(SPEED_BAR_END_X+2,580), glm::vec2(SPEED_BAR_END_X+2,560), glm::vec2(SPEED_BAR_START_X-2.0f,560),
                      glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    speedBar->addRect(glm::vec2(SPEED_BAR_START_X,578), glm::vec2(SPEED_BAR_END_X,578), glm::vec2(SPEED_BAR_END_X,562), glm::vec2(SPEED_BAR_START_X,562),
                      glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    renderPipeline.add2DObject(speedBar);

    std::shared_ptr<Text> speedBarText = std::make_shared<Text>(Shader::getShader(SHADER_TYPE_2D));
    speedBarText->addText2D("speed", (unsigned int)(SPEED_BAR_START_X - 156), 560, 26, defaultFont);
    renderPipeline.add2DObject(speedBarText);

    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

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
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
    bool qKeyPressed = false;           // decrement active segment ID
    bool wKeyPressed = false;           // increment active segment ID
#endif
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
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
    unsigned int activeSegmentId = 0;
    unsigned int lastActiveSegmentId = UINT_MAX;
    unsigned int lastNumSegments = 0;
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
            bike->toggleLightTrail();
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
            bike->saveBikeState();
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
            bike->restoreBikeState();
            if (stateIsSaved)
            {
                cameraRotationDegrees = savedCameraRotationDegrees;
                cameraRotating = savedCameraRotating;
                stop = savedStop;
            }
        }

#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
        // change active segment ID
        if (glfwGetKey(window, GLFW_KEY_Q ))
        {
            qKeyPressed = 1;
        }
        if (qKeyPressed && !glfwGetKey(window, GLFW_KEY_Q ))
        {
            qKeyPressed = 0;
            if (activeSegmentId != 0)
            {
                activeSegmentId--;
            }
            else
            {
                activeSegmentId = LightTrailSegment::getNumSegments();
            }
        }

        if (glfwGetKey(window, GLFW_KEY_W ))
        {
            wKeyPressed = 1;
        }
        if (wKeyPressed && !glfwGetKey(window, GLFW_KEY_W ))
        {
            wKeyPressed = 0;
            if (activeSegmentId == LightTrailSegment::getNumSegments())
            {
                activeSegmentId = 0;
            }
            else
            {
                activeSegmentId++;
            }
        }
#endif

#endif

        // update the bike and light trails
        bike->update(turnDirection, accelerating, stop);

        // check for collisions
        // only with it's own trail ATM, as there are no more

        // transform co-ords of front of bike to world co-ords.
        glm::vec3 bikeFrontLocation = bike->applyModelMatrx(glm::vec3(0,0,bike_most_forward));

        std::shared_ptr<const LightTrailManager> tm = bike->getTrailManager();
        if (tm->collides(glm::vec2(bikeFrontLocation.x, bikeFrontLocation.z)) ||
            bike->checkSelfCollision())
        {
            bike->setExploding();
            //cameraRotating = true;
        }

        // update camera location =============================================
        // transform origin of bike to world co-ords.
        glm::vec3 bikeLocation = bike->applyModelMatrx(glm::vec3(0,0,bike_most_forward));

        // we want the camera to be distanceBetweenBikeAndCamera from the bike->
        // by default it should be directly behind the bike so we can see where we are going
        // however I also want a mode where the camera rotates around so you can see the bike
        // so take a vector where the camera is directly behind the bike, in bike model space
        // and rotate it, this rotates the point around the bike->
        glm::vec3 cameraOffsetFromBike = glm::vec3(glm::vec4(0,0,distanceBetweenBikeAndCamera,1) *
                                                   glm::rotate(glm::radians(cameraRotationDegrees), glm::vec3(0,1,0)));
        // transform the camera position into world co-ordinates
        // note: this applies rotations, translations and scaling + any other transform
        //       so it won't work correctly if you scale your model in the Z direction
        //       your camera position will be scaled too
        glm::vec3 cameraPosition = bike->applyModelMatrx(cameraOffsetFromBike);
        // We want the y co-ord to be a bit above the bike
        cameraPosition.y = CAMERA_Y_POS;
        // point the camera at the bike, but adjust the y so we aren't looking too much down
        glm::vec3 cameraDirection = glm::vec3(bikeLocation.x, CAMERA_Y_LOOKAT_POS, bikeLocation.z);

        world->setCamera(glm::lookAt(cameraPosition,       // where the camera is in world co-ordinates
                                     cameraDirection,      // target (direction = target - location)
                                     glm::vec3(0, 1, 0)));  // which way is up

        // if the camera is rotating around the bike, then update the angle
        if (cameraRotating)
        {
            cameraRotationDegrees += 0.4f;
        }

        // update speed bar output
        float speedPercent = bike->getSpeedPercent();
        if (abs(lastSpeed - speedPercent) > 0.01f)
        {
            lastSpeed = speedPercent;
            float end_x = SPEED_BAR_START_X + (SPEED_BAR_END_X - SPEED_BAR_START_X) * speedPercent;
            speedBar->deleteObjData("speed_bar");
            speedBar->addRect(glm::vec2(SPEED_BAR_START_X,578), glm::vec2(end_x,578), glm::vec2(end_x,562), glm::vec2(SPEED_BAR_START_X,562),
                              glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(speedPercent, 1.0f - speedPercent, 0.0f), glm::vec3(speedPercent, 1.0f - speedPercent, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                              "speed_bar");
        }

        // update debug stats
        if (frameCount == 0)
        {
            char textBuff[32];
            text->deleteAllObjData();

            snprintf(textBuff, 32, "FR (Actual): %d", frameRate);
            text->addText2D(textBuff, 10, 560, 26, defaultFont);

            snprintf(textBuff, 32, "      (MAX): %d", displayedMaxPossibleFrameRate);
            text->addText2D(textBuff, 10, 530, 26, defaultFont);
        }

#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
        if (activeSegmentId != lastActiveSegmentId ||
            LightTrailSegment::getNumSegments() != lastNumSegments)
        {
            lastNumSegments = LightTrailSegment::getNumSegments();
            if (activeSegmentId != lastActiveSegmentId)
            {
                LightTrailSegment::setActiveSegmentID(activeSegmentId);
                lastActiveSegmentId = activeSegmentId;
            }

            activeSegmentText->deleteAllObjData();

            char textBuff[32];
            char activeString[16];
            if (activeSegmentId != 0)
            {
                snprintf(activeString, 16, "%u", activeSegmentId);
            }
            else
            {
                snprintf(activeString, 16, "ALL");
            }
            snprintf(textBuff, 32, "Segments: %u Active [%s]", LightTrailSegment::getNumSegments(), activeString);
            activeSegmentText->addText2D(textBuff, 10, 500, 26, defaultFont);
        }
#endif

        // render ==============================================================
        renderPipeline.render();

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
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

