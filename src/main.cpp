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

Shader *setupMainGeometryPassShader(const std::string *geometryShader = NULL)
{
    // first init main shader
    Shader *shader = new Shader("shaders/main_geometry_pass.vs", "shaders/main_geometry_pass.fs", geometryShader);
    if (!shader->compile())
    {
        printf("Failed to compile main geometry pass shader\n");
        delete shader;
        return NULL;
    }
    else
    {
        // Get main shader parameters
        if (// vertex params (variable)
            !shader->addAttribID("vertexPosition_Model", SHADER_ATTRIB_VERTEX_POS) ||
            !shader->addAttribID("vertexNormal_Model", SHADER_ATTRIB_VERTEX_NORMAL) ||
            !shader->addAttribID("vertexTextureUV", SHADER_ATTRIB_VERTEX_UV) ||
            // vertex params (static)
            !shader->addUniformID("MVP", SHADER_UNIFORM_MVP) ||
            !shader->addUniformID("MV", SHADER_UNIFORM_MODEL_VIEW_MATRIX) ||
            !shader->addUniformID("normalMV", SHADER_UNIFORM_NORMAL_MODEL_VIEW_MATRIX) ||
            // fragment params
            !shader->addUniformID("fragmentIsTexture", SHADER_UNIFORM_IS_TEXTURE) ||
            !shader->addUniformID("textureSampler", SHADER_UNIFORM_TEXTURE_SAMPLER) ||
            !shader->addUniformID("fragmentColour", SHADER_UNIFORM_FRAGMENT_COLOUR))
        {
            printf("Error adding shader IDs\n");
            delete shader;
            return NULL;
        }
    }

    return shader;
}

Shader *setupExplodeShader()
{
    // first init main shader
    std::string gsPath = "shaders/explode_geometry_pass.gs";
    Shader *explodeShader = setupMainGeometryPassShader(&gsPath);
    if (explodeShader)
    {
        if (!explodeShader->addUniformID("explode", SHADER_UNIFORM_EXPLODE) ||
            !explodeShader->addUniformID("inverseProjectionMatrix", SHADER_UNIFORM_INVERSE_PROJECTION_MATRIX))
        {
            printf("Error adding explode shader IDs\n");
            delete explodeShader;
            return NULL;
        }
    }

    return explodeShader;
}

Shader *setupMainLightingPassShader()
{
    // first init main shader
    Shader *shader = new Shader("shaders/main_lighting_pass.vs", "shaders/main_lighting_pass.fs");
    if (!shader->compile())
    {
        printf("Failed to compile main lighting pass shader\n");
        delete shader;
        return NULL;
    }
    else
    {
        // Get main shader parameters
        if (// vertex params (variable)
            !shader->addAttribID("vertexPosition_Model", SHADER_ATTRIB_VERTEX_POS) ||
            // vertex params (static)
            !shader->addUniformID("MVP", SHADER_UNIFORM_MVP) ||
            // fragment params
            !shader->addUniformID("screenResolution", SHARDER_UNIFORM_SCREEN_RES) ||
            !shader->addUniformID("lightPosition_Camera", SHADER_UNIFORM_LIGHT_POS_CAMERA) ||
            !shader->addUniformID("lightRadius", SHADER_UNIFORM_LIGHT_RADIUS) ||
            !shader->addUniformID("lightColour", SHADER_UNIFORM_LIGHT_COLOUR) ||
            !shader->addUniformID("lightAmbient", SHADER_UNIFORM_LIGHT_AMBIENT_FACTOR) ||
            !shader->addUniformID("lightDiffuse", SHADER_UNIFORM_LIGHT_DIFFUSE_FACTOR) ||
            !shader->addUniformID("lightSpecular", SHADER_UNIFORM_LIGHT_SPECULAR_FACTOR) ||
            !shader->addUniformID("geometryTextureSampler", SHADER_UNIFORM_GEOMETRY_TEXTURE_SAMPLER) ||
            !shader->addUniformID("normalTextureSampler", SHADER_UNIFORM_NORMAL_TEXTURE_SAMPLER) ||
            !shader->addUniformID("colourTextureSampler", SHADER_UNIFORM_COLOUR_TEXTURE_SAMPLER))
        {
            printf("Error adding shader IDs\n");
            delete shader;
            return NULL;
        }
    }

    return shader;
}

