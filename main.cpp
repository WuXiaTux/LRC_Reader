//预备准备
#include <graphics.h>
#include <ege/sys_edit.h>
#include <math.h>
#include<iostream>
#include<windows.h>
#include<fstream>
#include<vector>
#include<string>
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
//定义命名空间
using namespace std;
using namespace filesystem;
//全局变量定义
string str1="ffmpeg -stream_loop ",a,b,temp2,str2,opt;
vector<string> v;
int i=0,N=0,count_cd=0;
bool notANSI=false;
//函数定义
void main_ui();
void enter_program();
void create_music();
void create_lrc();
void create_lrc2();
void delall();
void backdir();
void convert_to_ansi(const string& input_path);
bool checkAndPrepareResources();

//检查程序依赖文件是否存在
bool checkAndPrepareResources() {
	// 1. 检查或创建 output 文件夹
	if (!exists("output")) {
		try {
			create_directory("output");
			cout << "[ERROR]output文件夹被删除！程序已自动重新创建 output 文件夹！" << endl;
			cout << "[WARNING]请勿随意删除程序内部文件，否则会导致程序无法运行或报错！"<< endl;
		} catch (const filesystem_error& e) {
			MessageBox(NULL,"无法创建 output 文件夹！","错误",MB_ICONERROR | MB_OK);
			return false;
		}
	} else if (!is_directory("output")) {
		MessageBox(NULL,"output 被占用且不是文件夹，请重新安装程序！","严重错误",MB_ICONERROR | MB_OK);
		return false;
	}
	
	// 2. 检查关键文件是否存在
	vector<string> critical_files = {"bg.jpg", "bg2.jpg", "bg3.jpg", "silent.mp3"};
	for (const auto& file : critical_files) {
		if (!exists(file) || !is_regular_file(file)) {
			MessageBox(NULL,"程序依赖文件被删除，请重新安装程序！","严重错误", MB_ICONERROR | MB_OK);
			return false;
		}
	}
	
	return true;
}
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
//将工作目录切换回程序目录，以用相对路径方式创建文件
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
		cout << "[ERROR]切换工作目录失败!" << endl;
		cout<<"[WARNING]现在重新尝试切换，若一直未能切换成功，请尝试修改本程序的安装位置，重新安装尝试"<<endl;
		if(count_cd>=5){
			MessageBox(NULL,"未能成功切换工作目录，请尝试修改本程序的安装位置、给本程序添加杀软白名单/n或使用管理员权限打开程序！若多次尝试无效，请在B站私信处向UP反馈！","错误！",MB_OK | MB_ICONERROR);
		}
		else{
			count_cd=count_cd+1;
			backdir();
		}
	}
}
//检查txt文件类型是否为ANSI
void check_ansi(const string& file_path) {
	notANSI = false; // 重置检测状态
	
	// 阶段1：验证文件扩展名
	size_t dot_pos = file_path.find_last_of('.');
	if (dot_pos == string::npos || 
		file_path.substr(dot_pos) != ".txt") return;
	
	// 阶段2：打开文件并检测BOM
	ifstream fin(file_path, ios::binary);
	if (!fin.is_open()) return;
	
	// 读取前4字节检测BOM
	unsigned char bom[4] = {0};
	fin.read(reinterpret_cast<char*>(bom), 4);
	const bool is_utf8 = (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF);
	const bool is_utf16le = (bom[0] == 0xFF && bom[1] == 0xFE);
	const bool is_utf16be = (bom[0] == 0xFE && bom[1] == 0xFF);
	
	if (is_utf8 || is_utf16le || is_utf16be) {
		notANSI = true;
		fin.close();
		return;
	}
	
	// 阶段3：读取完整文件内容
	fin.seekg(0, ios::end);
	const size_t file_size = fin.tellg();
	fin.seekg(0);
	vector<char> file_data(file_size);
	fin.read(file_data.data(), file_size);
	fin.close();
	
	// 阶段4：严格ANSI验证
	const int wc_size = MultiByteToWideChar(
		CP_ACP, 
		MB_ERR_INVALID_CHARS,
		file_data.data(),
		file_data.size(),
		nullptr,
		0
		);
	
	if (wc_size == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
		notANSI = true;
	}
}
//转换txt文件类型
void convert_to_ansi(const string& input_path) {
	// 扩展名验证（不区分大小写）
	size_t dot_pos = input_path.find_last_of('.');
	if (dot_pos == string::npos) return;
	
	string ext = input_path.substr(dot_pos);
	transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext != ".txt") return;
	
	// 生成输出路径
	const string output_path = [&]() {
		size_t slash_pos = input_path.find_last_of("/\\");
		string dir = (slash_pos != string::npos) ? 
		input_path.substr(0, slash_pos + 1) : "";
		string fname = (slash_pos != string::npos) ? 
		input_path.substr(slash_pos + 1) : input_path;
		return dir + fname.substr(0, fname.size() - 4) + "_ansi.txt";
	}();
	
	// 读取文件内容
	ifstream fin(input_path, ios::binary | ios::ate);
	if (!fin) return;
	
	const streamsize total_size = fin.tellg();
	fin.seekg(0, ios::beg);
	
	// 检测并跳过BOM
	vector<char> buffer(total_size);
	fin.read(buffer.data(), total_size);
	fin.close();
	
	const char* data_start = buffer.data();
	streamsize data_len = total_size;
	if (total_size >= 3 && 
		static_cast<unsigned char>(buffer[0]) == 0xEF &&
		static_cast<unsigned char>(buffer[1]) == 0xBB &&
		static_cast<unsigned char>(buffer[2]) == 0xBF) {
		data_start += 3;
		data_len -= 3;
	}
	
	// 编码转换（UTF-8 → UTF-16 → ANSI）
	vector<wchar_t> utf16_buf(data_len);
	int wlen = MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS,
		data_start, data_len,
		utf16_buf.data(), utf16_buf.size()
		);
	if (wlen <= 0) return;
	
	vector<char> ansi_buf(wlen * 2);  // 预分配安全空间
	int alen = WideCharToMultiByte(
		CP_ACP, WC_NO_BEST_FIT_CHARS,
		utf16_buf.data(), wlen,
		ansi_buf.data(), ansi_buf.size(),
		nullptr, nullptr
		);
	if (alen <= 0) return;
	
	// 写入文件
	ofstream fout(output_path, ios::binary);
	if (fout) {
		fout.write(ansi_buf.data(), alen);
	}
}

