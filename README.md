# vulkan-capture
This is used for test v4l2 userptr capture and rendered by vulkan.

## Building step for imx8qm b0 mek:
### Note you need to compile and install glfw lib first
see https://www.glfw.org/docs/latest/compile.html

1). install vulkan shader compiler refer to the website:
https://vulkan.lunarg.com/sdk/home#linux

2). 
```
$ git clone https://github.com/latiaoxia/vulkan-capture.git

$ cd vulkan-capture
$ mkdir build
$ cd build
$ source ${yocto-sdk-enviroment-for-cross-compile}
$ cmake ../
$ make install
```
all the outputs will be in ```Debug/bin```, copy to your platform and run:
```
$ scp Debug/bin/* <user>@<ip-addr>:/dir/to/copy
```
make sure you have connected the camera at ```/dev/video4```.
This is hard coded at ```captures[i].open("/dev/video4",```.
```
$ cd /dir/to/bin
$ ./vulkan-cap
```
