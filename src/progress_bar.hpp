#ifndef __PROGRESS_BAR_HPP
#define __PROGRESS_BAR_HPP

#include <map>
#include <string>
#include <memory>

struct GLFWwindow;
class Texture;
class Shader;
class Shape2D;
class Text;

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

    std::unique_ptr<Shape2D> progressBar;
    std::unique_ptr<Text> progressText;

    float lastTotalPercent;
};

#endif
