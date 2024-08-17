# DigitalMemories
基于GEC6818开发板开发的电子相册

## 环境配置

### alsa库配置

需要两个,一个是依赖,一个是utlis

1. 先安装依赖lib

2. 安装utils





#### lib的安装

下载alsa-lib-1.0.22.tar.bz2

解压

```shell
tar -zjvf alsa-lib-1.0.22.tar.bz2
```

解压后进入alsa, 进去目录

```shell
 1. cd alsa-lib-1.0.22/
 2. ./configure --prefix=/usr/local/alsa --host=arm-linux --disable-python
   # 这一步可能会有问题,找不到arm-linux, 这种情况多数出现在arm-linux装在了用户的usr里去而不是系统的,所以需要使用export来设置下环境变量
 3. make
 4. sudo make install
```



配置和就可以在/usr/local/alsa 找到了









#### utils的安装

三步走:

1. ./configura 后面配置参数,需要注意的是可以--按tab 然后看看需要的参数,这里需要with-上面lib的include和lib位置
2. make
3. make install

```shell
./configure --prefix=/usr/local/alsa \
        --host=arm-linux \
        --with-alsa-prefix=/usr/local/alsa/lib/ \
        --with-alsa-inc-prefix=/usr/local/alsa/include/ \
        --disable-alsamixer \
        --disable-xmlto    
        prefix=/home/gec/alsa/alsa-lib-1.2.9/build/include/ --with-alsa-prefix=/home/gec/alsa/alsa-lib-1.2.9/build/lib 
        --disable-alsamixer --disable-xmlto
        
make
# 会出现找不到文件夹的错误,可以手动创建

sudo make install
```

alsamixer 是一个ncurses程序,基本上在嵌入式终端上很难移植.所以这里取消掉.--disable-xmlto也是因为找不到库.





#### 移植开发板

使用下面命令

```shell
cd /usr/local/
tar -czvf alsa.tar.gz alsa/
```

然后发送到开发板

进入开发板后

```shell
tar -zxvf alsa.tar.gz
cp alsa/bin/arecord alsa/bin/aplay /bin

# 将alsa/share/alsa复制到开发板的/usr/local/alsa/share(若没有，则创建)中，方法如下：
mkdir -p /usr/local/alsa
cp -r share /usr/local/alsa/
#看看是否复制成功
ls /usr/local/alsa/share
# 将/alsa/lib/libasound.so.2 拷贝到开发板中/lib中
cp alsa/lib/libasound.so.2 /lib/


```





执行shell脚本

```shell
#!/bin/sh

# 创建 /dev/snd 目录
mkdir -p /dev/snd

# 进入 /dev/snd 目录
cd /dev/snd

# 创建设备节点
mknod dsp c 14 3
mknod audio c 14 4
mknod mixer c 14 0
mknod controlC0 c 116 0
mknod seq c 116 1
mknod pcmC0D0c c 116 24
mknod pcmC0D0p c 116 16
mknod pcmC0D1c c 116 25
mknod pcmC0D1p c 116 17
mknod timer c 116 33

```

没有权限的话可以 chmod 777 xx.sh



测试:

录音

```c
arecord -d3 -c1 -r16000 -twav -fS16_LE 1.wav
```



播放

```
aplay  1.wav
```







### jpeg库配置

#### 下载对应的jpeg库并解压缩

这一步略过....



#### 生成Makefile文件

```shel
gec@ubuntu:~$ ./configure --prefix=/xxx --host=arm-linux
```

- 默认安装到 /usr/local 下，可以通过 --prefix= 来指定安装路径
- 默认使用的编译器是gcc，可以通过 --host= 来指定特定编译器的前缀



在生成Makefile时可以通过--help来查看一些所需参数

```shell
gec@ubuntu:~/jpeg-9a$ ./configure --help
Installation directories:
  --prefix=PREFIX         install architecture-independent files in PREFIX
                          [/usr/local]
  --exec-prefix=EPREFIX   install architecture-dependent files in EPREFIX
                          [PREFIX]

Fine tuning of the installation directories:
  --bindir=DIR            user executables [EPREFIX/bin]
  --sbindir=DIR           system admin executables [EPREFIX/sbin]
  --libexecdir=DIR        program executables [EPREFIX/libexec]
  ...
  ...
```



