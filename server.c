#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
int main(int argc, char* argv[]){
    int socket_fd;
    if((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))< 0){
        printf("socket error %s\n", strerror(errno));
        exit(1);
    }
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    char* addr = "23.105.194.17";
    if(inet_aton(addr, (struct in_addr*)&servaddr.sin_addr) == 0){
        printf("aton error %s\n", strerror(errno));
        exit(1);
    }
    servaddr.sin_port = htons(80);
    if(bind(socket_fd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("bind error %s\n", strerror(errno));
        exit(1);
    }
    if(listen(socket_fd, 100) < 0){
        printf("listen error %s\n", strerror(errno));
        exit(1);
    }
    struct sockaddr_in cliaddr;
    char *buf = malloc(1000);
    void* temp = buf;
    int size = sizeof(cliaddr);
    int len; 
    struct sockaddr_in localaddr; 
    len = sizeof(struct sockaddr_in);
    uint16_t port;
    getsockname(socket_fd, (struct sockaddr*)&localaddr, &len);
    port = ntohs(localaddr.sin_port);
    addr = inet_ntoa(localaddr.sin_addr);
    printf("addr: %s port: %d\n", addr, port);
    for(;;){
        
        buf = temp;
        if(memset(buf, '\0', 1000) == NULL){
            printf("memset error %s\n", strerror(errno));
            exit(1);
        }
        int serv_fd = accept(socket_fd, (struct sockaddr*)&cliaddr, &size);
        int pid = fork();
        if(pid != 0){
            if(pid == -1){
                printf("pid error  %s\n", strerror(errno));
                exit(1);
            }
            close(serv_fd);
            printf("pid %d\n", pid);
            continue;
        }
        struct sockaddr_in romate_addr; 
        len = sizeof(struct sockaddr_in);
        getpeername(serv_fd, (struct sockaddr*)&romate_addr, &len);
        port = ntohs(romate_addr.sin_port);
        addr = inet_ntoa(romate_addr.sin_addr);
        printf("romate addr: %s port: %d\n", addr, port);
        if(serv_fd == -1){
            printf("accept error %s\n", strerror(errno));
            exit(1);
        }
        int offset = 0;

        int n = 0;
        while(( n = read(serv_fd, buf+offset, 1000))>0){
            printf("read: %d\n", n);
            offset += n;
            int pre = offset-n - 4;
            int back = offset;
            pre = pre < 0 ? 0 : pre;
            int flag = 0;
            while(pre+4<=back){
                if(buf[pre++] != '\r') continue;
                if(buf[pre++] != '\n') continue;
                if(buf[pre++] != '\r') continue;
                if(buf[pre++] != '\n') continue;
                flag = 1;
                break;
            }
            if(flag) break;
        };
        if(n == 0)printf("fin get\n");
        if(n<0){
            printf("read error %s\n", strerror(errno));
            exit(1);
        }
        fputs(buf, stdout);
        
        char html[] ="HTTP/1.1 200 OK\r\nUser-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\nHost: www.example.com\r\nAccept-Language: en, mi\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nnAccept-Ranges: bytes\r\nVary: Accept-Encoding\r\nContent-Type: text/html\r\n\r\n<html>\n<head>\n</head>\n<body>\n<h1>\nSun Yulong\n</h1>\n</body>\n</html>";
        int nleft = (int)sizeof(html);
        strcpy(buf, html);
        while(nleft > 0){
            n = write(serv_fd, buf, nleft);
            if(n < 0){
                printf("write  error %s\n", strerror(errno));
                exit(1);
            }
            nleft -= n;
            buf += n;
        }
        close(serv_fd);
        exit(0);
    }
}
