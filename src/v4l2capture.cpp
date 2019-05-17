#include "v4l2capture.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <stdexcept>
#include <iostream>
#include <cstring>

V4l2Capture::V4l2Capture()
{
}

V4l2Capture::~V4l2Capture()
{
    ::close(m_fd);
}

void V4l2Capture::open(const std::string &path, const ImgFormat &imgFormat,
                       const std::array<Buffer, 4> &buffers)
{
    int ret;

    if (imgFormat.width <= 0 ||
        imgFormat.height <= 0 ||
        buffers.size() < 2) {
        throw std::runtime_error("invalid initialization params");
    }

    m_width = imgFormat.width;
    m_height = imgFormat.height;
    m_pixFmt = static_cast<uint32_t>(imgFormat.m_pixFmt);
    m_bufferNum = buffers.size();

    m_fd = ::open(path.c_str(), O_RDWR | O_NONBLOCK);
    // m_fd = ::open(path.c_str(), O_RDWR);
    if (m_fd == -1) {
        throw std::runtime_error("failed to open: " + path);
    }

    struct v4l2_capability cap = {};
    ret = ioctl(m_fd, VIDIOC_QUERYCAP, &cap);
    if (ret == -1 || !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        throw std::runtime_error(path + ": do not support multi-plane");
    }

    enumFormat();

    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    fmt.fmt.pix_mp.width = m_width;
    fmt.fmt.pix_mp.height = m_height;
    fmt.fmt.pix_mp.pixelformat = m_pixFmt;
    fmt.fmt.pix_mp.num_planes = 1;
    if (ioctl(m_fd, VIDIOC_S_FMT, &fmt)) {
        throw std::runtime_error("failed to set format");
    }

    std::memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (ioctl(m_fd, VIDIOC_G_FMT, &fmt)) {
        throw std::runtime_error("VIDIOC_G_FMT failed");
    }
    std::cout << "\twidth: " << fmt.fmt.pix_mp.width
              << "\theight: " << fmt.fmt.pix_mp.height << std::endl;
    std::cout << "\timage size: " << fmt.fmt.pix_mp.plane_fmt[0].sizeimage
              << std::endl;
    std::cout << "\tpixelformat: "
              << static_cast<char>(fmt.fmt.pix_mp.pixelformat & 0xff)
              << static_cast<char>(fmt.fmt.pix_mp.pixelformat >> 8 & 0xff)
              << static_cast<char>(fmt.fmt.pix_mp.pixelformat >> 16 & 0xff)
              << static_cast<char>(fmt.fmt.pix_mp.pixelformat >> 24 & 0xff)
              << std::endl;
    m_frameSize =fmt.fmt.pix_mp.plane_fmt[0].sizeimage;

    struct v4l2_streamparm parm = {};
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (ioctl(m_fd, VIDIOC_G_PARM, &parm)) {
        throw std::runtime_error("failed to VIDIOC_G_PARM");
    }
    std::cout << "\tfps: " << parm.parm.capture.timeperframe.denominator
              << std::endl;

    struct v4l2_requestbuffers req = {};
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    req.count = m_bufferNum;
    req.memory = V4L2_MEMORY_USERPTR;
    if (ioctl(m_fd, VIDIOC_REQBUFS, &req)) {
        throw std::runtime_error("do not support V4L2_MEMORY_USERPTR");
    }
    if (req.count < 2) {
        throw std::runtime_error("Insufficient buffer memory");
    }

    m_buffers = buffers;
}

void V4l2Capture::start()
{
    for (int i = 0; i < m_bufferNum; i++) {
        struct v4l2_buffer buf = {};
        struct v4l2_plane plane = {};

        plane.length = m_buffers.at(i).length;
        plane.m.userptr = reinterpret_cast<unsigned long>(m_buffers.at(i).start);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf.memory = V4L2_MEMORY_USERPTR;
        buf.index = i;
        buf.m.planes = &plane;
        buf.length = 1;

        if (ioctl(m_fd, VIDIOC_QBUF, &buf)) {
            std::cout << errno << std::endl;
            throw std::runtime_error("VIDIOC_QBUF error");
        }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    if (ioctl(m_fd, VIDIOC_STREAMON, &type)) {
        throw std::runtime_error("VIDIOC_STREAMON error");
    }
}

void V4l2Capture::stop()
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

    if (ioctl(m_fd, VIDIOC_STREAMOFF, &type)) {
        throw std::runtime_error("VIDIOC_STREAMOFF error");
    }
}

int V4l2Capture::readFrame()
{
    struct v4l2_buffer buf = {};
    struct v4l2_plane plane = {};

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buf.memory = V4L2_MEMORY_USERPTR;
    buf.length = 1;
    buf.m.planes = &plane;

    if (ioctl(m_fd, VIDIOC_DQBUF, &buf)) {
        // std::cout << "no buffer " << errno << std::endl;
        return -1;
    }

    return buf.index;
}

void V4l2Capture::doneFrame(int index)
{
    struct v4l2_buffer buf = {};
    struct v4l2_plane plane = {};

    plane.length = m_buffers.at(index).length;
    plane.m.userptr = reinterpret_cast<unsigned long>(m_buffers.at(index).start);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    buf.memory = V4L2_MEMORY_USERPTR;
    buf.index = index;
    buf.m.planes = &plane;
    buf.length = 1;

    if (ioctl(m_fd, VIDIOC_QBUF, &buf)) {
        throw std::runtime_error("VIDIOC_QBUF error");
    }

}

void V4l2Capture::enumFormat()
{
    int ret;

    struct v4l2_fmtdesc fmtdesc = {};
    fmtdesc.type = V4L2_CAP_VIDEO_CAPTURE_MPLANE;
    for (int i = 0; ;i++) {
        fmtdesc.index = i;

        ret = ioctl(m_fd, VIDIOC_ENUM_FMT, &fmtdesc);
        if (ret == -1)
            break;

        std::cout << "index: " << i << ", pixelformat: "
                  << std::string(reinterpret_cast<char *>(&fmtdesc.pixelformat));
    }
}