#### 编译

```shell
gec@ubuntu:~$ make
```



#### 安装

```shell
gec@ubuntu:~$ make install
```



#### 安装成功标志

```shell
gec@ubuntu:~/jpg$ tree
.
├── bin/
│   ├── cjpeg
│   ├── djpeg
│   ├── jpegtran
│   ├── rdjpgcom
│   └── wrjpgcom
├── include/
│   ├── jconfig.h
│   ├── jerror.h
│   ├── jmorecfg.h
│   └── jpeglib.h
├── lib/
│   ├── libjpeg.a
│   ├── libjpeg.la
│   ├── libjpeg.so -> libjpeg.so.9.1.0
│   ├── libjpeg.so.9 -> libjpeg.so.9.1.0
│   └── libjpeg.so.9.1.0
└── share/
    └── man/
        └── man1/
            ├── cjpeg.1
            ├── djpeg.1
            ├── jpegtran.1
            ├── rdjpgcom.1
            └── wrjpgcom.1
6 directories, 19 files
```





#### 移植到开发板

- 将编译好的所有文件进行压缩

```
gec@ubuntu:~$ tar czvf jpeg.tar.gz /home/gec/jpg/
```



- 将压缩包 jpeg.tar.gz 传输到开发板。
- 在开发板中解压压缩包：

```
[root@GEC6818:~]# tar xzvf jpeg.tar.gz -C /root/jpg
```



- 分别设置 PATH、 LD_LIBRARY_PATH 环境变量：

```
[root@GEC6818:~]# export PATH=$PATH:/root/jpg/bin
[root@GEC6818:~]# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/root/jpg/lib
```

弄完就算配置完成了



 







## 数码相册

模块:

1. 读取目录模块
2. 图片预处理模块
3. 图片解码模块
4. 滑动模块
5. 语言识别模块







### 语言识别模块

分为:

1. 客户端开发
2. 本地服务器开发
3. 远程API接口提供



#### 客户端开发

```c
#ifndef __CLIENT_H
#define __CLIENT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>

static int send_wav()
{
    int sockfd;
    struct sockaddr_in servaddr;
    FILE *wavFile;
    // 记录wav的地址
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
    servaddr.sin_addr.s_addr = inet_addr("192.168.72.27");
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
        /* code */
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

#endif
```

1. 用户触发事件
2. 开始录音
3. 将wav文件发送到服务端





#### 本地服务端模块

使用python实现:

直接从开发板上接收了wav文件后发送给远程的API接口,接收到API的返回值后,返回对应的图片序号

```python
import socket
import requests
import struct

temp_wav_file = "./temp.wav"

while True:
    # 每次循环开始时，创建并绑定新的套接字
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = ("0.0.0.0", 50001)
    sock.bind(server_address)
    print("Server is running and waiting for audio data...")

    received_data = b""

    while True:
        data, address = sock.recvfrom(1024)
        if data:
            received_data += data

            if len(data) < 1024:
                break

    with open(temp_wav_file, "wb") as wav_file:
        wav_file.write(received_data)

    print(f"Received complete WAV file from {address}")

    # 发送接收到的WAV文件到http://localhost:6006/find_image
    with open(temp_wav_file, "rb") as wav_file:
        files = {"file": wav_file}
        image_search_response = requests.post(
            "http://localhost:6006/find_image", files=files
        )

    if image_search_response.status_code == 200:
        # 获取返回的整数结果
        image_result = int(image_search_response.text.strip())
        print(f"Image search result: {image_result}")
    else:
        print(
            f"Failed to get image search result. Status code: {image_search_response.status_code}"
        )
        sock.close()
        continue

    # 将整数转换为网络字节序的四字节数据并发送
    response = struct.pack("!I", image_result)
    sock.sendto(response, address)
    print(f"Sent reply '{image_result}' to {address}")

    # 关闭当前的套接字
    sock.close()
    print("Connection closed. Restarting server...")

```



