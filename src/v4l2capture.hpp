#pragma once

#include <linux/videodev2.h>
#include <string>
#include <vector>

class V4l2Capture
{
public:
    enum class PixFormat
    {
        XBGR32 = V4L2_PIX_FMT_XBGR32,
    };

    struct ImgFormat
    {
        int width;
        int height;
        PixFormat m_pixFmt;

        ImgFormat(int width = -1, int height = -1,
                  PixFormat pixFmt = PixFormat::XBGR32) :
            width(width),
            height(height),
            m_pixFmt(pixFmt)
        {}
    };

    struct Buffer
    {
        void *start;
        size_t length;

        Buffer(void *start_ = nullptr, size_t length_ = 0) :
            start(start_),
            length(length_)
        {}

        Buffer& operator=(const Buffer& other)
        {
            if (this != &other) {
                start = other.start;
                length = other.length;
            }
            return *this;
        }
    };

    V4l2Capture();
    virtual ~V4l2Capture();

    void open(const std::string &path, const ImgFormat &imgFormat,
              const std::vector<Buffer> &buffers);
    void start();
    void stop();
    int readFrame();
    void doneFrame(int index);

private:
    int m_fd = -1;
    int m_width;
    int m_height;
    int m_frameSize;
    uint32_t m_pixFmt;
    int m_bufferNum;
    std::vector<Buffer> m_buffers;

    void enumFormat();
};
