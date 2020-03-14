import os

# current directory
repo_dir = "C:/Users/Administrator/Desktop/Flyception_main/"

xml_file = [
"images_list_1.xml",
"images_list_2.xml",
"images_list_3.xml",
"images_list_4.xml"
]

image_dir = [
"03",
"07",
"62",
"83"
]



for i in range(4):
    with open(repo_dir + xml_file[i], 'w+') as f:
        print('<?xml version="1.0"?>\n<opencv_storage>\n<images>', file=f)

        for name in os.listdir(repo_dir + image_dir[i]):
            if name.endswith(".jpg"):
                print(repo_dir + image_dir[i] + '/' + name, file=f)

        print('</images>\n</opencv_storage>', file=f)

