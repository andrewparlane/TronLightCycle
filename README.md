# TronLightCycle
Tron light cycle game / demo

Requires these libraries:
    GLFW (v3.1.2)
    ASSIMP (v3.2)
        configure to not make shared libs, and not build exporter
    GLM (v0.9.7.2)
    GLEW (v1.13.0)

Credit to:
    http://www.opengl-tutorial.org/
        for tutorials and code to get me started
    http://www.learnopengl.com/
        for more advanced tutorials
    https://www.google.com.bo/url?sa=i&rct=j&q=&esrc=s&source=images&cd=&cad=rja&uact=8&ved=0CAMQjxxqFQoTCNGxz--H98gCFUHnJgodgFYC2w&url=http%3A%2F%2Fwww.darksim.com%2Fchallenge%2Fhtml%2Fapril_2003.html&psig=AFQjCNFW6bhuWnxmcfCkoLUb8fl9RQrdsg&ust=1446736342239189
        user: sky
        for tyre texture
    http://hhh316.deviantart.com/art/Seamless-metal-texture-smooth-164165216
        handlebar and foot pedal texture

TODO:
    improve assimp performance
        looking at stl flags
            with
                _HAS_ITERATOR_DEBUGGING=0
                _SECURE_SCL=0
            we are twice as fast.
            Check release builds, and 64 bit builds
            what to do?
                set the flags for only assimp in debug?
                deal with shitty debug build performance?
                etc..?
    loading bar
        add support for multiple actions
    Effects / Prettiness
        sort out lighting
            Everything is either too dark or too slow
                use stencil buffer to not render background
                research deferred lighting and tile-based deferred shading
                    https://software.intel.com/sites/default/files/m/d/4/1/d/8/lauritzen_deferred_shading_siggraph_2010.pdf
                    these are even more efficient methods of adding large numbers of lights
            research HDR
            add headlamps
                implement spot light support
            support strip lights (connected to body of bike)?
                research bloom
                ...
        sort out scales, maybe make bike smaller? or floor bigger?
        add boundary and buildings etc...
        make floor reflective
        make top floor semi transparent
        make bike lean on turns
    Tidy up
        tweak keyboard input to use callbacks
            receive character press rather than key press, so different layouts work
            add mouse scroll wheel event for zooming in or out
    Features
        limit speed up and braking power
        add reset key so don't have to restart app each time.
        do something after a collision
            grey out screen with restart overlay?
        add second floor with basic drops and ramps
        add demo mode
            bike drives itself
            camera follows set path too
            or can override and move camera
        work out how to distribute a binary to people
        Create build system
            use cmake
            test on linux?
            auto aquire dependencies?
        multi-player
            local or remote?
        add menu
            local play?
            network play?
            demo mode
            single player?
                like snake
                    collect items to do various things?
                        increase time left
                        clear trail
                        no collisions
                        allow more speed up power
                        allow more braking
                        ...
                with AIs?