## minhttpd

一个超小型http服务,仅支持html和get动词，暂不支持中文(稍后会解决),去掉注释及空行的剩余代码量在300行左右。可支持windows和linux gcc编译。

#### windwos MinGW编译:
``` bash
gcc -o http http.c -l wsock32
```

#### 站点搭建
站点搭建:
站点|
	|---  http.exe  服务程序
	|---  www   网站目录
``` bash
http 8080
```
