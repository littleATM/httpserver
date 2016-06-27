#include"load_config.h"
#include<fcntl.h>
#include<sys/stat.h>
#include<string.h>
#include<errno.h>
#define IP 0
#define PORT 1
#define NONE 2
int load_config(struct sockaddr_in* servaddr){
    int fd;
    if((fd = open("./config", O_RDONLY)) < 0){
        printf("open config file error : %s\n",strerror(errno));
        exit(1);
    }
    struct stat stat_buf;
    if(fstat(fd, &stat_buf) < 0){
        printf("fstat error :%s\n", strerror(errno));
        exit(1);
    }
    if(stat_buf.st_size > 2000){
        printf("config file is too large");
        exit(1);
    }
    void* file_buf = malloc(stat_buf.st_size);
    if(file_buf == NULL){
        printf("malloc error : %s\n:", strerror(errno));
        exit(1);
    }
    memset(file_buf, 0, stat_buf.st_size);
    while(1){
        void* temp_file_buf = file_buf;
        int n;
        n = read(fd, temp_file_buf, 1000);
        if(n == 0) break;

        temp_file_buf+= n;
    }
    int i = 0; 
    for(i = 0;i<stat_buf.st_size;i++){
       if(*(char*)(file_buf+i) == ' '||*(char*)(file_buf+i) == '\n'||*(char*)(file_buf+i) == '\r'||*(char*)(file_buf+i) == ';') *(char*)(file_buf+i) = '\0';
    }
    servaddr->sin_family = AF_INET;
    int flag = NONE;
    int has_ip = 0;
    int has_port = 0;
    for(i = 0;i<stat_buf.st_size;i++){
        if(*(char*)(file_buf+i) != '\0'){
            if(strcmp((char*)(file_buf+i), "ip") == 0){
                while(*(char*)(file_buf+i) != '\0') i++;
                flag = IP;
                continue;
            }else if(strcmp((char*)(file_buf+i), "port") == 0){
                while(*(char*)(file_buf+i) != '\0') i++;
                flag = PORT;
                continue;
            }else if(flag == IP){
                if(inet_aton((char*)(file_buf+i), (struct in_addr*)&servaddr->sin_addr) == 0){
                    printf("aton error %s\n", strerror(errno));
                    exit(1);
                }
                while(*(char*)(file_buf+i) != '\0') i++;
                flag = NONE;
                has_ip = 1;
                continue;
            }else if(flag == PORT){
                servaddr->sin_port = htons(atoi((char*)(file_buf+i)));
                while(*(char*)(file_buf+i) != '\0') i++;
                flag = NONE;
                has_port = 1;
                continue;
            }
        }
    }
    if(has_ip&&has_port) return 1;
    return 0;
}