Shader *setupFinalScreenPassShader()
{
    // first init main shader
    Shader *shader = new Shader("shaders/final_screen_pass.vs", "shaders/final_screen_pass.fs");
    if (!shader->compile())
    {
        printf("Failed to compile final screen pass shader\n");
        delete shader;
        return NULL;
    }
    else
    {
        // Get main shader parameters
        if (// vertex params (variable)
            !shader->addAttribID("vertexPosition_Screen", SHADER_ATTRIB_VERTEX_POS) ||
            // fragment params
            !shader->addUniformID("screenResolution", SHARDER_UNIFORM_SCREEN_RES) ||
            !shader->addUniformID("colourTextureSampler", SHADER_UNIFORM_COLOUR_TEXTURE_SAMPLER))
        {
            printf("Error adding shader IDs\n");
            delete shader;
            return NULL;
        }
    }

    return shader;
}

Shader *setupLampShader()
{
    // first init main shader
    Shader *shader = new Shader("shaders/lamp.vs", "shaders/lamp.fs");
    if (!shader->compile())
    {
        printf("Failed to compile lamp shader\n");
        delete shader;
        return NULL;
    }
    else
    {
        // Get main shader parameters
        if (// vertex params (variable)
            !shader->addAttribID("vertexPosition_Model", SHADER_ATTRIB_VERTEX_POS) ||
            // vertex params (static)
            !shader->addUniformID("MVP", SHADER_UNIFORM_MVP) ||
            // fragment params
            !shader->addUniformID("fragmentColour", SHADER_UNIFORM_FRAGMENT_COLOUR))
        {
            printf("Error adding shader IDs\n");
            delete shader;
            return NULL;
        }
    }

    return shader;
}

Shader *setup2DShader()
{
    // initialise text shader
    Shader *shader2D = new Shader( "shaders/2D.vs", "shaders/2D.fs");
    if (!shader2D->compile())
    {
        printf("Failed to compile 2D shader\n");
        return NULL;
    }
    else
    {
        // Get a handle for our buffers
        if (!shader2D->addAttribID("vertexPosition_screenspace", SHADER_ATTRIB_VERTEX_POS) ||
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
        delete arenaObjData;
        arenaObjData = NULL;
    }

    return arenaObjData;
}

ObjData3D *createLamp()
{
    ObjData3D *objData = new ObjData3D();
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
        delete objData;
        objData = NULL;
    }
    return objData;
}

bool setupArenaLighting(std::shared_ptr<World> world, std::shared_ptr<const Shader> shader)
{
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

    const float lightRadius = 50.0f;
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


    return true;
}