//主要函数
//主函数入口
int main()
{
	if(!checkAndPrepareResources())
	{
		closegraph();
		exit(0);
	}
	system("chcp 65001");
	init_console();
	initgraph(640,480,INIT_DEFAULT);
	setcaption("LRC阅读器 v2.2");
	cout<<"LRC阅读器 v2.2"<<"\n"<<"\n";
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
	outtextxy(200,110,"LRC阅读器 v2.2");
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
	check_ansi(a);
	if(notANSI==true)
	{
		MessageBox(NULL,"识别到您选择的txt文件格式不为ANSI格式！为了防止出现乱码，建议您将txt文件转换为ANSI格式", "提示", MB_OK | MB_ICONINFORMATION);
		if(MessageBox(NULL, "选择转换txt文件为ANSI格式(是)或继续生成LRC文件(否)", "选择是否转换", MB_YESNO | MB_ICONASTERISK)==IDYES)
		{	
			cout<<"[INFO]开始转换txt文件为ANSI格式"<<"\n";
			convert_to_ansi(a);
			MessageBox(NULL,"转换完成！请进入选择的目录检查文件是否生成，然后重新在本程序中选择转换后的txt文件","提示",MB_OK | MB_ICONINFORMATION);
			cout<<"[INFO]返回主程序部分"<<endl;
			enter_program();
		}
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
	cout<<"[INFO]生成完成，选择是否继续生成"<<endl;
	Sleep(5000);
	if(MessageBox(NULL,"选择继续生成LRC文件(是)或退出程序(否)","选择", MB_YESNO | MB_ICONASTERISK)==IDYES)
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
