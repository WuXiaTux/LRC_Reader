#include <graphics.h>
#include <ege/sys_edit.h>
#include <math.h>
#include<iostream>
#include<windows.h>
#include<fstream>
#include<vector>
#include<string.h>
#include<cstring>
#include<algorithm>
#include<cstdio>
#include <random>
#include <chrono>
#include <direct.h>
#include <commdlg.h> 
#include <tchar.h> 
#include <filesystem>
#include <locale.h>
using namespace std;
//全局变量定义
string str1="ffmpeg -stream_loop ",a,b,temp2,str2,opt;
vector<string> v;

int i=0,N=0;

//函数定义
void main_ui();
void enter_program();
void create_music();
void create_lrc();
void create_lrc2();
void delall();
void backdir();
//TestEOL类定义
struct TestEOL
{
	bool operator()(char c)
	{
		las=c;
		return las=='\n';
	}
	char las;
};
//文件选择对话框函数
string open_file_dialog() {
    OPENFILENAME ofn;
    char szFile[260] = { 0 }; // 文件名缓冲区
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0"; 
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrTitle = _T("选择TXT文件");
    ofn.lpstrInitialDir = _T("C:\\"); // 设置默认目录
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    if (GetOpenFileName(&ofn)) {
        return szFile;
    } else {
        return "";
    }
}
void backdir()
{
	char buffer[MAX_PATH];

    // 获取当前执行程序的路径
    GetModuleFileNameA(NULL, buffer, MAX_PATH);  

    // 将获取的路径转换为 string 类型
    string programPath = buffer;

    // 获取程序所在的目录
    size_t pos = programPath.find_last_of("\\/");
    string programDir = programPath.substr(0, pos);
    if (SetCurrentDirectory(programDir.c_str())) {
    } else {
        cout << "[Error]切换工作目录失败!" << endl;
		cout<<"[Warning]现在重新尝试切换，若一直未能切换成功，请尝试修改本程序的安装位置，重新安装尝试"<<endl;
		backdir();
    }
}
//主函数入口
int main()
{
	system("chcp 65001");
	init_console();
	initgraph(640,480,INIT_DEFAULT);
	setcaption("LRC阅读器 v2.0");
	cout<<"LRC阅读器 v2.0"<<"\n"<<"\n";
	cout<<"作者B站:武侠Tux,作者Github:WuXiaTux"<<"\n"<<"\n";
	cout<<"感谢您使用本程序,敬请关注!"<<"\n"<<"\n";
	cout<<"本程序完全免费,请勿进行倒卖传播,如果您花钱购买此软件,请您立即退款并举报"<<"\n"<<"\n";
	cout<<"官方下载地址:https://kali-linux.lanzn.com/b00pzrssfa 密码:wuxiatux"<<"\n"<<"\n";
	cout<<"以下为程序日志输出"<<endl;
	mt19937 gen(static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count()));
	uniform_int_distribution<> dis(0,2);
	int random_num = dis(gen);
	PIMAGE bg = newimage();
	if(random_num==0)
	{
		getimage(bg, "bg.jpg");
	}
	else if(random_num==1)
	{
		getimage(bg, "bg2.jpg");
	}
	else
	{
		getimage(bg, "bg3.jpg");
	}
	setcolor(WHITE); 
	putimage(0, 0, bg);
	main_ui();
	getch(); 
	MessageBox(NULL, "请选择要读取的txt文件位置", "提示", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
	enter_program();
	return 0;
}
//UI界面函数
void main_ui()
{
    setbkmode(TRANSPARENT);
    setfont(32, 0,"宋体");
	setcolor(BLUE); 
    outtextxy(200,110,"LRC阅读器 v2.0");
	setcolor(WHITE); 
    setfont(16, 0,"宋体");
    outtextxy(30,160,"作者B站:武侠Tux,作者Github:WuXiaTux");
    outtextxy(30,180,"感谢您使用本程序,敬请关注!");
    outtextxy(30,200,"本程序完全免费,请勿进行倒卖传播,如果您花钱购买此软件,请您立即退款并举报");
    outtextxy(30,220,"官方下载地址:https://kali-linux.lanzn.com/b00pzrssfa 密码:wuxiatux");
    outtextxy(250,300,"按下任意键继续");
}
//选择文件和分割模式
void enter_program()
{
    a = open_file_dialog();
    if (a.empty()) {
        cout << "[Error]用户未选择文件！" << "\n";
		MessageBox(NULL, "您未选择文件,请重新选择", "错误", MB_OK | MB_ICONHAND);
		enter_program();
    }
    ifstream in(a, ios::in);
	if(!in.is_open())
	{
		cout<<"[Error]文件打开失败！"<<"\n";
		MessageBox(NULL, "文件打开失败,请检查是否存在此文件并重新选择", "错误", MB_OK | MB_ICONHAND);
		enter_program();
	}
	TestEOL test;
	size_t count=count_if(istreambuf_iterator<char>(in),istreambuf_iterator<char>(),test);	
	if (test.las!='\n') 
	{                       
		count++;            
	}
	N=count;
	in.close();
	string x = "\\";
	string y = "/";
	size_t pos;
	while ((pos = a.find(x)) != string::npos) {
		a.replace(pos, 1, y);
	} 
	cout<<"[INFO]用户选择文件的绝对路径："<<a<<endl;
	b=a;
	b.erase(0,b.find_last_of("/")+1);
	b.erase(b.find_last_of("."),b.find_last_not_of(".txt")+3);
	cout<<"[INFO]用户选择文件的文件名:"<<b<<".txt"<<endl;
	string tp="您选择的文件绝对路径是:" + a;
	const char* c_filename = tp.c_str();
	if(MessageBox(NULL, c_filename, "检查文件是否正确？", MB_YESNO | MB_ICONASTERISK)==IDNO)
	{
		cout<<"[INFO]用户主动重新选择"<<endl;
		enter_program();
	}
	else{
		cout<<"[INFO]文件选择成功"<<endl;
	}
    /*if(N>=3600)
	{
		MessageBox(NULL, "本文件过大,建议您选择将本文件分割处理，以便减少进入播放器的时长", "提示", MB_OK | MB_ICONINFORMATION);
        MessageBox(NULL, "分割后，文件会分成多个音乐和LRC文件的组合，您可以打开其中任意一个文件进行播放。\n请注意，本程序会自动分割文件，若前面的文件结束播放，请打开下一个音频文件继续观看", "提示", MB_OK | MB_ICONINFORMATION);
		if(MessageBox(NULL, "选择'是'启用文件分割模式(推荐)，选择'否'进入普通模式", "选择模式", MB_YESNO | MB_ICONASTERISK)==IDYES)
        {
            //create_lrc2();
			cout<<"文件分割模式开启"<<"\n";
        }
	}*/
	create_lrc();
}
//创建歌词文件
void create_lrc()
{
	MessageBox(NULL, "现在开始转换TXT文件为LRC文件,输出在程序文件夹的output目录下", "提示", MB_OK | MB_ICONINFORMATION);
	MessageBox(NULL, "请等待，请勿退出软件或删除选择的txt文件", "等待", MB_OK | MB_ICONINFORMATION);
	string temp,numtemp,numtemp2,temp1="[",temp2=".00]",last;
	temp.reserve(99999999);
	int s=0,m=0;
	cout<<"[INFO]开始生成歌词文件"<<endl;
	ifstream inputFile(a,ios::in);
	for(int i=1;inputFile>>temp;i++)
	{
		temp.erase(0, temp.find_first_not_of(" \t"));
		if(temp.empty())
		{
			break;
		}
		s=i;
		if(s<10)
		{
			numtemp="0"+to_string(s);
			numtemp2="00:";
			last=temp1+numtemp2+numtemp+temp2+temp;
		}
		else if(s<60&&s>=10)
		{
			numtemp=to_string(s);
			numtemp2="00:";
			last=temp1+numtemp2+numtemp+temp2+temp;
		}
		else
		{
			m=(s-(s%60))/60;
			s=s%60;
			numtemp=to_string(s);
			numtemp2=to_string(m);
			if(s<10)
			{
				numtemp="0"+numtemp;
			}
			if(m<10)
			{
				numtemp2="0"+numtemp2+":";
			}
			else
			{
				numtemp2=numtemp2+":";
			}
			last=temp1+numtemp2+numtemp+temp2+temp;
		}
		v.push_back(last); 
	}
	backdir();
	string tmp="output/"+b+".lrc";
	ofstream out(tmp,ios::out);
	for(int i=0;i<v.size();i++)
	{
		out<<v[i]<<"\n";
	}
	inputFile.close();
	out.close();
	ifstream op(tmp,ios::in);
	if(!op.is_open())
	{
		delall();
		MessageBox(NULL, "未能成功生成歌词文件\n请检查txt文件是否损坏或被删除,然后重试", "错误", MB_OK | MB_ICONERROR);
		closegraph();
		exit(0);
	}
	op.close();
	ifstream check(tmp,ios::in);
	TestEOL tes;
	size_t count1=count_if(istreambuf_iterator<char>(check),istreambuf_iterator<char>(),tes);	
	if (tes.las!='\n') 
	{                       
		count1++;            
	}
	N=count1;
	check.close();
	MessageBox(NULL, "成功生成歌词文件","提示",MB_OK | MB_ICONINFORMATION);
	cout<<"[INFO]成功生成歌词文件"<<endl;
	create_music();
	MessageBox(NULL,"现在打开output文件夹\n请连接学习机/词典笔等音乐播放器，打开其对应音乐文件夹，将程序生成的音乐和歌词文件剪切至其中\n具体操作可观看软件作者B站视频", "提示", MB_OK | MB_ICONINFORMATION);
	cout<<"[INFO]打开output文件夹"<<"\n";
	system("start output");
	//cout<<"如果您选择了文件分割模式，output目录下会出现较多的音频和歌词文件，请将这些文件全部转至设备中"<<"\n";
	cout<<"[INFO]完成"<<endl;
	Sleep(5000);
	if(MessageBox(NULL,"是否回到主界面？","选择", MB_YESNO | MB_ICONASTERISK)==IDYES)
	{
		enter_program();
	}
	else{
		closegraph();
		exit(0);
	}
	
}
void create_music()
{
	MessageBox(NULL, "现在创建与LRC文件配套的音频文件\n本音频文件为静音文件,输出在程序文件夹的output目录下\n控制台会出现许多英文字符,请不要做任何操作(尤其是按q和?键，否则会导致生成的文件无法正常使用)或退出程序,等待程序提示下一步", "提示", MB_OK | MB_ICONINFORMATION);
	temp2="output/"+b+".mp3";
	cout<<"[INFO]开始创建音频文件"<<endl;
	ifstream read(temp2,ios::in);
	if(read.is_open())
	{
		cout<<"[INFO]重复文件！现在删除原文件，创建新文件"<<endl;
		remove(temp2.c_str());
	}
	read.close();
	string temp1=to_string(N)+" ";
	str2=str1+temp1+"-i silent.mp3 -c copy output/"+b+".mp3"; 
	const char *command=str2.c_str();
	cout<<"[INFO]ffmpeg开始创建音频文件"<<endl;
	system(command);
	ifstream read2(temp2,ios::in);
	if(!read2.is_open())
	{
		MessageBox(NULL,"未能成功生成音频文件\n请查看ffmpeg输出，从中查看问题然后重试","错误",MB_OK | MB_ICONERROR);
		MessageBox(NULL,"如果您不能看懂输出内容，请将输出内容全部复制(选中并按住CTRL+C)，\n私信我的哔哩哔哩账号'武侠Tux'寻找解决办法","错误",MB_OK | MB_ICONERROR);
		delall();
		MessageBox(NULL,"三秒后返回主页面","提示",MB_OK | MB_ICONINFORMATION);
		Sleep(3000);
		getch();
		enter_program();
	}
	read2.close();
	MessageBox(NULL,"成功生成音频文件","提示",MB_OK | MB_ICONINFORMATION);
	cout<<"[INFO]成功生成音频文件"<<endl;
}
//删除出错文件
void delall()
{
	string rm = "del /q output\\*";
	system(rm.c_str());
}
