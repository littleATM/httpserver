#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<sys/select.h>
#include<sys/time.h>
//#include"fd_list.h"
#include"load_config.h"
static void handle_task(int ,int, void*);
int main(int argc, char* argv[]){
    int socket_fd;
    if((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))< 0){
        printf("socket error %s\n", strerror(errno));
        exit(1);
    }
    struct sockaddr_in servaddr;
    if(!load_config(&servaddr)){
        printf("config file error\n");
        exit(0);
    }
//    servaddr.sin_family = AF_INET;
//    char* addr = "23.105.194.17";
//    if(inet_aton(addr, (struct in_addr*)&servaddr.sin_addr) == 0){
//        printf("aton error %s\n", strerror(errno));
//        exit(1);
//    }
//    servaddr.sin_port = htons(80);
    if(bind(socket_fd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
        printf("bind error %s\n", strerror(errno));
        exit(1);
    }
    if(listen(socket_fd, 100) < 0){
        printf("listen error %s\n", strerror(errno));
        exit(1);
    }
    struct sockaddr_in cliaddr;
    int size = sizeof(cliaddr);
    int len; 
    struct sockaddr_in localaddr; 
    len = sizeof(struct sockaddr_in);
    getsockname(socket_fd, (struct sockaddr*)&localaddr, &len);
    uint16_t port = ntohs(localaddr.sin_port);
    char* addr = inet_ntoa(localaddr.sin_addr);
    printf("addr: %s port: %d\n", addr, port);
    int page_fd = open("./page/test.html",O_RDONLY);
    if (page_fd == -1){
        printf("page file open error %s\n", strerror(errno));
        exit(1);
    }
    int file_size = get_file_size(page_fd);
    void* file_addr = mmap(0, file_size, PROT_READ, MAP_SHARED, page_fd, 0);
    if(file_addr == MAP_FAILED){
        printf("file map error %s\n", strerror(errno));
        exit(1);
    }
    int max_fd = socket_fd;
    fd_set readable;
    FD_ZERO(&readable);
    FD_SET(socket_fd, &readable);
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
   // struct fd_list list ;
    //list.head=NULL;
    //list.tail = NULL;
    //list.nodes_count = 0;
    //add_fd(socket_fd, &list);
    for(;;){
        fd_set temp = readable;
        if(select(max_fd+1, &readable, NULL, NULL, NULL) < 0){
            printf("select error\n");
        }
        int ready_fd;
        for(ready_fd = 0;ready_fd<=max_fd;ready_fd++){
           if(FD_ISSET(ready_fd, &readable)){
               break;
           } 
        }
        readable = temp;
        if(ready_fd == socket_fd){
             int serv_fd = accept(socket_fd, (struct sockaddr*)&cliaddr, &size);
             if(serv_fd == -1) continue;
             FD_SET(serv_fd, &readable);
             max_fd = serv_fd>max_fd?serv_fd:max_fd;
        } else {
            handle_task(ready_fd, file_size, file_addr);
            FD_CLR(ready_fd, &readable);
        }
   }
}
void handle_task(int serv_fd, int file_size, void* file_addr){
        int pid = fork();
        if(pid != 0){
            if(pid == -1){
                printf("pid error  %s\n", strerror(errno));
                exit(1);
            }
            close(serv_fd);
            printf("pid %d\n", pid);
            return;
        }
        char *buf = malloc(1000);
        if(memset(buf, '\0', 1000) == NULL){
            printf("memset error %s\n", strerror(errno));
            exit(1);
        }
        struct sockaddr_in romate_addr; 
        int len = sizeof(struct sockaddr_in);
        getpeername(serv_fd, (struct sockaddr*)&romate_addr, &len);
        int port = ntohs(romate_addr.sin_port);
        char* addr = inet_ntoa(romate_addr.sin_addr);
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
        
        char html[] ="HTTP/1.1 200 OK\r\nUser-Agent: curl/7.16.3 libcurl/7.16.3 OpenSSL/0.9.7l zlib/1.2.3\r\nHost: www.example.com\r\nAccept-Language: en, mi\r\nDate: Mon, 27 Jul 2009 12:28:53 GMT\r\nServer: Apache\r\nLast-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\nnAccept-Ranges: bytes\r\nVary: Accept-Encoding\r\nContent-Type: text/html\r\n\r\n";
        int nleft = (int)sizeof(html);
        strcpy(buf, html);
         while(nleft > 0){
            n = write(serv_fd, buf, nleft);
            printf("write size : %d\n", n);
            if(n < 0){
                printf("write  error %s\n", strerror(errno));
                exit(1);
            }
            nleft -= n;
            buf += n;
        }
        while(file_size > 0){
            printf("page size: %d\n", file_size);
            n = write(serv_fd, file_addr, file_size);
            printf("write size : %d\n", n);
            if(n < 0){
                printf("write  error %s\n", strerror(errno));
                exit(1);
            }
            file_size -= n;
            file_addr += n;
        }
        close(serv_fd);
        free(buf);
}
int get_file_size(int fd){
    FILE* f = fdopen(fd, "r");
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}
