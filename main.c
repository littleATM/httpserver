#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#define READ_MAX_LENGTH 10000
int main(int argc, char* argv[]){
    if (argc < 2){
        printf("arg error\n");
        exit(1);
    }
    int i=1;
    for(i = 1; i < argc; i++){
        printf("%s\n",argv[i]);
    }
    printf("\n");
    int fd;
    if((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("socket error %s\n",strerror(errno));
        exit(1);
    }
    struct sockaddr_in ipv4;
    ipv4.sin_family = AF_INET;
    const char* addr= argv[1];
    uint16_t port= atoi(argv[2]);
    if(inet_aton(addr,&ipv4.sin_addr) <= 0){
        printf("inet_aton error\n");
        exit(1);
    }
    ipv4.sin_port = htons(port);
    int connfd = 0;
    if((connfd = connect(fd, (const struct sockaddr*)&ipv4, sizeof(ipv4))) < 0){
        printf("connect error %s\n",strerror(errno));
        exit(1);
    }
    //const char* in = "GET / HTTP/1.1\nHost: www.sohu.com\nConnection: keep-alive\nCache-Control: max-age=0\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\nUpgrade-Insecure-Requests: 1\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.86 Safari/537.36\nAccept-Encoding: gzip, deflate, sdch\nAccept-Language: zh-CN,zh;q=0.8,en;q=0.6\nIf-Modified-Since: Sat, 07 May 2016 20:54:12 GMT";
   // const char* in = "GET / HTTP/1.1\r\nHost: www.sohu.com\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8;charset=utf-8\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.86 Safari/537.36\r\nReferer: http://www.hao123.com/?tn=82013038_3_hao_pg\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\nCookie: vjuids=d1dc1caf.153026e9faa.0.dd318303; Hm_lvt_c154d502f02b12a996c34a02281a3bff=1458130195; _smuid=7nlWzwkqB6JnbUgVXlAUW; NUV=1460044800000; DIFF=1460026781907; IPLOC=CN88; SUV=1602211305292373\r\nIf-Modified-Since: Sun, 08 May 2016 14:38:07 GMT\r\n\r\n"
    const char* in ="GET / HTTP/1.1\r\nHost: 202.108.22.5\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.86 Safari/537.36\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\nCookie: BD_HOME=0; BD_UPN=123353\r\n\r\n"// "GET / HTTP/1.1\r\nHost: edition.cnn.com\r\n\r\n"
        ;
    int nleft = strlen(in);
    int n = 0;
    while((n = write(fd, in, nleft)) > 0){
        printf("write byte num: %d \n", n);
        nleft -= n;
        in += n;
        if(nleft ==0){
            break;
        }
    }
    printf("write is over\n");
    if(nleft<0){
        printf("write error\n");
        exit(0);
    }
    char out[READ_MAX_LENGTH];
    while((n = read(fd, out, READ_MAX_LENGTH - 1))>0){
        //out[READ_MAX_LENGTH - 1]=0;
        fputs(out,stdout);
    }
    if(n < 0){
        printf("read error:%s\n",strerror(errno));
    }
    exit(0);
}
