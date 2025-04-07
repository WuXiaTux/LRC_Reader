// 导入头文件
#include <graphics.h>	  // EGE图形库
#include <ege/sys_edit.h> // EGE编辑框组件
#include <math.h>		  // 数学函数
#include <iostream>		  // 输入输出流
#include <windows.h>	  // Windows API
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <random>
#include <chrono>
#include <direct.h>
#include <commdlg.h>
#include <tchar.h>
#include <filesystem>
#include <locale.h>
#include <Shlwapi.h>
// 定义命名空间
using namespace std;
using namespace filesystem;
// 全局变量定义
string str1 = "ffmpeg -stream_loop "; // ffmpeg命令1
string txt_path, txt_name;			  // txt_path:TXT路径,txt_name:TXT文件名
string mp3_out, str2;				  // mp3_out:输出MP3的路径,str2:ffmpeg命令2
string pic_path, pic_name, tmp2;	  // pic_path:图片路径,pic_name:图片文件名称,tmp2:输出LRC和MP3的文件名
string language = "zh", temp_set, encoding = "ANSI";
string width_height = "null", width, height,tp;
vector<string> v_read_txt; // 用于保存TXT文件内容的数组
vector<string> countn;	   // 用于计算TXT文件长度的数组
vector<string> v_pak;	   // 用于保存main.pak文件数据的数组
PIMAGE bg;				   // 背景图片
int line_number = 0, count_cd = 0, cycle_times, line_last, bg_number;
bool notANSI = false;
// 按钮类定义
struct RectButton
{
	int x, y;
	int width, height;
};
// 定义按钮区域
RectButton button = {
	50, 300, /* x, y */
	140, 60, /* width, height */
};
RectButton button2 = {
	450, 300, /* x, y */
	140, 60,  /* width, height */
};
RectButton button3 = {
	280, 400, /* x, y */
	80, 40,	  /* width, height */
};
RectButton chinese = {
	53, 210, /* x, y */
	80, 40,	 /* width, height */
};
RectButton english = {
	450, 210, /* x, y */
	80, 40,	  /* width, height */
};
RectButton ansi = {
	53, 330, /* x, y */
	80, 40,	 /* width, height */
};
RectButton utf_8 = {
	450, 330, /* x, y */
	80, 40,	  /* width, height */
};
RectButton back = {
	280, 400, /* x, y */
	80, 40,	  /* width, height */
};
struct TestEOL
{
	bool operator()(char c)
	{
		las = c;
		return las == '\n';
	}
	char las;
};
// 函数定义
bool checkAndPrepareResources();
string open_file_dialog();
void backdir();
string checkFileEncoding(const string &filePath);
void change_utf_8(const string &input_path);
bool ANSItoUTF8(const string &inputPath);
bool insideRectButton(const RectButton *button, int x, int y);
void drawRectButton(const RectButton *button);
void draw();
void draw2();
void put_image();
void main_menu();
void enter_program();
void create_music();
void create_music2();
void create_lrc();
void create_lrc2();
void create_pic();
void delall();
void setting_opinion();
void set_language();
void set_encoding();
void enter_width_height();

// 次要函数
//  检查程序依赖文件是否存在
bool checkAndPrepareResources()
{
	/**
	 * 检查并准备程序依赖资源
	 * 1. 检查/创建output目录
	 * 2. 验证关键文件存在性
	 * 返回：准备就绪返回true，否则false
	 */
	// 1. 检查或创建 output 文件夹
	if (!exists("output"))
	{
		try
		{
			create_directory("output");
			if (v_pak[0] == "zh")
			{
				MessageBox(NULL, "output文件夹被删除！程序已自动重新创建 output 文件夹！", "错误", MB_ICONERROR | MB_OK);
				MessageBox(NULL, "请勿随意删除程序内部文件，否则会导致程序无法运行或报错！", "错误", MB_ICONERROR | MB_OK);
			}
			else
			{
				MessageBox(NULL, "The output folder has been deleted! Now,the program will automatically create the output folder!", "Error", MB_ICONERROR | MB_OK);
				MessageBox(NULL, "Please do not delete any files in the program, otherwise the program will not run or report errors!", "Error", MB_ICONERROR | MB_OK);
			}
		}
		catch (const filesystem_error &e)
		{
			if (v_pak[0] == "zh")
			{
				MessageBox(NULL, "无法创建 output 文件夹！", "错误", MB_ICONERROR | MB_OK);
			}
			else
			{
				MessageBox(NULL, "Cannot create output folder!", "Error", MB_ICONERROR | MB_OK);
			}
			return false;
		}
	}
	else if (!is_directory("output"))
	{
		if (v_pak[0] == "zh")
		{
			MessageBox(NULL, "output 被占用且不是文件夹，请重新安装程序！", "错误", MB_ICONERROR | MB_OK);
		}
		else
		{
			MessageBox(NULL, "The output folder is occupied and is not a folder. Please reinstall the program!", "Error", MB_ICONERROR | MB_OK);
		}
		return false;
	}

	// 2. 检查关键文件是否存在
	vector<string> critical_files = {"bg1.jpg", "bg2.jpg", "bg3.jpg", "silent.mp3", "silent2.mp3", "ffmpeg.exe", "ascii-image-converter.exe", "main.pak"};
	for (const auto &file : critical_files)
	{
		if (!exists(file) || !is_regular_file(file))
		{
			if (v_pak[0] == "zh")
			{
				MessageBox(NULL, "程序依赖文件被删除，请重新安装程序！", "错误", MB_ICONERROR | MB_OK);
			}
			else
			{
				MessageBox(NULL, "The program dependency file has been deleted, please reinstall the program!", "Error", MB_ICONERROR | MB_OK);
			}
			return false;
		}
	}
	return true;
}
// TXT文件选择对话框函数
string open_file_dialog()
{
	/**
	 * 文件选择对话框（TXT文件）
	 * 返回：选择的文件路径，取消选择返回空字符串
	 */
	OPENFILENAME ofn;
	char szFile[260] = {0}; // 文件名缓冲区
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (v_pak[0] == "zh")
	{
		ofn.lpstrTitle = _T("选择TXT文件");
	}
	else
	{
		ofn.lpstrTitle = _T("Select TXT file");
	}
	ofn.lpstrInitialDir = _T("C:\\"); // 设置默认目录
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	if (GetOpenFileName(&ofn))
	{
		return szFile;
	}
	else
	{
		return "";
	}
}
// 图片文件选择对话框函数
string open_file_dialog_p()
{
	OPENFILENAME ofn;
	char szFile[260] = {0}; // 文件名缓冲区
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = "\0*.jpg\0\0*.png\0\0*.gif\0\0*.bmp\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (v_pak[0] == "zh")
	{
		ofn.lpstrTitle = _T("选择图片文件");
	}
	else
	{
		ofn.lpstrTitle = _T("Select image file");
	}
	ofn.lpstrInitialDir = _T("C:\\"); // 设置默认目录
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	if (GetOpenFileName(&ofn))
	{
		return szFile;
	}
	else
	{
		return "";
	}
}
// 将工作目录切换回程序目录，以用相对路径方式创建文件
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
	if (SetCurrentDirectory(programDir.c_str()))
	{
	}
	else
	{
		cout << "[ERROR]切换工作目录失败!" << "\n";
		cout << "[WARNING]现在重新尝试切换，若一直未能切换成功，请尝试修改本程序的安装位置，重新安装尝试" << "\n";
		if (count_cd >= 5)
		{
			MessageBox(NULL, "未能成功切换工作目录，请尝试修改本程序的安装位置、给本程序添加杀软白名单/n或使用管理员权限打开程序！若多次尝试无效，请在B站私信处向UP反馈！", "错误！", MB_OK | MB_ICONERROR);
		}
		else
		{
			count_cd = count_cd + 1;
			backdir();
		}
	}
}
// 检查文件编码类型的函数
string checkFileEncoding(const string &filePath)
{
	// 检测逻辑：
	// 1. 检查UTF-8 BOM头(EF BB BF)
	// 2. 逐字节验证GB2312编码规则
	// 返回："UTF-8"、"GB2312"或"false"
	ifstream file(filePath, ios::binary);
	if (!file)
	{
		cerr << "无法打开文件: " << filePath << endl;
		return "false";
	}

	// 读取前几个字节以检测UTF-8 BOM
	vector<char> buffer(3);
	file.read(buffer.data(), buffer.size());

	// 检测UTF-8 BOM
	if (buffer.size() >= 3 &&
		(unsigned char)buffer[0] == 0xEF &&
		(unsigned char)buffer[1] == 0xBB &&
		(unsigned char)buffer[2] == 0xBF)
	{
		return "UTF-8";
	}

	// 重新定位到文件开头
	file.clear();
	file.seekg(0, ios::beg);

	// 读取文件内容并检测GB2312编码
	string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
	file.close();

	// 检测GB2312编码
	for (size_t i = 0; i < content.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(content[i]);
		// GB2312字符范围检查
		if (c >= 0x80 && c <= 0xFF)
		{ // 高位字节
			if (i + 1 < content.size())
			{
				unsigned char next = static_cast<unsigned char>(content[i + 1]);
				if (next >= 0x40 && next <= 0xFE)
				{
					// 找到一个有效的GB2312双字节字符
					i++; // 跳过下一个字节
				}
				else
				{
					return "false"; // 不符合GB2312
				}
			}
			else
			{
				return "false"; // 单独的高位字节没有匹配
			}
		}
	}

	return "GB2312"; // 如果没有问题，返回GB2312
}
// 将UTF-8转换为ANSI类型
void change_utf_8(const string &input_path)
{
	/*
	转换步骤：
	1. 读取文件并跳过BOM头
	2. 使用Windows API进行编码转换：
	   MultiByteToWideChar(CP_UTF8) → WideCharToMultiByte(CP_ACP)
	3. 生成_ansi.txt后缀的新文件
	*/
	// 扩展名验证（不区分大小写）
	size_t dot_pos = input_path.find_last_of('.');
	if (dot_pos == string::npos)
		return;

	string ext = input_path.substr(dot_pos);
	transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext != ".txt")
		return;

	// 生成输出路径
	const string output_path = [&]()
	{
		size_t slash_pos = input_path.find_last_of("/\\");
		string dir = (slash_pos != string::npos) ? input_path.substr(0, slash_pos + 1) : "";
		string fname = (slash_pos != string::npos) ? input_path.substr(slash_pos + 1) : input_path;
		return dir + fname.substr(0, fname.size() - 4) + "_ansi.txt";
	}();

	// 读取文件内容
	ifstream fin(input_path, ios::binary | ios::ate);
	if (!fin)
		return;

	const streamsize total_size = fin.tellg();
	fin.seekg(0, ios::beg);

	// 检测并跳过BOM
	vector<char> buffer(total_size);
	fin.read(buffer.data(), total_size);
	fin.close();

	const char *data_start = buffer.data();
	streamsize data_len = total_size;
	if (total_size >= 3 &&
		static_cast<unsigned char>(buffer[0]) == 0xEF &&
		static_cast<unsigned char>(buffer[1]) == 0xBB &&
		static_cast<unsigned char>(buffer[2]) == 0xBF)
	{
		data_start += 3;
		data_len -= 3;
	}

	// 编码转换（UTF-8 → UTF-16 → ANSI）
	vector<wchar_t> utf16_buf(data_len);
	int wlen = MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS,
		data_start, data_len,
		utf16_buf.data(), utf16_buf.size());
	if (wlen <= 0)
		return;

	vector<char> ansi_buf(wlen * 2); // 预分配安全空间
	int alen = WideCharToMultiByte(
		CP_ACP, WC_NO_BEST_FIT_CHARS,
		utf16_buf.data(), wlen,
		ansi_buf.data(), ansi_buf.size(),
		nullptr, nullptr);
	if (alen <= 0)
		return;

	// 写入文件
	ofstream fout(output_path, ios::binary);
	if (fout)
	{
		fout.write(ansi_buf.data(), alen);
	}
}
// 将ANSI编码转换为UTF-8类型
bool ANSItoUTF8(const string &inputPath)
{
	// 检查文件有效性
	if (!filesystem::exists(inputPath) || !filesystem::is_regular_file(inputPath))
	{
		return false;
	}

	// 生成输出路径
	filesystem::path pathObj(inputPath);
	string outputPath = pathObj.parent_path().string() + "\\" + pathObj.stem().string() + "_utf8" + pathObj.extension().string();

	// 读取文件内容
	ifstream fin(inputPath, ios::binary | ios::ate);
	if (!fin)
		return false;

	const streamsize fileSize = fin.tellg();
	fin.seekg(0, ios::beg);
	vector<char> buffer(fileSize);
	fin.read(buffer.data(), fileSize);
	fin.close();

	// ANSI转UTF-16
	int wideLen = MultiByteToWideChar(CP_ACP, 0, buffer.data(), fileSize, nullptr, 0);
	if (wideLen <= 0)
		return false;

	vector<wchar_t> utf16Buffer(wideLen);
	MultiByteToWideChar(CP_ACP, 0, buffer.data(), fileSize,
						utf16Buffer.data(), wideLen);

	// UTF-16转UTF-8
	int utf8Len = WideCharToMultiByte(CP_UTF8, 0, utf16Buffer.data(), wideLen,
									  nullptr, 0, nullptr, nullptr);
	if (utf8Len <= 0)
		return false;

	vector<char> utf8Buffer(utf8Len);
	WideCharToMultiByte(CP_UTF8, 0, utf16Buffer.data(), wideLen,
						utf8Buffer.data(), utf8Len, nullptr, nullptr);

	// 写入文件（不带BOM）
	ofstream fout(outputPath, ios::binary);
	if (!fout)
		return false;

	fout.write(utf8Buffer.data(), utf8Len);
	return fout.good();
}

