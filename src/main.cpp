#include <iostream>
#include <string>
#include <vector>

#include <signal.h>

#include <opencv2/opencv.hpp>

#include "render.hpp"

static volatile bool keepRunning = true;

int main()
{
    Render render;
    signal(SIGINT, [](int){ keepRunning = false; });

    try {
        render.init();

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
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
