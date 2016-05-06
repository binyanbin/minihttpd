## minhttpd

一个超小型静态资源http服务器,,去掉注释及空行的剩余代码量小于300行。可支持windows和linux gcc编译。

#### 功能及目的

支持静态资源上传与下载，如图片,js,html,css等静态资源，使用静态资源与web主服务器分离的目的，可做为单独静态资源服务器使用。

#### windwos MinGW编译:
``` bash
gcc -o http http.c -l wsock32
```

#### 站点搭建
站点搭建:
创建站点目录，把http.exe复制到目录下，创建www目录并把网站的所有静态页面copy到目录下。
启动服务
``` bash
http 8080
```

#### 版本
v0.1.1 支持图片(最大1m),css，解决中文问题。[2016-5-4]
