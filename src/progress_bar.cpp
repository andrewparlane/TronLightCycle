#include <progress_bar.hpp>

#define PROGRESS_BAR_START_X 10.0f
#define PROGRESS_BAR_END_X   790.0f
#define PROGRESS_BAR_START_Y 280.0f
#define PROGRESS_BAR_END_Y   320.0f

ProgressBar::ProgressBar(std::map<ProgressType, unsigned int> _weights, GLFWwindow* _window, std::shared_ptr<const Shader> _shader, unsigned int _font)
    : weights(_weights), window(_window), shader(_shader), font(_font),
      progressBar(shader), progressText(shader), lastTotalPercent(0.0f)
{
    progressBar.addRect(glm::vec2(PROGRESS_BAR_START_X - 2.0f, PROGRESS_BAR_END_Y + 2.0f), glm::vec2(PROGRESS_BAR_END_X + 2.0f, PROGRESS_BAR_END_Y + 2.0f), glm::vec2(PROGRESS_BAR_END_X + 2.0f, PROGRESS_BAR_START_Y - 2.0f), glm::vec2(PROGRESS_BAR_START_X - 2.0f, PROGRESS_BAR_START_Y - 2.0f),
                        glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    progressBar.addRect(glm::vec2(PROGRESS_BAR_START_X, PROGRESS_BAR_END_Y), glm::vec2(PROGRESS_BAR_END_X, PROGRESS_BAR_END_Y), glm::vec2(PROGRESS_BAR_END_X, PROGRESS_BAR_START_Y), glm::vec2(PROGRESS_BAR_START_X, PROGRESS_BAR_START_Y),
                        glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::setText(const std::string &text)
{
    progressText.addText2D(text, 10, 250, 30, font);
}

void ProgressBar::update(ProgressType type, float percentage)
{
    if (type == PROGRESS_TYPE_NONE)
    {
        return;
    }

    float currentTotalPercent = percentage;
    if (currentTotalPercent > 1.0f)
    {
        currentTotalPercent = 1.0f;
    }
    else if (currentTotalPercent < 0.0f)
    {
        currentTotalPercent = 0.0f;
    }

    if (abs(lastTotalPercent - currentTotalPercent) > 0.01f)
    {
        lastTotalPercent = currentTotalPercent;
        float end_x = PROGRESS_BAR_START_X + (PROGRESS_BAR_END_X - PROGRESS_BAR_START_X) * currentTotalPercent;
        progressBar.deleteObjData("progress_bar");
        progressBar.addRect(glm::vec2(PROGRESS_BAR_START_X, PROGRESS_BAR_END_Y), glm::vec2(end_x, PROGRESS_BAR_END_Y), glm::vec2(end_x, PROGRESS_BAR_START_Y), glm::vec2(PROGRESS_BAR_START_X, PROGRESS_BAR_START_Y),
                            glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(currentTotalPercent, 1.0f - currentTotalPercent, 0.0f), glm::vec3(currentTotalPercent, 1.0f - currentTotalPercent, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                            "progress_bar");
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    progressBar.drawAll();
    progressText.drawAll();
    glfwSwapBuffers(window);
}
