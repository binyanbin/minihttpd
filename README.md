## minhttpd

一个超小型http服务,仅支持html和get动词，暂不支持中文(稍后会解决),去掉注释及空行的剩余代码量在300行左右。可支持windows和linux gcc编译。

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
