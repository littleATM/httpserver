#include"../load_config.h"
int main(){
    struct sockaddr_in servaddr;
    load_config(&servaddr);
    exit(0);
}
