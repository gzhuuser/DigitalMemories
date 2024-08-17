#include "client.h"

// send_wav函数的实现
int send_wav()
{
    int sockfd;
    struct sockaddr_in servaddr;
    FILE *wavFile;
    char *wavPath = "1.wav";
    char buffer[1024];
    size_t bytesRead;

    // 创建UDP套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 初始化servaddr
    memset(&servaddr, 0, sizeof(servaddr));

    // 配置服务器地址
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.72.49");
    servaddr.sin_port = htons(50001);

    // 打开wav文件
    wavFile = fopen(wavPath, "rb");
    if (wavFile == NULL)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    // 组块读取文件并发送
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), wavFile)) > 0)
    {
        sendto(sockfd, buffer, bytesRead, 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    }
    fclose(wavFile);
    printf("WAV file sent to server.\n");

    // 接收服务器的回复
    int n, len;
    len = sizeof(servaddr);
    int reply;

    n = recvfrom(sockfd, &reply, sizeof(reply), 0, (struct sockaddr *)&servaddr, &len);
    if (n < 0)
    {
        perror("recvfrom failed");
    }
    else
    {
        printf("Server reply: %d\n", ntohl(reply)); // 转换为主机字节序
    }

    // 关闭套接字
    close(sockfd);
    return ntohl(reply);
}