// 以下是绘制按钮的函数
bool insideRectButton(const RectButton *button, int x, int y)
{
	return (x >= button->x) && (y >= button->y) && (x < button->x + button->width) && (y < button->y + button->height);
}
void drawRectButton(const RectButton *button)
{
	setfillcolor(EGERGB(0x1E, 0x90, 0xFF));
	bar(button->x, button->y, button->x + button->width, button->y + button->height);
}
// 以下是绘制界面的函数
// 绘制主界面
void draw()
{
	/*
	  主界面绘制函数
	  绘制按钮和文字提示
	 */
	drawRectButton(&button);
	drawRectButton(&button2);
	drawRectButton(&button3);
	if (v_pak[0] == "zh")
	{
		outtextxy(88, 322, "开始转换");
		outtextxy(470, 322, "ASCII图像显示");
		outtextxy(303, 411, "设置");
	}
	else
	{
		outtextxy(56, 322, "Convert TXT file");
		outtextxy(475, 312, "ASCII Image");
		outtextxy(488, 337, "Display");
		outtextxy(293, 411, "Setting");
	}
}
// 绘制设置界面
void draw2()
{
	drawRectButton(&chinese);
	drawRectButton(&english);
	drawRectButton(&ansi);
	drawRectButton(&utf_8);
	drawRectButton(&back);
	outtextxy(76, 223, "中文");
	outtextxy(462, 223, "English");
	outtextxy(75, 343, "ANSI");
	outtextxy(470, 343, "UTF-8");
	outtextxy(282, 415, "返回(back)");
}
// 绘制背景图片
void put_image()
{
	bg = newimage();
	if (bg_number == 1)
	{
		getimage(bg, "bg1.jpg");
	}
	else if (bg_number == 2)
	{
		getimage(bg, "bg2.jpg");
	}
	else
	{
		getimage(bg, "bg3.jpg");
	}
	putimage(0, 0, bg);
}

