#define MINGW32

#include <stdio.h>  
#include <sys/types.h>  
#include <ctype.h>  
#include <string.h>  
#include <sys/stat.h>  
#include <stdlib.h>  

#ifdef MINGW32
#include <winsock2.h>
#include <direct.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h> 
#endif
  
#pragma warning(disable : 4267)  
  
#define ISspace(x) isspace((int)(x))  
  
#define SERVER_STRING "Server: bdsoftmgr httpd/0.1.0\r\n"  
  
void accept_request(SOCKET);  
void bad_request(int);  
void cat(SOCKET, FILE *);  
void cannot_execute(int);  
void error_die(const char *);  
int get_line(SOCKET, char *, int);  
void headers(SOCKET);  
void not_found(SOCKET);  
void serve_file(SOCKET, const char *);  
SOCKET startup(u_short *);  
void unimplemented(SOCKET);  
void discardheaders(SOCKET);  
  
/**********************************************************************/  
/*请求处理*/
/**********************************************************************/  
void accept_request(SOCKET client)  
{  
  char buf[1024];  
  int numchars;  
  char method[255];  
  char url[255];  
  char path[512]; 
  size_t i, j;  
  struct stat st;  
  
  char *query_string = NULL;  
  
  numchars = get_line(client, buf, sizeof(buf));  
  i = 0; j = 0;   
  while (j < numchars && !ISspace(buf[j]) && (i < sizeof(method) - 1))  
  {  
    method[i] = buf[j];  
    i++; j++;  
  }  
  method[i] = '\0';  
  //method是第一行
  //未实现除get之外的其它动词 
  if (_stricmp(method, "GET") != 0)       
  {  
    if (numchars > 0)  
        discardheaders(client);  
      
    unimplemented(client);  
    closesocket(client);  
    return;  
  }  
  
  //取http地址
  i = 0;  
  while (ISspace(buf[j]) && (j < sizeof(buf)))  
    j++;  
  while (!ISspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf)))  
  {  
    url[i] = buf[j];  
    i++; j++;  
  }  
  url[i] = '\0';  
  
  if (_stricmp(method, "GET") == 0)  
  {  
    query_string = url;  
    while ((*query_string != '?') && (*query_string != '\0'))  
      query_string++;  
    if (*query_string == '?')  
    {  
      *query_string = '\0';  
      query_string++;  
    }  
  }  
  
  //获取当前程序目录
  #ifdef MINGW32
    getcwd(path,sizeof(path));
  #else
    _getcwd(path,sizeof(path));
  #endif 
  //静态页面目录
  strcat(path,"\\www");
  strcat(path,url);

  if (url[strlen(url) - 1] == '/')
    strcat(path, "index.html");  
  printf("location file:%s\n",path); 
  //是否存在文件
  if (stat(path, &st) == -1)
  { 
    if (numchars > 0)  
      discardheaders(client);  
    not_found(client);  
  }  
  else  
  { 
    if ((st.st_mode & S_IFMT) == S_IFDIR)  
      strcat(path, "/index.html");  
    serve_file(client, path);  
  }  
  
  closesocket(client);  
}  
  
/**********************************************************************/  
/*请求出错*/
/**********************************************************************/  
void bad_request(int client)  
{  
  char buf[1024];  
  sprintf(buf,  "HTTP/1.0 400 BAD REQUEST\r\n");  
  send(client, buf, sizeof(buf), 0);  
  sprintf(buf, "Content-type: text/html\r\n");  
  send(client, buf, sizeof(buf), 0);  
  sprintf(buf, "\r\n");  
  send(client, buf, sizeof(buf), 0);  
  sprintf(buf, "<P>Your browser sent a bad request, ");  
  send(client, buf, sizeof(buf), 0);  
  sprintf(buf, "such as a POST without a Content-Length.\r\n");  
  send(client, buf, sizeof(buf), 0);  
}  
  
/**********************************************************************/  
/* 输出文件内容*/  
/**********************************************************************/  
void cat(SOCKET client, FILE *resource)  
{  
  char buf[1024];  

  fgets(buf, sizeof(buf), resource);  
  while (!feof(resource))  
  {  
    send(client, buf, strlen(buf), 0);  
    fgets(buf, sizeof(buf), resource);  
  }  
}  
  
/**********************************************************************/  
/*不支持动态程序处理*/
/**********************************************************************/  
void cannot_execute(int client)  
{  
  char buf[1024];  

  sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");  
  send(client, buf, strlen(buf), 0);  
  sprintf(buf, "Content-type: text/html\r\n");  
  send(client, buf, strlen(buf), 0);  
  sprintf(buf, "\r\n");  
  send(client, buf, strlen(buf), 0);  
  sprintf(buf, "<P>Error prohibited CGI execution.</p>\r\n");  
  send(client, buf, strlen(buf), 0);  
}  
  
/**********************************************************************/  
/*服务出错并中止运行*/
/**********************************************************************/  
void error_die(const char *sc)  
{  
  perror(sc);  
  exit(1);  
}  
  
/**********************************************************************/  
/* 从http请求中读一行数据
   返回:此行数据长度
 */  
