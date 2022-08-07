# webserver

使用
------------
* 修改webserver.cpp中的root路径

    ```C++
	// 下面的路径换成你自己本机的路径就好
    root_dir = "/home/wyf/mywebserver";
    ```

* 在项目地址的主目录下执行脚本生成webserver

    ```C++
    ./build.sh
    ```

* 启动webserver

    ```C++
    ./webserver 
    ```

* 在项目地址的skiplist目录下执行脚本生成main

    ```
    ./build.sh
    ```

- 启动跳表数据库服务

  ```
  ./main
  ```

- 在项目地址的multi-player目录下执行脚本生成websocket

  ```
  ./build.sh
  ```

- 启动websocket服务、

  ```
  ./websocket
  ```

* 浏览器端访问

    ```C++
    ip:port
    // 182.92.85.127:16666
    ```
