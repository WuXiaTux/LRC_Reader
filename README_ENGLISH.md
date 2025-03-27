[中文](https://github.com/WuXiaTux/LRC_Reader/blob/main/README.md) [English](https://github.com/WuXiaTux/LRC_Reader/blob/main/README_ENGLISH.md)

### The English skill of me is not good,so if you find some grammar mistake or something you can't understand,please use translation software to translate the Chinese version of this readme.<br><br>
### The systems and languages of this software:<br>
This software only support Windows7 and upgrader Windows system,support 32 or 64 base system,and it only support Chinese and English.<br>
### This information of this software:<br>
1.change TXT file to LRC file,and create MP3 file according to the line numbers of the TXT file.<br>
(1)File Separation Mode:Separate TXT file into 500 lines,and create corresponding MP3 file.<br>
2.using ascii-image-converter change picture file to LRC and MP3 file combination,to display ASCII picture.<br>
(This mode is not mature,now it only support Listener M2/M2S and it need more develop.)<br><br>
### The attention points develop and compile software:<br>
1.The computer system of me is Windows LTSC 10.0.19044 build 19044 64 base system.<br>
2.This software is written by C++,using EGE graphics library,develop by XiaoXiongMaoC++(IDE),so I suggest you [download this IDE](http://royqh.net/redpandacpp/download/) to develop this software conveniently.<br>
3.To fitter 32 base system,don't change ffmpeg.exe,because FFMPEG offical didn't support 32 base system now,please use this lastest version.Also,if you change ascii-image-converter.exe,please using 32 base version.<br>
4.This is my compile opinion:<br>
g++.exe -c main.cpp -o main.o   -O2 -pipe -lwinmm -std=c++17 -lComdlg32  -finput-charset=UTF-8 -fexec-charset=gbk <br>
windres.exe -i v3_private.rc --input-format=rc -o v3_private.res -O coff <br>
g++.exe main.o v3_private.res -o v3.0.exe -mwindows -s -mwindows -lgraphics -luuid -lmsimg32 -lgdi32 -limm32 -lole32 -loleaut32 -lwinmm -lgdiplus -static
