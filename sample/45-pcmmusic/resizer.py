from PIL import Image
from io import BytesIO

with BytesIO() as f:
    image = Image.open('alisa1.jpg')
    new_img = image.resize((640, 480)).convert('RGBA')
    new_img.save(f, 'BMP')

with open('alisa1-raw.img', 'wb') as f:
    f.write(new_img.tobytes())