// 主要函数
// 程序入口
int main()
{
	backdir();
	ifstream check_la("main.pak", ios::in);
	while (getline(check_la, temp_set))
	{
		v_pak.push_back(temp_set);
	}
	check_la.close();
	if (v_pak[0] != "zh" && v_pak[0] != "en")
	{
		v_pak[0] = "zh";
		cout << "[Error]main.pak被错误修改，语言重新调整为中文！" << "\n";
		ofstream reset_la("main.pak", ios::out);
		for (int i = 0; i < 2; i++)
		{
			reset_la << v_pak[i] << "\n";
		}
		reset_la.close();
	}
	if (v_pak[1] != "ANSI" && v_pak[1] != "UTF_8")
	{
		v_pak[1] = "ANSI";
		if (v_pak[0] == "zh")
		{
			cout << "[Error]main.pak被错误修改，编码重新调整为ANSI！" << "\n";
		}
		else
		{
			cout << "[Error]main.pak has been incorrectly modified,encoding reset to ANSI." << "\n";
		}
		ofstream reset_en("main.pak", ios::out);
		for (int i = 0; i < 2; i++)
		{
			reset_en << v_pak[i] << "\n";
		}
		reset_en.close();
	}
	if (!checkAndPrepareResources())
	{ // 1. 检查依赖文件
		delimage(bg);
		cleardevice();
		put_image();
		main_menu();
	}
	system("chcp 65001"); // 2. 设置控制台UTF-8编码
	SetConsoleOutputCP(65001);
	init_console();
	initgraph(640, 480, INIT_WITHLOGO); // 3. 初始化640x480图形窗口
	if (v_pak[0] == "zh")
	{
		setcaption("LRC阅读器 v3.0");
	}
	else
	{
		setcaption("LRC Reader v3.0");
	}
	ege_enable_aa(true);
	if (v_pak[0] == "zh")
	{
		cout << "LRC阅读器 v3.0" << "\n"
			 << "\n";
		cout << "作者B站:武侠Tux,作者Github:WuXiaTux" << "\n"
			 << "\n";
		cout << "感谢您使用本程序,敬请关注!" << "\n"
			 << "\n";
		cout << "本程序完全免费开源,请勿进行倒卖传播,如果您花钱购买此软件,请您立即退款并举报" << "\n"
			 << "\n";
		cout << "官方开源地址:https://github.com/WuXiaTux/LRC_Reader" << "\n"
			 << "\n";
		cout << "官方下载地址:https://kali-linux.lanzn.com/b00pzrssfa 密码:wuxiatux" << "\n"
			 << "\n"
			 << "\n";
		cout << "以下为程序日志输出" << "\n";
		cout << "[Info]当前语言为中文，" << "当前LRC文件编码为" << v_pak[1] << "\n";
	}
	else
	{
		cout << "LRC Reader v3.0" << "\n"
			 << "\n";
		cout << "Author Bilibili:WuXiaTux,Author Github:WuXiaTux" << "\n"
			 << "\n";
		cout << "Thank you for using this program,please follow&star!" << "\n"
			 << "\n";
		cout << "This program is completely free and open source,please don't sell it.if you pay for this software,please refund and report the seller immediately." << "\n"
			 << "\n";
		cout << "Official open source address:https://github.com/WuXiaTux/LRC_Reader" << "\n"
			 << "\n";
		cout << "Official download address:https://kali-linux.lanzn.com/b00pzrssfa password:wuxiatux" << "\n"
			 << "\n"
			 << "\n";
		cout << "Program log output" << "\n";
		cout << "[Info]Current language is English, " << "Current LRC file encoding is " << v_pak[1] << "\n";
	}
	mt19937 gen(static_cast<unsigned int>(chrono::high_resolution_clock::now().time_since_epoch().count()));
	uniform_int_distribution<> dis(1, 3);
	int random_num = dis(gen);
	if (random_num == 0)
	{
		bg_number = 1;
	}
	else if (random_num == 2)
	{
		bg_number = 2;
	}
	else
	{
		bg_number = 3;
	}
	put_image();
	main_menu();
	return 0;
}
// UI界面和按钮操作函数
void main_menu()
{
	setcolor(WHITE);
	setbkmode(TRANSPARENT);
	setfont(32, 0, "宋体");
	setcolor(BLUE);
	if (v_pak[0] == "zh")
	{
		outtextxy(200, 110, "LRC阅读器 v3.0");
	}
	else
	{
		outtextxy(200, 110, "LRC Reader v3.0");
	}
	setcolor(WHITE);
	setfont(16, 0, "宋体");
	if (v_pak[0] == "zh")
	{
		outtextxy(30, 150, "作者B站:武侠Tux,作者Github:WuXiaTux");
		outtextxy(30, 170, "感谢您使用本程序,敬请关注!");
		outtextxy(30, 190, "本程序完全免费开源,请勿进行倒卖传播,如果您花钱购买此软件,请您立即退款并举报");
		outtextxy(30, 210, "官方开源地址:https://github.com/WuXiaTux/LRC_Reader");
		outtextxy(30, 230, "官方下载地址:https://kali-linux.lanzn.com/b00pzrssfa 密码:wuxiatux");
	}
	else
	{
		outtextxy(30, 150, "Author Bilibili:WuXiaTux,Author Github:WuXiaTux");
		outtextxy(30, 170, "Thank you for using this program,please follow&star!");
		outtextxy(30, 190, "This program is completely free and open source,please don't sell it.");
		outtextxy(30, 210, "if you pay for this software,please refund and report the seller immediately.");
		outtextxy(30, 225, "Official open source address:https://github.com/WuXiaTux/LRC_Reader");
		outtextxy(30, 245, "Official download address:https://kali-linux.lanzn.com/b00pzrssfa");
		outtextxy(30, 260, "password:wuxiatux");
	}
	bool clickButton = false, clickButton2 = false, clickButton3 = false;
	bool redraw = true;
	for (; is_run(); delay_fps(60))
	{
		while (mousemsg())
		{
			mouse_msg msg = getmouse();
			// 判断鼠标左键点击（左键按下确定位置，抬起为执行时刻）
			if (msg.is_left())
			{
				if (msg.is_down())
				{
					// 检测点击的按钮
					clickButton = insideRectButton(&button, msg.x, msg.y);
					clickButton2 = insideRectButton(&button2, msg.x, msg.y);
					clickButton3 = insideRectButton(&button3, msg.x, msg.y);
				}
				else
				{
					// 左键抬起，判断是否需要执行事件响应
					if (clickButton)
					{
						clickButton = false;
						redraw = true;
						if (v_pak[0] == "zh")
						{
							MessageBox(NULL, "请选择要转换的TXT文件位置", "提示", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
						}
						else
						{
							MessageBox(NULL, "Please select the location of the TXT file to be converted", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
						}
						enter_program();
					}
					if (clickButton2)
					{
						clickButton2 = false;
						redraw = true;
						if (v_pak[0] == "zh")
						{
							MessageBox(NULL, "本功能不支持滚动播放LRC文件的播放器\n请在左侧命令行界面输入播放器显示LRC文件的长和宽，按下Enter键完成", "提示", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
						}
						else
						{
							MessageBox(NULL, "This function does not support the player that play the LRC file in scrolling mode.\nPlease enter the width and height of the player to display the LRC file, press Enter to continue", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
						}
						if (v_pak[0] == "zh")
						{
							MessageBox(NULL, "请输入播放器显示的长和宽，输入示例：宽x高(中间的x是小写的英文字母x)", "提示", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
						}
						else
						{
							MessageBox(NULL, "Please enter the length and width displayed by the player.Ex: widthxheight(the middle \"x\" is a lowercase English letter \"X\".)", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
						}
						enter_width_height();
						create_pic();
					}
					if (clickButton3)
					{
						clickButton3 = false;
						redraw = true;
						delimage(bg);
						cleardevice();
						put_image();
						setting_opinion();
					}
				}
			}
		}
		// 绘制
		if (redraw)
		{
			draw();
			redraw = false;
		}
	}
}
// 选择文件和分割模式
void enter_program()
{
	txt_path = open_file_dialog();
	if (txt_path.empty())
	{
		if (v_pak[0] == "zh")
		{
			cout << "[Error]用户未选择文件！" << "\n";
			if (MessageBox(NULL, "您未选择文件,请重新选择\n选择继续生成LRC文件(是)或返回主界面(否)", "错误", MB_YESNO | MB_ICONHAND) == IDYES)
			{
				enter_program();
			}
			else
			{
				delimage(bg);
				cleardevice();
				put_image();
				main_menu();
			}
		}
		else
		{
			cout << "[Error]User did not select any file!" << "\n";
			if (MessageBox(NULL, "You did not select any file, please select again\nContinue to generate LRC file (Yes) or return to the main interface (No)", "Error", MB_YESNO | MB_ICONHAND) == IDYES)
			{
				enter_program();
			}
			else
			{
				delimage(bg);
				cleardevice();
				put_image();
				main_menu();
			}
		}
	}
	ifstream in(txt_path, ios::in);
	if (!in.is_open())
	{
		if (v_pak[0] == "zh")
		{
			cout << "[Error]文件打开失败！" << "\n";
			MessageBox(NULL, "文件打开失败,请检查是否存在此文件并重新选择", "错误", MB_OK | MB_ICONHAND);
		}
		else
		{
			cout << "[Error]File opening failed!" << "\n";
			MessageBox(NULL, "File opening failed, please check if the file exists and select again", "Error", MB_OK | MB_ICONHAND);
		}
		enter_program();
	}
	ifstream inputFile(txt_path, ios::in);
	string temp;
	for (int i = 1; inputFile >> temp; i++)
	{
		temp.erase(0, temp.find_first_not_of(" \t"));
		if (temp.empty())
		{
			continue;
		}
		countn.push_back(temp);
	}
	inputFile.close();
	for (int i = 0; i < countn.size(); i++)
	{
		line_number++;
	}
	string x = "\\";
	string y = "/";
	size_t pos;
	while ((pos = txt_path.find(x)) != string::npos)
	{
		txt_path.replace(pos, 1, y);
	}
	txt_name = txt_path;
	txt_name.erase(0, txt_name.find_last_of("/") + 1);
	txt_name.erase(txt_name.find_last_of("."), txt_name.find_last_not_of(".txt") + 3);
	if (v_pak[0] == "zh")
	{
		cout << "[Info]用户选择文件的绝对路径：" << txt_path << "\n";
		cout << "[Info]用户选择文件的文件名:" << txt_name << "\n";
		tp = "您选择的文件绝对路径是:" + txt_path;
	}
	else
	{
		cout << "[Info]The user selects the absolute path of the file:" << txt_path << "\n";
		cout << "[Info]The user selects the file name:" << txt_name << "\n";
		tp = "The file you selected is:" + txt_path;
	}
	const char *c_filename = tp.c_str();
	if (v_pak[0] == "zh")
	{
		if (MessageBox(NULL, c_filename, "检查文件是否正确？", MB_YESNO | MB_ICONASTERISK) == IDNO)
		{
			cout << "[Info]用户主动重新选择" << "\n";
			enter_program();
		}
		else
		{
			cout << "[Info]文件选择成功" << "\n";
		}
	}
	else
	{
		if (MessageBox(NULL, tp.c_str(), "Check the file is correct?", MB_YESNO | MB_ICONASTERISK) == IDNO)
		{
			cout << "[Info]User actively re-selects" << "\n";
			enter_program();
		}
		else
		{
			cout << "[Info]File selection is successful" << "\n";
		}
	}
	if (checkFileEncoding(txt_path) == "UTF-8")
	{
		if (v_pak[1] == "ANSI")
		{
			if (v_pak[0] == "zh")
			{
				cout << "[Info]检查txt文件格式" << "\n";
				MessageBox(NULL, "识别到您选择的txt文件格式不为ANSI格式！为了防止出现乱码，建议您将txt文件转换为ANSI格式，转换后的文件将在文件名后加入_ansi", "提示", MB_OK | MB_ICONINFORMATION);
				if (MessageBox(NULL, "选择转换txt文件为ANSI格式(是)或继续生成LRC文件(否)", "选择是否转换", MB_YESNO | MB_ICONASTERISK) == IDYES)
				{
					cout << "[Info]txt文件格式为GB2312" << "\n";
					cout << "[Info]开始转换txt文件为ANSI格式" << "\n";
					change_utf_8(txt_path);
					MessageBox(NULL, "转换完成！请进入选择的目录检查文件是否生成，然后重新在本程序中选择转换后的txt文件", "提示", MB_OK | MB_ICONINFORMATION);
					cout << "[Info]重新选择txt文件" << "\n";
					enter_program();
				}
			}
			else
			{
				cout << "[Info]Check txt file format" << "\n";
				MessageBox(NULL, "Check the txt file format is not ANSI format! To prevent garbled code, it is recommended that you convert the txt file to ANSI format,new file will add \"_ansi\" in the filename.", "Information", MB_OK | MB_ICONINFORMATION);
				if (MessageBox(NULL, "Select to convert txt file to ANSI format (Yes) or continue to generate LRC file (No)", "Select whether to convert", MB_YESNO | MB_ICONASTERISK) == IDYES)
				{
					cout << "[Info]TXT file format is GB2312" << "\n";
					cout << "[Info]Start converting TXT to ANSI format." << "\n";
					change_utf_8(txt_path);
					MessageBox(NULL, "Conversion completed! Please check if the file is generated in the selected directory, and then select the converted txt file in this program", "Information", MB_OK | MB_ICONINFORMATION);
					cout << "[Info]Re-select txt file" << "\n";
					enter_program();
				}
			}
		}
	}
	else if (checkFileEncoding(txt_path) == "GB2312")
	{
		if (v_pak[1] == "UTF_8")
		{
			if (v_pak[0] == "zh")
			{
				cout << "[Info]检查txt文件格式" << "\n";
				MessageBox(NULL, "识别到您选择的txt文件格式不为UTF-8格式！为了防止出现乱码，建议您将txt文件转换为UTF-8格式，输出的文件将在文件名后加入_utf8", "提示", MB_OK | MB_ICONINFORMATION);
				if (MessageBox(NULL, "选择转换txt文件为UTF-8格式(是)或继续生成LRC文件(否)", "选择是否转换", MB_YESNO | MB_ICONASTERISK) == IDYES)
				{
					cout << "[Info]txt文件格式为GB2312" << "\n";
					cout << "[Info]开始转换txt文件为UTF-8格式" << "\n";
					ANSItoUTF8(txt_path);
					MessageBox(NULL, "转换完成！请进入选择的目录检查文件是否生成，然后重新在本程序中选择转换后的txt文件", "提示", MB_OK | MB_ICONINFORMATION);
					cout << "[Info]重新选择txt文件" << "\n";
					enter_program();
				}
			}
			else
			{
				cout << "[Info]Check txt file format" << "\n";
				MessageBox(NULL, "Check the txt file format is not UTF-8 format! To prevent garbled code, it is recommended that you convert the txt file to UTF-8 format,new file will add \"_utf8\" in the filename.", "Information", MB_OK | MB_ICONINFORMATION);
				if (MessageBox(NULL, "Select to convert txt file to UTF-8 format (Yes) or continue to generate LRC file (No)", "Select whether to convert", MB_YESNO | MB_ICONASTERISK) == IDYES)
				{
					cout << "[Info]TXT file format is GB2312" << "\n";
					cout << "[Info]Start converting TXT to UTF-8 format." << "\n";
					ANSItoUTF8(txt_path);
					MessageBox(NULL, "Conversion completed! Please check if the file is generated in the selected directory, and then select the converted txt file in this program", "Information", MB_OK | MB_ICONINFORMATION);
					cout << "[Info]Re-select txt file" << "\n";
					enter_program();
				}
			}
		}
	}
	else
	{
		if (v_pak[0] == "zh")
		{
			MessageBox(NULL, "识别到您选择的txt文件格式不为UTF-8或GB2312格式！为了防止出现乱码,建议您使用记事本等工具将TXT文件的编码格式转换为上述两种通用的格式！", "提示", MB_OK | MB_ICONINFORMATION);
			if (MessageBox(NULL, "是否重新选择文件？选择'是'重新选择文件，选择'否'继续进行LRC文件生成", "是否重选文件？", MB_YESNO | MB_ICONASTERISK) == IDYES)
			{
				cout << "[Info]重新选择文件" << "\n";
				enter_program();
			}
		}
		else
		{
			MessageBox(NULL, "Check the txt file format is not UTF-8 or GB2312 format! To prevent garbled code, it is recommended that you use a text editor such as Notepad to convert the txt file encoding format to UTF-8 or GB2312.", "Information", MB_OK | MB_ICONINFORMATION);
			if (MessageBox(NULL, "Do you want to re-select the file? Select 'Yes' to re-select the file, select 'No' to continue generating LRC file", "Do you want to re-select?", MB_YESNO | MB_ICONASTERISK) == IDYES)
			{
				cout << "[Info]Re-select txt file" << "\n";
				enter_program();
			}
		}
	}
	if (line_number > 500)
	{
		if (v_pak[0] == "zh")
		{
			MessageBox(NULL, "本文件较大,建议您选择将本文件分割处理，以便减少进入播放器的时长", "提示", MB_OK | MB_ICONINFORMATION);
			MessageBox(NULL, "分割后，文件会分成多个音乐和LRC文件的组合，如果前面的文件结束播放，请打开下一个音频文件继续观看", "提示", MB_OK | MB_ICONINFORMATION);
			if (MessageBox(NULL, "选择'是'启用文件分割模式(推荐)，选择'否'进入普通模式", "选择模式", MB_YESNO | MB_ICONASTERISK) == IDYES)
			{
				cout << "[Info]文件分割模式开启" << "\n";
				create_lrc2();
			}
		}
		else
		{
			MessageBox(NULL, "This file is large, it is recommended that you select to divide the file to reduce the time entered into the player", "Information", MB_OK | MB_ICONINFORMATION);
			MessageBox(NULL, "After division, the file will be divided into a combination of music and LRC files, if the previous file ends the playback, please open the next audio file to continue watching", "Information", MB_OK | MB_ICONINFORMATION);
			if (MessageBox(NULL, "Select 'Yes' to enable file division mode (recommended), select 'No' to enter normal mode", "Select mode", MB_YESNO | MB_ICONASTERISK) == IDYES)
			{
				cout << "[Info]File division mode is enabled" << "\n";
				create_lrc2();
			}
		}
	}
	if (v_pak[0] == "zh")
	{
		cout << "[Info]普通转换模式" << "\n";
		create_lrc();
	}
	else
	{
		cout << "[Info]Normal conversion mode" << "\n";
		create_lrc();
	}
}
// 创建歌词文件（不使用分割模式）
void create_lrc()
{
	// 生成流程：
	// 1. 按行读取文本文件
	// 2. 生成时间戳（格式：[mm:ss.00]）
	// 3. 组合成LRC格式行
	// 4. 输出到output目录
	// 示例：[00:01.00]第一行文本
	if (v_pak[0] == "zh")
	{
		MessageBox(NULL, "现在开始转换TXT文件为LRC文件和mp3文件,输出在程序文件夹的output目录下", "提示", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]开始生成歌词文件" << "\n";
	}
	else
	{
		MessageBox(NULL, "Now start to convert TXT file to LRC file and MP3 file, file will output in the output directory of the program.", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Start to convert TXT file to LRC file and MP3 file." << "\n";
	}
	string temp, numtemp, numtemp2, temp1 = "[", mp3_out = ".00]", last;
	temp.reserve(99999999);
	int s = 0, m = 0;
	ifstream inputFile2(txt_path, ios::in);
	for (int i = 1; inputFile2 >> temp; i++)
	{
		temp.erase(0, temp.find_first_not_of(" \t"));
		if (temp.empty())
		{
			continue;
		}
		s = i;
		if (s < 10)
		{
			numtemp = "0" + to_string(s);
			numtemp2 = "00:";
			last = temp1 + numtemp2 + numtemp + mp3_out + temp;
		}
		else if (s < 60 && s >= 10)
		{
			numtemp = to_string(s);
			numtemp2 = "00:";
			last = temp1 + numtemp2 + numtemp + mp3_out + temp;
		}
		else
		{
			m = (s - (s % 60)) / 60;
			s = s % 60;
			numtemp = to_string(s);
			numtemp2 = to_string(m);
			if (s < 10)
			{
				numtemp = "0" + numtemp;
			}
			if (m < 10)
			{
				numtemp2 = "0" + numtemp2 + ":";
			}
			else
			{
				numtemp2 = numtemp2 + ":";
			}
			last = temp1 + numtemp2 + numtemp + mp3_out + temp;
		}
		v_read_txt.push_back(last);
	}
	inputFile2.close();
	backdir();
	string tmp = "output/" + txt_name + ".lrc";
	ofstream out(tmp, ios::out);
	for (int i = 0; i < v_read_txt.size(); i++)
	{
		out << v_read_txt[i] << "\n";
	}
	out.close();
	ifstream op(tmp, ios::in);
	if (!op.is_open())
	{
		delall();
		if (v_pak[0] == "zh")
		{
			MessageBox(NULL, "未能成功生成歌词文件\n请检查txt文件是否损坏或被删除,然后重试", "错误", MB_OK | MB_ICONERROR);
		}
		else
		{
			MessageBox(NULL, "Failed to create LRC file.\nPlease check if the txt file is damaged or deleted, then retry.", "Error", MB_OK | MB_ICONERROR);
		}
		delimage(bg);
		cleardevice();
		put_image();
		main_menu();
	}
	op.close();
	if (v_pak[0] == "zh")
	{
		cout << "[Info]成功生成歌词文件" << "\n";
	}
	else
	{
		cout << "[Info]Success to create LRC file." << "\n";
	}
	create_music();
	if (v_pak[0] == "zh")
	{
		MessageBox(NULL, "成功生成LRC文件和音频文件", "提示", MB_OK | MB_ICONINFORMATION);
		MessageBox(NULL, "现在打开output文件夹\n请连接学习机/词典笔等音乐播放器，打开其对应音乐文件夹，将程序生成的音乐和歌词文件剪切至其中\n具体操作可观看软件作者B站视频", "提示", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]打开output文件夹" << "\n";
		system("start output");
		Sleep(5000);
		cout << "[Info]生成完成，选择是否继续生成" << "\n";
		if (MessageBox(NULL, "选择继续生成LRC文件(是)或返回主菜单(否)", "选择", MB_YESNO | MB_ICONASTERISK) == IDYES)
		{
			create_pic();
		}
		else
		{
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
	}
	else
	{
		MessageBox(NULL, "Successfully generated LRC file and audio file", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Successfully generated LRC file" << "\n";
		MessageBox(NULL, "Now open the output folder\nPlease connect the learning machine/dictionary pen etc. music player, open its corresponding music folder, and cut the program generated music and LRC file to it\nFor specific operation, please watch the software author's Youtube video", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Open the output folder" << "\n";
		system("start output");
		Sleep(5000);
		cout << "[Info]Generation complete, ask user continue or back to main menu" << "\n";
		if (MessageBox(NULL, "Do you want to continue generating LRC files?\nChoose \"Yes\" continue create file,choose \"No\" back to the main menu.", "Choose", MB_YESNO | MB_ICONASTERISK) == IDYES)
		{
			create_pic();
		}
		else
		{
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
	}
}
// 创建歌词文件（使用分割模式）
void create_lrc2()
{
	/*
	分割规则：
	1. 每500行分割为一个文件
	2. 计算总分割次数：cycle_times = (总行数 - 余数)/500
	3. 处理剩余行数（line_last = 总行数%500）
	4. 生成形如 filename1.lrc, filename2.lrc... 的文件
	*/
	if (v_pak[0] == "zh")
	{
		MessageBox(NULL, "现在开始转换TXT文件为LRC文件,输出在程序文件夹的output目录下", "提示", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]开始生成歌词文件" << "\n";
	}
	else
	{
		MessageBox(NULL, "Now start to convert TXT file to LRC file, file will output in the output directory of the program.", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Start to convert TXT file to LRC file." << "\n";
	}
	cycle_times = ((line_number - line_number % 500) / 500), line_last = line_number % 500;
	string temp, numtemp, numtemp2, temp1 = "[", mp3_out = ".00]", last;
	temp.reserve(99999999);
	int s = 0, m = 0;
	ifstream inputFile3(txt_path, ios::in);
	for (int cycle = 1; cycle <= cycle_times; cycle++)
	{
		for (int i = 1; i <= 500; i++)
		{
			inputFile3 >> temp;
			temp.erase(0, temp.find_first_not_of(" \t"));
			if (temp.empty())
			{
				continue;
			}
			s = i;
			if (s < 10)
			{
				numtemp = "0" + to_string(s);
				numtemp2 = "00:";
				last = temp1 + numtemp2 + numtemp + mp3_out + temp;
			}
			else if (s < 60 && s >= 10)
			{
				numtemp = to_string(s);
				numtemp2 = "00:";
				last = temp1 + numtemp2 + numtemp + mp3_out + temp;
			}
			else
			{
				m = (s - (s % 60)) / 60;
				s = s % 60;
				numtemp = to_string(s);
				numtemp2 = to_string(m);
				if (s < 10)
				{
					numtemp = "0" + numtemp;
				}
				if (m < 10)
				{
					numtemp2 = "0" + numtemp2 + ":";
				}
				else
				{
					numtemp2 = numtemp2 + ":";
				}
				last = temp1 + numtemp2 + numtemp + mp3_out + temp;
			}
			v_read_txt.push_back(last);
		}
		backdir();
		string tmp = "output/" + txt_name + to_string(cycle) + ".lrc";
		ofstream out(tmp, ios::out);
		for (int i = 0; i < v_read_txt.size(); i++)
		{
			out << v_read_txt[i] << "\n";
		}
		out.close();
		ifstream op(tmp, ios::in);
		if (!op.is_open())
		{
			delall();
			if (v_pak[0] == "zh")
			{
				MessageBox(NULL, "未能成功生成歌词文件\n请检查txt文件是否损坏或被删除,然后重试", "错误", MB_OK | MB_ICONERROR);
				cout << "三秒后返回主界面" << "\n";
			}
			else
			{
				MessageBox(NULL, "Failed to create LRC file.\nPlease check if the txt file is damaged or deleted, then retry.", "Error", MB_OK | MB_ICONERROR);
				cout << "Three seconds later, return to the main menu" << "\n";
			}
			Sleep(3000);
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
		op.close();
		v_read_txt.clear();
	}
	for (int i = 1; i <= line_last; i++)
	{
		inputFile3 >> temp;
		temp.erase(0, temp.find_first_not_of(" \t"));
		if (temp.empty())
		{
			continue;
		}
		s = i;
		if (s < 10)
		{
			numtemp = "0" + to_string(s);
			numtemp2 = "00:";
			last = temp1 + numtemp2 + numtemp + mp3_out + temp;
		}
		else if (s < 60 && s >= 10)
		{
			numtemp = to_string(s);
			numtemp2 = "00:";
			last = temp1 + numtemp2 + numtemp + mp3_out + temp;
		}
		else
		{
			m = (s - (s % 60)) / 60;
			s = s % 60;
			numtemp = to_string(s);
			numtemp2 = to_string(m);
			if (s < 10)
			{
				numtemp = "0" + numtemp;
			}
			if (m < 10)
			{
				numtemp2 = "0" + numtemp2 + ":";
			}
			else
			{
				numtemp2 = numtemp2 + ":";
			}
			last = temp1 + numtemp2 + numtemp + mp3_out + temp;
		}
		v_read_txt.push_back(last);
	}
	backdir();
	int cycle_temp = cycle_times + 1;
	tmp2 = "output/" + txt_name + to_string(cycle_temp);
	string tmp22 = tmp2 + ".lrc";
	ofstream out(tmp22, ios::out);
	for (int i = 0; i < v_read_txt.size(); i++)
	{
		out << v_read_txt[i] << "\n";
	}
	out.close();
	ifstream op(tmp22, ios::in);
	if (!op.is_open())
	{
		delall();
		if (v_pak[0] == "zh")
		{
			MessageBox(NULL, "未能成功生成歌词文件\n请检查txt文件是否损坏或被删除,然后重试", "错误", MB_OK | MB_ICONERROR);
		}
		else
		{
			MessageBox(NULL, "Failed to create LRC file.\nPlease check if the txt file is damaged or deleted, then retry.", "Error", MB_OK | MB_ICONERROR);
		}
		delimage(bg);
		cleardevice();
		put_image();
		main_menu();
	}
	op.close();
	v_read_txt.clear();
	inputFile3.close();
	if (v_pak[0] == "zh")
	{
		cout << "[Info]成功生成歌词文件" << "\n";
	}
	else
	{
		cout << "[Info]Successfully generated LRC file" << "\n";
	}
	create_music2();
	if (v_pak[0] == "zh")
	{
		MessageBox(NULL, "成功生成LRC文件和音频文件", "提示", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]成功生成音频文件" << "\n";
		MessageBox(NULL, "现在打开output文件夹\n请连接学习机/词典笔等音乐播放器，打开其对应音乐文件夹，将程序生成的音乐和歌词文件剪切至其中\n具体操作可观看软件作者B站视频", "提示", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]打开output文件夹" << "\n";
		system("start output");
		Sleep(5000);
		cout << "[Info]生成完成，选择是否继续生成" << "\n";
		if (MessageBox(NULL, "选择继续生成LRC文件(是)或返回主菜单(否)", "选择", MB_YESNO | MB_ICONASTERISK) == IDYES)
		{
			create_pic();
		}
		else
		{
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
	}
	else
	{
		MessageBox(NULL, "Successfully generated LRC file and audio file", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Successfully generated LRC file" << "\n";
		MessageBox(NULL, "Now open the output folder\nPlease connect the learning machine/dictionary pen etc. music player, open its corresponding music folder, and cut the program generated music and LRC file to it\nFor specific operation, please watch the software author's Youtube video", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Open the output folder" << "\n";
		system("start output");
		Sleep(5000);
		cout << "[Info]Generation complete, ask user continue or back to main menu" << "\n";
		if (MessageBox(NULL, "Do you want to continue generating LRC files?\nChoose \"Yes\" continue create file,choose \"No\" back to the main menu.", "Choose", MB_YESNO | MB_ICONASTERISK) == IDYES)
		{
			create_pic();
		}
		else
		{
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
	}
}
// 创建歌词文件（不使用分割模式模式）
void create_music()
{
	// 使用FFmpeg生成静音音频：
	// 命令模板：ffmpeg -stream_loop line_number -i silent.mp3 ...
	// N表示循环次数（对应文本行数）
	// 输出MP3时长与歌词行数匹配
	if (v_pak[0] == "zh")
	{
		cout << "[Info]现在创建与LRC文件配套的音频文件" << "\n"
			 << "[Info]本音频文件为静音文件,输出在程序文件夹的output目录下" << "\n"
			 << "[Info]控制台会出现许多英文字符,请不要对程序做任何操作(尤其是按q和?键，否则会导致生成的文件无法正常使用)或退出程序,等待程序提示下一步" << "\n";
	}
	else
	{
		cout << "[Info]Now create audio file" << "\n"
			 << "[Info]This audio file is a silent file,these files will output in the output directory of the program folder" << "\n"
			 << "[Info]Do not do any operation on the program, wait for the program to prompt the next step" << "\n";
		cout << "[Info]Start to create audio file" << "\n";
	}
	mp3_out = "output/" + txt_name + ".mp3";
	ifstream read(mp3_out, ios::in);
	if (read.is_open())
	{
		if (v_pak[0] == "zh")
		{
			cout << "[Info]重复文件！现在删除原文件，创建新文件" << "\n";
		}
		else
		{
			cout << "[Info]Duplicate file! Now delete the original file and create a new file" << "\n";
		}
		remove(mp3_out.c_str());
	}
	read.close();
	string temp1 = to_string(line_number) + " ";
	str2 = str1 + temp1 + "-i silent.mp3 -c copy \"output/" + txt_name + ".mp3\" -y";
	const char *command = str2.c_str();
	if (v_pak[0] == "zh")
	{
		cout << "[Info]ffmpeg开始创建音频文件" << "\n";
	}
	else
	{
		cout << "[Info]ffmpeg start create audio files" << "\n";
	}
	int ret = system(command);
	if (ret != 0)
	{
		MessageBox(NULL, "ffmpeg错误\n请查看ffmpeg输出，从中查看问题并重试，如果不知道如何解决请私信作者B站账号", "错误", MB_OK | MB_ICONERROR);
		delall();
		MessageBox(NULL, "三秒后返回主菜单", "提示", MB_OK | MB_ICONINFORMATION);
		Sleep(3000);
		delimage(bg);
		cleardevice();
		put_image();
		main_menu();
	}
	ifstream read2(mp3_out, ios::in);
	if (!read2.is_open())
	{
		MessageBox(NULL, "未能成功生成音频文件\n请查看ffmpeg输出", "错误", MB_OK | MB_ICONERROR);
		MessageBox(NULL, "如果您不能看懂输出内容，请将输出内容全部复制(选中并按住Ctrl+Shift+C)或将LOG部分截图，\n私信我的哔哩哔哩账号'武侠Tux'寻找解决办法", "错误", MB_OK | MB_ICONERROR);
		delall();
		MessageBox(NULL, "三秒后返回主菜单", "提示", MB_OK | MB_ICONINFORMATION);
		Sleep(3000);
		delimage(bg);
		cleardevice();
		put_image();
		main_menu();
	}
	read2.close();
	MessageBox(NULL, "成功生成LRC文件和音频文件", "提示", MB_OK | MB_ICONINFORMATION);
	cout << "[Info]成功生成音频文件" << "\n";
}
// 创建歌词文件（使用分割模式）
void create_music2()
{
	if (v_pak[0] == "zh")
	{
		cout << "[Info]现在创建与LRC文件配套的音频文件" << "\n"
			 << "[Info]本音频文件为静音文件,输出在程序文件夹的output目录下" << "\n"
			 << "[Info]控制台会出现许多英文字符,请不要对程序做任何操作(尤其是按q和?键，否则会导致生成的文件无法正常使用)或退出程序,等待程序提示下一步" << "\n";
	}
	else
	{
		cout << "[Info]Now create audio file" << "\n"
			 << "[Info]This audio file is a silent file,these files will output in the output directory of the program folder" << "\n"
			 << "[Info]Do not do any operation on the program, wait for the program to prompt the next step" << "\n";
		cout << "[Info]Start to create audio file" << "\n";
	}
	string silent2 = "silent2.mp3";
	for (int i = 1; i <= cycle_times; i++)
	{
		backdir();
		string temp_mp3 = "output/" + txt_name + to_string(i) + ".mp3";
		ifstream read(temp_mp3, ios::in);
		if (read.is_open())
		{
			if (v_pak[0] == "zh")
			{
				cout << "[Info]重复文件！现在删除原文件，创建新文件" << "\n";
			}
			else
			{
				cout << "[Info]Duplicate file! Now delete the original file and create a new file" << "\n";
			}
			remove(temp_mp3.c_str());
		}
		read.close();
		CopyFile(silent2.c_str(), temp_mp3.c_str(), FALSE);
		backdir();
		ifstream read2(temp_mp3, ios::in);
		if (!read2.is_open())
		{
			if (v_pak[0] == "zh")
			{
				MessageBox(NULL, "未能成功生成音频文件\n请查看ffmpeg输出", "错误", MB_OK | MB_ICONERROR);
				MessageBox(NULL, "如果您不能看懂输出内容，请将输出内容全部复制(选中并按住Ctrl+Shift+C)或将LOG部分截图，\n私信我的哔哩哔哩账号'武侠Tux'寻找解决办法", "错误", MB_OK | MB_ICONERROR);
				delall();
				MessageBox(NULL, "三秒后返回主菜单", "提示", MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(NULL, "Failed to generate audio file\nPlease check the ffmpeg output", "Error", MB_OK | MB_ICONERROR);
				MessageBox(NULL, "If you can't understand the output content, please copy (select and hold Ctrl+Shift+C) or take a screenshot of the LOG part, \nand contact my YouTube account '武侠Tux' for help", "Error", MB_OK | MB_ICONERROR);
				delall();
				MessageBox(NULL, "Three seconds later, return to the home page", "Information", MB_OK | MB_ICONINFORMATION);
			}
			Sleep(3000);
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
		read2.close();
	}
	if (line_last > 0)
	{
		str2 = str1 + to_string(line_last) + " -i silent.mp3 -c copy \"" + tmp2 + ".mp3\" -y";
		const char *command1 = str2.c_str();
		if (v_pak[0] == "zh")
		{
			cout << "[Info]ffmpeg开始创建音频文件" << "\n";
		}
		else
		{
			cout << "[Info]ffmpeg start create audio files" << "\n";
		}
		int ret1 = system(command1);
		if (ret1 != 0)
		{
			if (v_pak[0] == "zh")
			{
				MessageBox(NULL, "ffmpeg错误\n请查看ffmpeg输出，从中查看问题并重试，如果不知道如何解决请私信作者B站账号\"武侠Tux\"", "错误", MB_OK | MB_ICONERROR);
				delall();
				MessageBox(NULL, "三秒后返回主菜单", "提示", MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(NULL, "ffmpeg error\nPlease check the ffmpeg output, from which you can find the problem and retry, if you do not know how to solve, please contact my YouTube account '武侠Tux'", "Error", MB_OK | MB_ICONERROR);
				delall();
				MessageBox(NULL, "Three seconds later, return to the home page", "Information", MB_OK | MB_ICONINFORMATION);
			}
			Sleep(3000);
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
		string tmp3 = tmp2 + ".mp3";
		ifstream read2(tmp3, ios::in);
		if (!read2.is_open())
		{
			if (v_pak[0] == "zh")
			{
				MessageBox(NULL, "未能成功生成音频文件\n请查看ffmpeg输出", "错误", MB_OK | MB_ICONERROR);
				MessageBox(NULL, "如果您不能看懂输出内容，请将输出内容全部复制(选中并按住Ctrl+Shift+C)或将LOG部分截图，\n私信我的哔哩哔哩账号'武侠Tux'寻找解决办法", "错误", MB_OK | MB_ICONERROR);
				delall();
				MessageBox(NULL, "三秒后返回主菜单", "提示", MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(NULL, "Failed to generate audio file\nPlease check the ffmpeg output", "Error", MB_OK | MB_ICONERROR);
				MessageBox(NULL, "If you can't understand the output content, please copy (select and hold Ctrl+Shift+C) or take a screenshot of the LOG part, \nand contact my YouTube account '武侠Tux' for help", "Error", MB_OK | MB_ICONERROR);
				delall();
				MessageBox(NULL, "Three seconds later, return to the home page", "Information", MB_OK | MB_ICONINFORMATION);
			}
			Sleep(3000);
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
		read2.close();
	}
	if (v_pak[0] == "zh")
	{
		MessageBox(NULL, "成功生成LRC文件和音频文件", "提示", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]成功生成音频文件" << "\n";
	}
	else
	{
		MessageBox(NULL, "Successfully generated LRC file and audio file", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Successfully generated LRC file and audio file" << "\n";
	}
}
// ASCII图像显示（包含歌词文件和音频文件）
void create_pic()
{
	pic_path = open_file_dialog_p();
	if (pic_path.empty())
	{
		if (v_pak[0] == "zh")
		{
			cout << "[Error]用户未选择文件！" << "\n";
			if (MessageBox(NULL, "您未选择文件,请重新选择\n选择继续生成LRC文件(是)或返回主界面(否)", "错误", MB_YESNO | MB_ICONHAND) == IDYES)
			{
				create_pic();
			}
			else
			{
				delimage(bg);
				cleardevice();
				put_image();
				main_menu();
			}
		}
		else
		{
			cout << "[Error]User did not select any file!" << "\n";
			if (MessageBox(NULL, "You did not select any file, please re-select later.\nSelect continue to generate LRC file (Yes) or return to the main interface (No)", "Error", MB_YESNO | MB_ICONHAND) == IDYES)
			{
				create_pic();
			}
			else
			{
				delimage(bg);
				cleardevice();
				put_image();
				main_menu();
			}
		}
	}
	size_t dot_pos = pic_path.find_last_of('.');
	string ext = pic_path.substr(dot_pos);
	transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext != ".jpg" && ext != ".png" && ext != ".gif" && ext != ".bmp" && ext != ".jpeg" && ext != ".tiff" && ext != ".tif" && ext != ".webp")
	{
		if (v_pak[0] == "zh")
		{
			if (MessageBox(NULL, "您选择的文件不是有效的图片格式\n选择继续生成LRC文件(是)或返回主界面(否)", "错误", MB_YESNO | MB_ICONHAND) == IDYES)
			{
				create_pic();
			}
			else
			{
				delimage(bg);
				cleardevice();
				put_image();
				main_menu();
			}
		}
		else
		{
			if (MessageBox(NULL, "The file you selected is not a valid image format.\nSelect continue to generate LRC file (Yes) or return to the main interface (No)", "Error", MB_YESNO | MB_ICONHAND) == IDYES)
			{
				create_pic();
			}
			else
			{
				delimage(bg);
				cleardevice();
				put_image();
				main_menu();
			}
		}
	}
	ifstream in(pic_path, ios::in);
	if (!in.is_open())
	{
		if (v_pak[0] == "zh")
		{
			cout << "[Error]文件打开失败！" << "\n";
			MessageBox(NULL, "文件打开失败,请检查是否存在此文件并重新选择", "错误", MB_OK | MB_ICONHAND);
		}
		else
		{
			cout << "[Error]File opening failed!" << "\n";
			if (MessageBox(NULL, "File opening failed, please check if the file exists and re-select\nSelect continue to generate LRC file (Yes) or return to the main interface (No)", "Error", MB_YESNO | MB_ICONHAND) == IDYES)
			{
				create_pic();
			}
			else
			{
				delimage(bg);
				cleardevice();
				put_image();
				main_menu();
			}
		}
		create_pic();
	}
	string x = "\\";
	string y = "/";
	size_t pos;
	while ((pos = pic_path.find(x)) != string::npos)
	{
		pic_path.replace(pos, 1, y);
	}
	if (v_pak[0] == "zh")
	{
		cout << "[Info]用户选择文件的绝对路径：" << pic_path << "\n";
	}
	else
	{
		cout << "[Info]User selected the absolute path of the file:" << pic_path << "\n";
	}
	pic_name = pic_path;
	pic_name.erase(0, pic_name.find_last_of("/") + 1);
	pic_name.erase(pic_name.find_last_of("."), pic_name.find_last_not_of(".txt") + 3);
	if (v_pak[0] == "zh")
	{
		cout << "[Info]用户选择文件的文件名:" << pic_name << "\n";
		tp = "您选择的文件绝对路径是:" + pic_path;
	}
	else
	{
		cout << "[Info]The user selected the name of the file:" << pic_name << "\n";
		tp = "The user selected the file absolute path is:" + pic_path;
	}
	const char *c_filename = tp.c_str();
	if (v_pak[0] == "zh")
	{
		if (MessageBox(NULL, c_filename, "检查文件是否正确？", MB_YESNO | MB_ICONASTERISK) == IDNO)
		{
			cout << "[Info]用户主动重新选择" << "\n";
			create_pic();
		}
		else
		{
			cout << "[Info]文件选择成功" << "\n";
		}
		MessageBox(NULL, "现在开始转换图片文件为LRC文件和MP3文件,输出在程序文件夹的output目录下", "提示", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]开始生成歌词文件" << "\n";
	}
	else
	{
		if (MessageBox(NULL, c_filename, "Check the file is correct?", MB_YESNO | MB_ICONASTERISK) == IDNO)
		{
			cout << "[Info]User actively re-select" << "\n";
			create_pic();
		}
		else
		{
			cout << "[Info]File selection successful" << "\n";
		}
		MessageBox(NULL, "Now start to convert the image file to LRC file and MP3 file, these files will output in the output directory of the program folder", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Start to generate LRC file" << "\n";
	}
	backdir();
	string ascii = "ascii-image-converter.exe " + pic_path + " -d " + width + "," + height + " --save-txt . --only-save", ascii_out = pic_name + "-ascii-art.txt";
	string pic_out_lrc = "output/" + pic_name + ".lrc", pic_out_mp3 = "output/" + pic_name + ".mp3";
	system(ascii.c_str());
	ifstream inputFile5(ascii_out, ios::in);
	ofstream out(pic_out_lrc, ios::out);
	out << "[00:01.00]";
	string temp;
	for (int i = 1; inputFile5 >> temp; i++)
	{
		temp.erase(0, temp.find_first_not_of(" \t"));
		out << temp;
	}
	inputFile5.close();
	out.close();
	remove(ascii_out.c_str());
	ifstream op(pic_out_lrc, ios::in);
	if (!op.is_open())
	{
		delall();
		if (v_pak[0] == "zh")
		{
			MessageBox(NULL, "未能成功生成歌词文件\n请检查图片文件是否损坏或被删除,然后重试", "错误", MB_OK | MB_ICONERROR);
		}
		else
		{
			MessageBox(NULL, "Failed to generate LRC file\nPlease check if the image file is damaged or deleted, then retry", "Error", MB_OK | MB_ICONERROR);
		}
		delimage(bg);
		cleardevice();
		put_image();
		main_menu();
	}
	op.close();
	if (v_pak[0] == "zh")
	{
		cout << "[Info]成功生成歌词文件" << "\n";
		cout << "[Info]现在创建与LRC文件配套的音频文件" << "\n"
			 << "[Info]本音频文件为静音文件,输出在程序文件夹的output目录下" << "\n"
			 << "[Info]请不要对程序做任何操作,等待程序提示下一步" << "\n";
		cout << "[Info]开始创建音频文件" << "\n";
	}
	else
	{
		cout << "[Info]Successfully generated LRC file" << "\n";
		cout << "[Info]Now create audio file" << "\n"
			 << "[Info]This audio file is a silent file,these files will output in the output directory of the program folder" << "\n"
			 << "[Info]Do not do any operation on the program, wait for the program to prompt the next step" << "\n";
		cout << "[Info]Start to create audio file" << "\n";
	}
	ifstream read(pic_out_mp3, ios::in);
	if (read.is_open())
	{
		if (v_pak[0] == "zh")
		{
			cout << "[Info]重复文件！现在删除原文件，创建新文件" << "\n";
		}
		else
		{
			cout << "[Info]Duplicate file! Now delete the original file and create a new file" << "\n";
		}
		remove(pic_out_mp3.c_str());
	}
	read.close();
	string silent = "silent.mp3";
	CopyFile(silent.c_str(), pic_out_mp3.c_str(), FALSE);
	ifstream op2(pic_out_mp3, ios::in);
	if (!op2.is_open())
	{
		delall();
		if (v_pak[0] == "zh")
		{
			MessageBox(NULL, "未能成功生成音频文件\n请检查图片文件是否损坏或被删除,然后重试", "错误", MB_OK | MB_ICONERROR);
		}
		else
		{
			MessageBox(NULL, "Failed to generate audio file\nPlease check if the image file is damaged or deleted, then retry", "Error", MB_OK | MB_ICONERROR);
		}
		delall();
		delimage(bg);
		cleardevice();
		put_image();
		main_menu();
	}
	op2.close();
	if (v_pak[0] == "zh")
	{
		MessageBox(NULL, "成功生成LRC文件和音频文件", "提示", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]成功生成音频文件" << "\n";
		MessageBox(NULL, "现在打开output文件夹\n请连接学习机/词典笔等音乐播放器，打开其对应音乐文件夹，将程序生成的音乐和歌词文件剪切至其中\n具体操作可观看软件作者B站视频", "提示", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]打开output文件夹" << "\n";
		system("start output");
		Sleep(5000);
		cout << "[Info]生成完成，选择是否继续生成" << "\n";
		if (MessageBox(NULL, "选择继续生成LRC文件(是)或返回主菜单(否)", "选择", MB_YESNO | MB_ICONASTERISK) == IDYES)
		{
			create_pic();
		}
		else
		{
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
	}
	else
	{
		MessageBox(NULL, "Successfully generated LRC file and audio file", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Successfully generated LRC file" << "\n";
		MessageBox(NULL, "Now open the output folder\nPlease connect the learning machine/dictionary pen etc. music player, open its corresponding music folder, and cut the program generated music and LRC file to it\nFor specific operation, please watch the software author's Youtube video", "Information", MB_OK | MB_ICONINFORMATION);
		cout << "[Info]Open the output folder" << "\n";
		system("start output");
		Sleep(5000);
		cout << "[Info]Generation complete, ask user continue or back to main menu" << "\n";
		if (MessageBox(NULL, "Do you want to continue generating LRC files?\nChoose \"Yes\" continue create file,choose \"No\" back to the main menu.", "Choose", MB_YESNO | MB_ICONASTERISK) == IDYES)
		{
			create_pic();
		}
		else
		{
			delimage(bg);
			cleardevice();
			put_image();
			main_menu();
		}
	}
}
// 删除出错文件
void delall()
{
	backdir();
	string rm = "del /q output\\*";
	system(rm.c_str());
}
// 选择设置项
void setting_opinion()
{
	setcolor(WHITE);
	setbkmode(TRANSPARENT);
	setfont(32, 0, "宋体");
	setcolor(BLUE);
	outtextxy(200, 110, "设置(Settings)");
	setcolor(WHITE);
	setfont(16, 0, "宋体");
	outtextxy(53, 170, "设置程序语言(Set Program Language)");
	outtextxy(53, 290, "设置LRC文件编码(Set Output Encoding)");
	bool choosechinese = false, chooseenglish = false, chooseansi = false, chooseutf_8 = false, chooseback = false;
	bool redraw2 = true;
	for (; is_run(); delay_fps(60))
	{
		while (mousemsg())
		{
			mouse_msg msg2 = getmouse();
			// 判断鼠标左键点击（左键按下确定位置，抬起为执行时刻）
			if (msg2.is_left())
			{
				if (msg2.is_down())
				{
					// 检测点击的按钮
					choosechinese = insideRectButton(&chinese, msg2.x, msg2.y);
					chooseenglish = insideRectButton(&english, msg2.x, msg2.y);
					chooseansi = insideRectButton(&ansi, msg2.x, msg2.y);
					chooseutf_8 = insideRectButton(&utf_8, msg2.x, msg2.y);
					chooseback = insideRectButton(&back, msg2.x, msg2.y);
				}
				else
				{
					backdir();
					if (v_pak[0] != "zh" && v_pak[0] != "en")
					{
						v_pak[0] = "zh";
						cout << "[Error]main.pak被错误修改，语言重新调整为中文！" << "\n";
						ofstream reset_la("main.pak", ios::out);
						for (int i = 0; i < 2; i++)
						{
							reset_la << v_pak[i] << "\n";
						}
						reset_la.close();
					}
					if (v_pak[1] != "ANSI" && v_pak[1] != "UTF_8")
					{
						v_pak[1] = "ANSI";
						cout << "[Error]main.pak被错误修改，编码重新调整为ANSI！" << "\n";
						ofstream reset_en("main.pak", ios::out);
						for (int i = 0; i < 2; i++)
						{
							reset_en << v_pak[i] << "\n";
						}
						reset_en.close();
					}
					// 左键抬起，判断是否需要执行事件响应
					if (choosechinese)
					{
						choosechinese = false;
						redraw2 = true;
						language = "zh";
						set_language();
					}
					if (chooseenglish)
					{
						chooseenglish = false;
						redraw2 = true;
						language = "en";
						set_language();
					}
					if (chooseansi)
					{
						chooseansi = false;
						redraw2 = true;
						encoding = "ANSI";
						set_encoding();
					}
					if (chooseutf_8)
					{
						chooseutf_8 = false;
						redraw2 = true;
						encoding = "UTF_8";
						set_encoding();
					}
					if (chooseback)
					{
						chooseback = false;
						redraw2 = true;
						delimage(bg);
						cleardevice();
						put_image();
						main_menu();
					}
				}
			}
		}
		// 绘制
		if (redraw2)
		{
			draw2();
			redraw2 = false;
		}
	}
}
// 设置语言并保存至main.pak
void set_language()
{
	if (v_pak[0] == language)
	{
		if (v_pak[0] == "zh")
		{
			MessageBox(NULL, "您选择的语言与之前重复，请重新选择", "提示", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		else
		{
			MessageBox(NULL, "The language you have selected is same as the previous one,please choose again.", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
	}
	else
	{
		if (language == "zh")
		{
			v_pak[0] = "zh";
		}
		else
		{
			v_pak[0] = "en";
		}
		ofstream set_la("main.pak", ios::out);
		for (int i = 0; i < 2; i++)
		{
			set_la << v_pak[i] << "\n";
		}
		set_la.close();
		system("cls");
		if (v_pak[0] == "zh")
		{
			cout << "LRC阅读器 v3.0" << "\n"
				 << "\n";
			cout << "作者B站:武侠Tux,作者Github:WuXiaTux" << "\n"
				 << "\n";
			cout << "感谢您使用本程序,敬请关注!" << "\n"
				 << "\n";
			cout << "本程序完全免费开源,请勿进行倒卖传播,如果您花钱购买此软件,请您立即退款并举报" << "\n"
				 << "\n";
			cout << "官方开源地址:https://github.com/WuXiaTux/LRC_Reader" << "\n"
				 << "\n";
			cout << "官方下载地址:https://kali-linux.lanzn.com/b00pzrssfa 密码:wuxiatux" << "\n"
				 << "\n"
				 << "\n";
			cout << "以下为程序日志输出" << "\n";
			cout << "[Info]语言成功设定为中文" << "\n";
			cout << "[Info]当前语言为中文，" << "当前LRC文件编码为" << v_pak[1] << "\n";
			MessageBox(NULL, "语言成功设定为中文", "提示", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		else
		{
			cout << "LRC Reader v3.0" << "\n"
				 << "\n";
			cout << "Author Bilibili:WuXiaTux,Author Github:WuXiaTux" << "\n"
				 << "\n";
			cout << "Thank you for using this program, please follow&star!" << "\n"
				 << "\n";
			cout << "This program is completely free and open source, please don't sell it.if you pay for this software,please refund and report the seller immediately." << "\n"
				 << "\n";
			cout << "Official open source address:https://github.com/WuXiaTux/LRC_Reader" << "\n"
				 << "\n";
			cout << "Official download address:https://kali-linux.lanzn.com/b00pzrssfa password:wuxiatux" << "\n"
				 << "\n"
				 << "\n";
			cout << "Program log output" << "\n";
			cout << "[Info]Language successfully set to English" << "\n";
			cout << "[Info]Current language is English, " << "Current LRC file encoding is " << v_pak[1] << "\n";
			MessageBox(NULL, "Language successfully set to English", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
	}
}
// 设置编码并保存至main.pak
void set_encoding()
{
	if (v_pak[1] == encoding)
	{
		if (v_pak[0] == "zh")
		{
			MessageBox(NULL, "您选择的编码与之前重复，请重新选择", "提示", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
		else
		{
			MessageBox(NULL, "The encoding you have selected is same as the previous one,please choose again.", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
		}
	}
	else
	{
		if (encoding == "ANSI")
		{
			v_pak[1] = "ANSI";
		}
		else
		{
			v_pak[1] = "UTF_8";
		}
		ofstream set_en("main.pak", ios::out);
		for (int i = 0; i < 2; i++)
		{
			set_en << v_pak[i] << "\n";
		}
		set_en.close();
		if (encoding == "ANSI")
		{
			if (v_pak[0] == "zh")
			{
				cout << "[Info]编码成功设置为" << encoding << "\n";
				MessageBox(NULL, "编码成功设置为ANSI", "提示", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
			}
			else
			{
				cout << "[Info]Encoding successfully set to " << encoding << "\n";
				MessageBox(NULL, "Encoding successfully set to ANSI", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
			}
		}
		else
		{
			if (v_pak[0] == "zh")
			{
				cout << "[Info]编码成功设置为" << encoding << "\n";
				MessageBox(NULL, "编码成功设置为UTF-8", "提示", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
			}
			else
			{
				cout << "[Info]Encoding successfully set to " << encoding << "\n";
				MessageBox(NULL, "Encoding successfully set to UTF-8", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
			}
		}
		if (v_pak[0] == "zh")
		{
			cout << "[Info]当前语言为中文，" << "当前LRC文件编码为" << v_pak[1] << "\n";
		}
		else
		{
			cout << "[Info]Current language is English, " << "Current LRC file encoding is " << v_pak[1] << "\n";
		}
	}
}
// 用于在create_pic()时输入播放器的宽和高
void enter_width_height()
{
	if (v_pak[0] == "zh")
	{
		cout << "[Info]请输入播放器显示的长和宽，输入示例：宽x高(中间的x是小写的英文字母x)：" << "\n";
	}
	else
	{
		cout << "[Info]Please enter the length and width displayed by the player.Ex: widthxheight(the middle \"x\" is a lowercase English letter \"X\".)" << "\n";
	}
	getline(cin, width_height);
	int x_tmp = width_height.find("x");
	if (x_tmp == -1)
	{
		if (v_pak[0] == "zh")
		{
			cout << "[Error]您未输入正确的格式，请重新输入" << "\n";
			MessageBox(NULL, "您未输入正确的格式，请重新输入", "错误", MB_OK | MB_ICONERROR);
		}
		else
		{
			cout << "[Error]You did not enter the width and height in the correct format, please re-enter" << "\n";
			MessageBox(NULL, "You did not enter the width and height in the correct format, please re-enter", "Error", MB_OK | MB_ICONERROR);
		}
		enter_width_height();
	}
	width = width_height.substr(0, x_tmp);
	height = width_height.substr(x_tmp + 1, width_height.size());
	if (width.size() == 0 || height.size() == 0)
	{
		if (v_pak[0] == "zh")
		{
			cout << "[Error]您未输入数字，请重新输入" << "\n";
			MessageBox(NULL, "您未输入数字，请重新输入", "错误", MB_OK | MB_ICONERROR);
		}
		else
		{
			cout << "[Error]You did not enter any number, please re-enter" << "\n";
			MessageBox(NULL, "You did not enter any number, please re-enter", "Error", MB_OK | MB_ICONERROR);
		}
		enter_width_height();
	}
	for (int i = 0; i < width.size(); i++)
	{
		int width_tmp = (int)width[i];
		if (width_tmp >= 48 && width_tmp <= 57)
		{
			continue;
		}
		else
		{
			if (v_pak[0] == "zh")
			{
				cout << "[Error]您未输入正确的格式，请重新输入" << "\n";
				MessageBox(NULL, "您未输入正确的格式，请重新输入", "错误", MB_OK | MB_ICONERROR);
			}
			else
			{
				cout << "[Error]You did not enter the width and height in the correct format, please re-enter" << "\n";
				MessageBox(NULL, "You did not enter the width and height in the correct format, please re-enter", "Error", MB_OK | MB_ICONERROR);
			}
			enter_width_height();
		}
	}
	for (int i = 0; i < height.size(); i++)
	{
		int height_tmp = (int)height[i];
		if (height_tmp >= 48 && height_tmp <= 57)
		{
			continue;
		}
		else
		{
			if (v_pak[0] == "zh")
			{
				cout << "[Error]您未输入正确的格式，请重新输入" << "\n";
				MessageBox(NULL, "您未输入正确的格式，请重新输入", "错误", MB_OK | MB_ICONERROR);
			}
			else
			{
				cout << "[Error]You did not enter the width and height in the correct format, please re-enter" << "\n";
				MessageBox(NULL, "You did not enter the width and height in the correct format, please re-enter", "Error", MB_OK | MB_ICONERROR);
			}
			enter_width_height();
		}
	}
}