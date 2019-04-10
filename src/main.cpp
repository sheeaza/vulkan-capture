#include <iostream>
#include <string>
#include <vector>

#include <signal.h>

#include "render.hpp"

static volatile bool keepRunning = true;

int main()
{
    Render render;

    if (render.init()) {
        std::cout << "init failed" << std::endl;
    } else {
        std::cout << "init success" << std::endl;
    }

    signal(SIGINT, [](int){ keepRunning = false; });

    int frameCount = 0;
    double previousTime = glfwGetTime();
    double currentTime;

    while (keepRunning) {
        currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - previousTime >= 1.0) {
            std::cout << frameCount << std::endl;
            frameCount = 0;
            previousTime = currentTime;
        }

        glfwPollEvents();
        render.render();
    }

    return 0;
}
