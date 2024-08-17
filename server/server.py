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
