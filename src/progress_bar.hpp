#ifndef __PROGRESS_BAR_HPP
#define __PROGRESS_BAR_HPP

#include "two_dimensional.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <map>
#include <string>
#include <memory>

#include <glfw3.h>

class ProgressBar
{
public:
    enum ProgressType
    {
        PROGRESS_TYPE_LOAD_BIKE = 0,

        NUM_PROGRESS_TYPES,

        PROGRESS_TYPE_NONE
    };

    // takes a map with key ProgressType and value progress weight
    ProgressBar(std::map<ProgressType,unsigned int> _weights, GLFWwindow* _window, std::shared_ptr<const Shader> _shader, std::shared_ptr<Texture> _font);
    ~ProgressBar();

    void setText(const std::string &text);
    void update(ProgressType, float percentage);

protected:
    std::map<ProgressType,unsigned int> weights;

    GLFWwindow* window;
    std::shared_ptr<const Shader> shader;
    std::shared_ptr<Texture> font;

    Shape2D progressBar;
    Text progressText;

    float lastTotalPercent;
};

#endif
