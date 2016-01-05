# TronLightCycle
Tron light cycle game / demo

Requires these libraries:
    GLFW (v3.1.2)
    ASSIMP (v3.2)
    GLM (v0.9.7.2)
    GLEW (v1.13.0)

Credit to:
    http://www.opengl-tutorial.org/
        for tutorials and code to get me started
    https://www.google.com.bo/url?sa=i&rct=j&q=&esrc=s&source=images&cd=&cad=rja&uact=8&ved=0CAMQjxxqFQoTCNGxz--H98gCFUHnJgodgFYC2w&url=http%3A%2F%2Fwww.darksim.com%2Fchallenge%2Fhtml%2Fapril_2003.html&psig=AFQjCNFW6bhuWnxmcfCkoLUb8fl9RQrdsg&ust=1446736342239189
        user: sky
        for tyre texture
    http://hhh316.deviantart.com/art/Seamless-metal-texture-smooth-164165216
        handlebar and foot pedal texture

TODO:
    loading bar
        need to mod assimp
            check out my changes to ObjFileImporter::InternalReadFile
            need to add progress to ObjFileParser parser(m_Buffer, modelName, pIOHandler);
            see if can contribute via GIT?
    sort out lighting, everything is too dark
        support multiple lights
        support strip lights (connected to body of bike)?
            look into defered shading?
            render to texture
            apply blur
            ...
    add support for collisions effects
        ie big splosion - think michael bay.
    Tidy up
        shader class, use some sort of map rather than individual vars
        texture manager
            create texture class
            pass shared ptrs around
            textureManager creates new texture unless it's already cached
        tweak keyboard input to use callbacks
            receive character press rather than key press, so different layouts work
            add mouse scroll wheel event for zooming in or out
    sort out scales, maybe make bike smaller? or floor bigger?
    add boundary and buildings etc...
    make floor reflective
    add second floor with basic drops and ramps
    make top floor semi transparent
    work out how to distribute a binary to people
    add demo mode
        bike drives itself
        camera follows set path too
        or can override and move camera
    make bike lean on turns