/**********************************************************************/  
int get_line(SOCKET sock, char *buf, int size)  
{  
 int i = 0;  
 char c = '\0';  
 int n;  
  
 while ((i < size - 1) && (c != '\n'))  
 {  
  n = recv(sock, &c, 1, 0);  
  if (n > 0)  
  {  
   if (c == '\r')  
   {  
    n = recv(sock, &c, 1, MSG_PEEK);  
    if ((n > 0) && (c == '\n'))  
     recv(sock, &c, 1, 0);  
    else  
     c = '\n';  
   }  
   buf[i] = c;  
   i++;  
  }  
  else  
   c = '\n';  
 }  
 buf[i] = '\0';  
   
 return(i);  
}  
  
/**********************************************************************/  
/* http头输出 */
/**********************************************************************/  
void headers(SOCKET client)  
{  
 char buf[1024];  
 strcpy(buf, "HTTP/1.0 200 OK\r\n");  
 send(client, buf, strlen(buf), 0);  
 strcpy(buf, SERVER_STRING);  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "Content-Type: text/html\r\n");  
 send(client, buf, strlen(buf), 0);  
 strcpy(buf, "\r\n");  
 send(client, buf, strlen(buf), 0);  
}  
  
/**********************************************************************/  
/* 404错误输出 */  
/**********************************************************************/  
void not_found(SOCKET client)  
{  
 char buf[1024];  
  
 sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, SERVER_STRING);  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "Content-Type: text/html\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "<BODY><P>The server could not fulfill\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "your request because the resource specified\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "is unavailable or nonexistent.\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "</BODY></HTML>\r\n");  
 send(client, buf, strlen(buf), 0);  
}  
  
/**********************************************************************/  
/* 获取本地方 判断文件是否存在 */  
/**********************************************************************/  
void serve_file(SOCKET client, const char *filename)  
{  
 FILE *resource = NULL;  
 char buf[1024];  
  
 discardheaders(client);  
 resource = fopen(filename, "r");  
 if (resource == NULL)  
  not_found(client);  
 else  
 {  
  headers(client);  
  cat(client, resource);  
 }  
 fclose(resource);  
}  
  
  
/**********************************************************************/  
/* 丢弃一行数据*/  
/**********************************************************************/  
void discardheaders(SOCKET client)  
{  
    char buf[1024];  
    int numchars = 1;  
    while ((numchars > 0) && strcmp("\n", buf)) 
        numchars = get_line(client, buf, sizeof(buf));  
}  
  
/**********************************************************************/  
/* 启动服务并进行相关设置 */  
/**********************************************************************/  
SOCKET startup(u_short *port)  
{  
  SOCKET httpd = 0;  
  struct sockaddr_in name;  

  httpd = socket(AF_INET, SOCK_STREAM, 0);  
  if (httpd == INVALID_SOCKET)  
    error_die("socket");  
  memset(&name, 0, sizeof(name));  
  name.sin_family = AF_INET;  
  name.sin_port = htons(*port);  
  name.sin_addr.s_addr = inet_addr("127.0.0.1");  
  if (bind(httpd, (struct sockaddr *)&name, sizeof(name)) < 0)  
    error_die("bind");  
  if (*port == 0)  /* 随机生成端口*/  
  {  
    int namelen = sizeof(name);  
    if (getsockname(httpd, (struct sockaddr *)&name, &namelen) == -1)  
      error_die("getsockname");  
    *port = ntohs(name.sin_port);  
  }  
  if (listen(httpd, 5) < 0)  
    error_die("listen");  
  return(httpd);  
}  
  
/**********************************************************************/  
/* 不支持其它Http动词 */  
/**********************************************************************/  
void unimplemented(SOCKET client)  
{  
 char buf[1024];  
  
 sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, SERVER_STRING);  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "Content-Type: text/html\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "</TITLE></HEAD>\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");  
 send(client, buf, strlen(buf), 0);  
 sprintf(buf, "</BODY></HTML>\r\n");  
 send(client, buf, strlen(buf), 0);  
}  
  
/**********************************************************************/  
int main(int argc,char **argv)  
{
  //输入参数不正确
  if(argc != 2) 
  { 
    printf("Usage :%s PORT\n", argv[0]);
    exit(0);
  }

  SOCKET server_sock = INVALID_SOCKET;  
  //设置服务端口
  u_short port = atoi(argv[1]);  
  SOCKET client_sock = -1;  
  struct sockaddr_in client_name;  
  int client_name_len = sizeof(client_name);  
 
  #ifdef MINGW32
  WSADATA wsaData;  
  WSAStartup(MAKEWORD(2,2), &wsaData); 
  #endif 

  server_sock = startup(&port);  
  printf("httpd running on port %d\n", port);  

  while (1)  
  {  
    client_sock = accept(server_sock,  
                         (struct sockaddr *)&client_name,  
                         &client_name_len);  
    if (client_sock == INVALID_SOCKET)  
      error_die("accept");  
    //处理请求
    accept_request(client_sock);
  }  

  closesocket(server_sock); 

  #ifdef MINGW32
  WSACleanup();  
  #endif 
  return(0);  
}  
