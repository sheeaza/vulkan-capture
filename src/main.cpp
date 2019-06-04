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
    // V4l2Capture captures;
    // std::vector<V4l2Capture::Buffer> buffers(4);
    // try {
        // captures.open("/dev/video1", V4l2Capture::ImgFormat(1280, 720, V4l2Capture::PixFormat::XBGR32),
                     // buffers);
    // } catch (const std::exception& e) {
        // std::cerr << e.what() << std::endl;
        // return -1;
    // }
    // return 0;

    int cameraNum = 4;
    Render render;
    signal(SIGINT, [](int){ keepRunning = false; });
    std::vector<V4l2Capture> captures(1);
    std::vector<std::array<V4l2Capture::Buffer, 4>> buffers(4);
    std::vector<std::array<void *, 4>> renderBufs(4);

    try {
        render.init();
        for (size_t i = 0; i < captures.size(); i++) {
            render.getBufferAddrs(i, renderBufs[i]);
            for (size_t j = 0; j < renderBufs[i].size(); j++) {
                buffers[i][j].start = renderBufs[i][j];
                buffers[i][j].length = 1280 * 800 * 4;
            }
            captures[i].open("/dev/video4",
                             V4l2Capture::ImgFormat(
                                 1280, 800, V4l2Capture::PixFormat::XBGR32),
                             buffers[i]);
            captures[i].start();
        }

        int frameCount = 0;
        double previousTime = glfwGetTime();
        double currentTime;
        int fCount = 0;

        int index[4] = {0, 0, 0, 0};

        while (keepRunning) {
            glfwPollEvents();

            for (size_t i = 0; i < captures.size(); i++) {

                index[i] = captures[i].readFrame();

                if (index[i] == -1) {
                    // std::cout << "errno : " << errno << std::endl;
                    continue;
                }

                fCount++;

                render.updateTexture(i, index[i]);
                captures[i].doneFrame(index[i]);
            }
            render.render(0);

            if (fCount != 0) {
                currentTime = glfwGetTime();
                frameCount++;
                double deltaT = currentTime - previousTime;
                if (deltaT >= 1.0) {
                    std::cout << frameCount / deltaT << std::endl;
                    frameCount = 0;
                    previousTime = currentTime;
                }
                fCount = 0;
            }
            // std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}
