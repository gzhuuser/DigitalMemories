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
processor = ChineseCLIPProcessor.from_pretrained(
    "OFA-Sys/chinese-clip-vit-base-patch16"
)

# 加载Whisper模型
whisper_model_dir = "/root/.cache/modelscope/hub/keepitsimple/faster-whisper-large-v3"
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
        image_features = image_features / image_features.norm(
            p=2, dim=-1, keepdim=True
        )  # 归一化
        image_vectors.append(image_features)

# 将图像向量保存到文件中
with open("image_vectors.pkl", "wb") as f:
    pickle.dump((image_paths, image_vectors), f)

# 创建Flask应用
app = Flask(__name__)

# 查找最相似图像的路由
@app.route("/find_image", methods=["POST"])
def find_similar_image():
    # 接收WAV文件
    wav_file = request.files.get("file")

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
    with open("image_vectors.pkl", "rb") as f:
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
    best_match_number = best_match_filename.split(".")[0]  # 提取数字部分
    return best_match_number  # 返回数字部分


# 运行Flask应用，监听端口6006
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=6006)
