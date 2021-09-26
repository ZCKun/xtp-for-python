# xtp-for-python

you need:
> pybind11
> cmake (maybe need > 3.20+)

## Build
1. install pybind11
```shell
./vcpkg install pybind11
```
or [installing](https://pybind11.readthedocs.io/en/latest/installing.html)

2. build
```shell
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=/home/x2h1z/Source/vcpkg/scripts/buildsystems/vcpkg.cmake -DPYTHON_INCLUDE_PATH="/usr/include/python3.9" -DPYTHON_LIBRARY_PATH="/usr/lib/python3.9" -DLINUX=ON
make -j 8
```
You can change the compile platfrom with option parameter of -DLINUX/WIN/OSX. Default LINUX.

Enjoy!


## Thanks
[xtp_api_pytyhon](https://github.com/ztsec/xtp_api_python)

## Concat
E-Mail: zckuna@163.com