bool setupGeometryPassFrameBuffer(GLuint &geometryPassFrameBuffer, GLuint &geometryPassPositionTexture, GLuint &geometryPassNormalTexture, GLuint &geometryPassColourTexture)
{
    glGenFramebuffers(1, &geometryPassFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, geometryPassFrameBuffer);

    // - Positions
    glGenTextures(1, &geometryPassPositionTexture);
    glBindTexture(GL_TEXTURE_2D, geometryPassPositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, geometryPassPositionTexture, 0);

    // - Normals
    glGenTextures(1, &geometryPassNormalTexture);
    glBindTexture(GL_TEXTURE_2D, geometryPassNormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, geometryPassNormalTexture, 0);

    // - Colours
    glGenTextures(1, &geometryPassColourTexture);
    glBindTexture(GL_TEXTURE_2D, geometryPassColourTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, geometryPassColourTexture, 0);

    // - Depth
    GLuint depth;
    glGenTextures(1, &depth);
    glBindTexture(GL_TEXTURE_2D, depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

    // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Geometry pass framebuffer not complete!\n");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool setupLightingPassFrameBuffer(GLuint &lightingPassFrameBuffer, GLuint &lightingPassColourTexture)
{
    glGenFramebuffers(1, &lightingPassFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, lightingPassFrameBuffer);

    // - Colours
    glGenTextures(1, &lightingPassColourTexture);
    glBindTexture(GL_TEXTURE_2D, lightingPassColourTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightingPassColourTexture, 0);

    // - Depth
    GLuint depth;
    glGenTextures(1, &depth);
    glBindTexture(GL_TEXTURE_2D, depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

    // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    GLuint attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Lighting pass framebuffer not complete!\n");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

ObjData2D *setupScreenQuad()
{
    ObjData2D *objData = new ObjData2D();
    MeshData<glm::vec2> md;
    md.hasTexture = false;

    md.vertices.push_back(glm::vec2(-1.0f, -1.0f));
    md.vertices.push_back(glm::vec2(-1.0f,  1.0f));
    md.vertices.push_back(glm::vec2( 1.0f,  1.0f));
    md.vertices.push_back(glm::vec2( 1.0f, -1.0f));

    md.indices.push_back(0); md.indices.push_back(1); md.indices.push_back(2);
    md.indices.push_back(0); md.indices.push_back(2); md.indices.push_back(3);

    if (!objData->addMesh(md))
    {
        delete objData;
        objData = NULL;
    }
    return objData;
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

    std::shared_ptr<const Shader> shader2D;
    shader2D.reset(setup2DShader());
    if (!shader2D)
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
    ProgressBar progressBar({{ProgressBar::PROGRESS_TYPE_LOAD_BIKE,100}}, window, shader2D, defaultFont);

    // set up the rest of our shaders
    std::shared_ptr<const Shader> mainGeometryPassShader;
    mainGeometryPassShader.reset(setupMainGeometryPassShader());
    if (!mainGeometryPassShader)
    {
        system("pause");
        return -1;
    }

    std::shared_ptr<const Shader> mainLightingPassShader;
    mainLightingPassShader.reset(setupMainLightingPassShader());
    if (!mainLightingPassShader)
    {
        system("pause");
        return -1;
    }

    std::shared_ptr<const Shader> explodeShader;
    explodeShader.reset(setupExplodeShader());
    if (!explodeShader)
    {
        system("pause");
        return -1;
    }

    std::shared_ptr<const Shader> lampShader;
    lampShader.reset(setupLampShader());
    if (!lampShader)
    {
        system("pause");
        return -1;
    }

    std::shared_ptr<const Shader> finalScreenPassShader;
    finalScreenPassShader.reset(setupFinalScreenPassShader());
    if (!finalScreenPassShader)
    {
        system("pause");
        return -1;
    }

    // set up deferred shading frame buffer
    GLuint geometryPassFrameBuffer;
    GLuint geometryPassPositionTexture;
    GLuint geometryPassNormalTexture;
    GLuint geometryPassColourTexture;
    if (!setupGeometryPassFrameBuffer(geometryPassFrameBuffer, geometryPassPositionTexture, geometryPassNormalTexture, geometryPassColourTexture))
    {
        system("pause");
        return -1;
    }

    GLuint lightingPassFrameBuffer;
    GLuint lightingPassColourTexture;
    if (!setupLightingPassFrameBuffer(lightingPassFrameBuffer, lightingPassColourTexture))
    {
        system("pause");
        return -1;
    }

    std::unique_ptr<ObjData2D> screenQuad;
    screenQuad.reset(setupScreenQuad());
    if (!screenQuad)
    {
        printf("failed to set up screen quad\n");
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
    float distanceBetweenBikeAndCamera = 9.0f;
    const float CAMERA_Y_POS = 4.0f;
    const float CAMERA_Y_LOOKAT_POS = 2.0f;
    world->setCamera(glm::lookAt(glm::vec3(0, 0, distanceBetweenBikeAndCamera),    // where the camera is in world co-ordinates
                                 glm::vec3(0, CAMERA_Y_LOOKAT_POS, 0),    // target (direction = target - location)
                                 glm::vec3(0, 1, 0)));  // which way is up

    std::shared_ptr<ObjLoader> bikeLoader(new ObjLoader("models/obj/bike.obj", "models/obj/bike.tex", &progressBar, ProgressBar::PROGRESS_TYPE_LOAD_BIKE));
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
    Bike bike(bikeLoader, world, mainGeometryPassShader, explodeShader, bike_model, tronBlue);

    // create arena
    std::shared_ptr<ObjData3D> arenaObjData(createArena());
    if (!arenaObjData)
    {
        printf("Failed to create arena obj data\n");
        system("pause");
        return -1;
    }
    Object arena(arenaObjData, world, mainGeometryPassShader, glm::mat4(1.0f));

    if (!setupArenaLighting(world, lampShader))
    {
        printf("Failed to set up arena lighting\n");
        system("pause");
        return -1;
    }

    // create debug text object
    Text text(shader2D);
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
    Text activeSegmentText(shader2D);
#endif

    // create speed bar
    Shape2D speedBar(shader2D);
    speedBar.addRect(glm::vec2(SPEED_BAR_START_X-2.0f,580), glm::vec2(SPEED_BAR_END_X+2,580), glm::vec2(SPEED_BAR_END_X+2,560), glm::vec2(SPEED_BAR_START_X-2.0f,560),
                     glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    speedBar.addRect(glm::vec2(SPEED_BAR_START_X,578), glm::vec2(SPEED_BAR_END_X,578), glm::vec2(SPEED_BAR_END_X,562), glm::vec2(SPEED_BAR_START_X,562),
        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));

    Text speedBarText(shader2D);
    speedBarText.addText2D("speed", (unsigned int)(SPEED_BAR_START_X - 156), 560, 26, defaultFont);

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
        bike.update(turnDirection, accelerating, stop);

        // check for collisions
        // only with it's own trail ATM, as there are no more

        // transform co-ords of front of bike to world co-ords.
        glm::vec3 bikeFrontLocation = bike.applyModelMatrx(glm::vec3(0,0,bike_most_forward));

        std::shared_ptr<const LightTrailManager> tm = bike.getTrailManager();
        if (tm->collides(glm::vec2(bikeFrontLocation.x, bikeFrontLocation.z)) ||
            bike.checkSelfCollision())
        {
            bike.setExploding();
            //cameraRotating = true;
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

        // do geometry pass for 3D objects =====================================
        glBindFramebuffer(GL_FRAMEBUFFER, geometryPassFrameBuffer);

        // enable both depth and stencil buffers for writting
        // needed here so we can clear them in glClear below
        glDepthMask(GL_TRUE);
        glStencilMask(0xFF);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        // enable the stencil test to always increment
        // this means the stencil buffer will be non 0 for any rendered pixel
        // leaving it 0 for only the background (non rendered pixels)
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 0, 0);
        glStencilOp(GL_INCR, GL_INCR, GL_INCR);

        glDisable(GL_BLEND);

        // draw bike
        bike.drawAll();

        // draw arena
        arena.drawAll();

        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);

        // copy stencil buffer from deferred frame buffer to default frame buffer =================
        glBindFramebuffer(GL_READ_FRAMEBUFFER, geometryPassFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightingPassFrameBuffer);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT,
                          0, 0, SCR_WIDTH, SCR_HEIGHT,
                          GL_STENCIL_BUFFER_BIT, GL_NEAREST);

        // Do deferred lighting stage ===========================================
        glBindFramebuffer(GL_FRAMEBUFFER, lightingPassFrameBuffer);
        glClear(GL_COLOR_BUFFER_BIT);

        glStencilMask(0x00);
        glStencilFunc(GL_NOTEQUAL, 0, 0xff);

        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        mainLightingPassShader->useShader();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, geometryPassPositionTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, geometryPassNormalTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, geometryPassColourTexture);

        // set screen resolution
        glm::vec2 screenRes(SCR_WIDTH, SCR_HEIGHT);
        glUniform2fv(mainLightingPassShader->getUniformID(SHARDER_UNIFORM_SCREEN_RES), 1, &screenRes[0]);

        world->sendLightingInfoToShader(mainLightingPassShader);

        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_CULL_FACE);

        // copy depth buffer from deferred frame buffer to default frame buffer =================
        glBindFramebuffer(GL_READ_FRAMEBUFFER, geometryPassFrameBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightingPassFrameBuffer);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT,
                          0, 0, SCR_WIDTH, SCR_HEIGHT,
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_FRAMEBUFFER, lightingPassFrameBuffer);

        // draw lamps
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        world->drawLamps();

        // Draw 2D objects in order they are listed =================================
        glDepthMask(GL_FALSE);
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

            snprintf(textBuff, 32, "FR (Actual): %d", frameRate);
            text.addText2D(textBuff, 10, 560, 26, defaultFont);

            snprintf(textBuff, 32, "      (MAX): %d", displayedMaxPossibleFrameRate);
            text.addText2D(textBuff, 10, 530, 26, defaultFont);
        }
        text.drawAll();

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

            activeSegmentText.deleteAllObjData();

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
            activeSegmentText.addText2D(textBuff, 10, 500, 26, defaultFont);
        }
        activeSegmentText.drawAll();
#endif

        // finally draw it all to the screen
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDisable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            finalScreenPassShader->useShader();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, lightingPassColourTexture);

            GLuint vertexPosition_ScreenID = finalScreenPassShader->getAttribID(SHADER_ATTRIB_VERTEX_POS);
            glUniform1i(finalScreenPassShader->getUniformID(SHADER_UNIFORM_COLOUR_TEXTURE_SAMPLER), 0);
            glUniform2fv(finalScreenPassShader->getUniformID(SHARDER_UNIFORM_SCREEN_RES), 1, &screenRes[0]);

            auto sqMeshes = screenQuad->getMeshes();
            for (auto &it : sqMeshes)
            {
                glEnableVertexAttribArray(vertexPosition_ScreenID);

                glBindBuffer(GL_ARRAY_BUFFER, it->vertexBuffer);
                glVertexAttribPointer(vertexPosition_ScreenID, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it->indiceBuffer);
                glDrawElements(GL_TRIANGLES, it->numIndices, GL_UNSIGNED_SHORT, (void *)0);

                glDisableVertexAttribArray(vertexPosition_ScreenID);
            }
        }

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

