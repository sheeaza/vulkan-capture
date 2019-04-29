#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include <signal.h>

#include <opencv2/opencv.hpp>

#include "render.hpp"
#include "v4l2capture.hpp"

static volatile bool keepRunning = true;

int main()
{
    // V4l2Capture capture;
    // std::vector<V4l2Capture::Buffer> buffers(4);
    // try {
        // capture.open("/dev/video1", V4l2Capture::ImgFormat(1280, 720, V4l2Capture::PixFormat::XBGR32),
                     // buffers);
    // } catch (const std::exception& e) {
        // std::cerr << e.what() << std::endl;
        // return -1;
    // }
    // return 0;

    Render render;
    signal(SIGINT, [](int){ keepRunning = false; });
    V4l2Capture capture;
    std::vector<V4l2Capture::Buffer> buffers(4);
    std::vector<void *> renderBufs;

    try {
        render.init();
        render.getBufferAddrs(renderBufs);
        for (int i = 0; i < renderBufs.size(); i++) {
            buffers.at(i).start = renderBufs.at(i);
            buffers.at(i).length = 1280 * 800 * 4;
        }
        capture.open("/dev/video0", V4l2Capture::ImgFormat(1280, 800, V4l2Capture::PixFormat::XBGR32),
                     buffers);
        capture.start();

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

            int index = capture.readFrame();
            std::this_thread::sleep_for(std::chrono::seconds(10));
            exit(0);

            if (index == -1)
                continue;

            render.updateTexture(index);
            render.render();
            capture.doneFrame(index);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