#### 远程部分

需要下载的库:

```
transformers
pickle
flask
faster_whisper
tempfile
```



使用了的模型:

1. 阿里开源的OFA-Sys/chinese-clip-vit-base-patch16模型
2. openai开源的faster-whisper-large-v3模型



使用python开发API,代码如下:

app.py

```python
import os
from PIL import Image
from transformers import ChineseCLIPProcessor, ChineseCLIPModel
import torch
import pickle
from flask import Flask, request
from faster_whisper import WhisperModel
import tempfile

# 加载CLIP模型和处理器
model = ChineseCLIPModel.from_pretrained("OFA-Sys/chinese-clip-vit-base-patch16")
processor = ChineseCLIPProcessor.from_pretrained("OFA-Sys/chinese-clip-vit-base-patch16")

# 加载Whisper模型
whisper_model_dir = '/root/.cache/modelscope/hub/keepitsimple/faster-whisper-large-v3'
whisper_model = WhisperModel(whisper_model_dir)

# 指定本地图像目录
image_directory = "./img"

# 读取目录下的所有图像文件并计算图像向量
image_vectors = []
image_paths = []

for filename in os.listdir(image_directory):
    if filename.endswith((".png", ".jpg", ".jpeg", ".bmp", ".gif")):
        # 构造图像路径
        image_path = os.path.join(image_directory, filename)
        image_paths.append(image_path)
        
        # 加载图像并转为向量
        image = Image.open(image_path)
        inputs = processor(images=image, return_tensors="pt")
        image_features = model.get_image_features(**inputs)
        image_features = image_features / image_features.norm(p=2, dim=-1, keepdim=True)  # 归一化
        image_vectors.append(image_features)

# 将图像向量保存到文件中
with open('image_vectors.pkl', 'wb') as f:
    pickle.dump((image_paths, image_vectors), f)

# 创建Flask应用
app = Flask(__name__)

# 查找最相似图像的路由
@app.route('/find_image', methods=['POST'])
def find_similar_image():
    # 接收WAV文件
    wav_file = request.files.get('file')
    
    # 将WAV文件保存到临时目录
    with tempfile.NamedTemporaryFile(delete=False, suffix=".wav") as temp_wav:
        wav_file.save(temp_wav.name)
        temp_wav_path = temp_wav.name
    
    # 使用fast_whisper解析WAV文件
    segments, info = whisper_model.transcribe(temp_wav_path)
    text = " ".join(segment.text for segment in segments)
    print(text)
    # 删除临时WAV文件
    os.remove(temp_wav_path)
    
    # 加载保存的图像向量
    with open('image_vectors.pkl', 'rb') as f:
        image_paths, image_vectors = pickle.load(f)
    
    # 处理解析出的文本并转为向量
    inputs = processor(text=[text], return_tensors="pt")
    text_features = model.get_text_features(**inputs)
    text_features = text_features / text_features.norm(p=2, dim=-1, keepdim=True)  # 归一化
    
    # 计算文本向量与所有图像向量的相似度
    similarities = []
    for image_vector in image_vectors:
        similarity = torch.nn.functional.cosine_similarity(text_features, image_vector)
        similarities.append(similarity.item())
    
    # 找到最相似的图像
    best_match_index = similarities.index(max(similarities))
    best_match_path = image_paths[best_match_index]
    
    # 提取并返回文件名前面的数字部分
    best_match_filename = os.path.basename(best_match_path)
    best_match_number = best_match_filename.split('.')[0]  # 提取数字部分
    return best_match_number  # 返回数字部分


# 运行Flask应用，监听端口6006
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6006)

```





由于要访问huggingface,需要科学上网,可以使用autodl的服务器,他们提供了学术资源加速服务,下面是启动命令

```shell
source /etc/network_turbo
```



然后运行命令:

```shell
python app.py
```



