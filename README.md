# SimpleQtTcpServerClient



Qt 6.8  + CMake 3.29



服务器编译

```sh
cd TCPServer
mkdir build
cd build && cmake ..
make install
make
```



客户端编译

```sh
cd TCPClient
mkdir build
cd build && cmake ..
make install
make
```

先运行服务端程序,再运行客户端程序