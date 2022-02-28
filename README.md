# webserver

使用
------------
* 修改http_conn.cpp中的root路径

    ```C++
	// 修改为静态文件夹所在路径
    const char* doc_root="/home/wyf/mywebserver/static";
    ```

* 脚本生成webserver

    ```C++
    ./build.sh
    ```

* 启动webserver

    ```C++
    ./webserver localhost port
    ```

* 浏览器端

    ```C++
    ip:port
    ```
