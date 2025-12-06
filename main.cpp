// 导入头文件
#include <graphics.h>	  // EGE图形库：用于创建图形界面窗口
#include <ege/sys_edit.h> // EGE编辑框组件：用于GUI输入（虽然本代码主要用控制台输入）
#include <math.h>		  // 数学函数库
#include <iostream>		  // 标准输入输出流
#include <windows.h>	  // Windows API：用于文件对话框、消息框、系统调用等
#include <fstream>        // 文件流：用于读写TXT、LRC、PAK等文件
#include <vector>         // 向量容器：用于动态存储字符串数组
#include <string>         // 字符串类
#include <cstring>        // C风格字符串处理
#include <algorithm>      // 算法库：用于transform等操作
#include <cstdio>         // 标准I/O
#include <random>         // 随机数（代码中未深度使用，可能是保留库）
#include <chrono>         // 时间库
#include <sstream>        // 字符串流：用于格式化时间戳
#include <iomanip>        // I/O操纵符：用于设置输出格式（如补零）
#include <direct.h>       // 目录操作：如_mkdir等
#include <commdlg.h>      // 公共对话框：用于打开文件选择窗口
#include <tchar.h>        // 字符映射
#include <filesystem>     // C++17 文件系统库：用于路径和文件检查
#include <locale.h>       // 本地化设置
#include <Shlwapi.h>      // Shell轻量级API

// 定义命名空间，省去std::和filesystem::前缀
using namespace std;
using namespace filesystem;

// ---------------------------------------------------------
// 全局变量定义
// ---------------------------------------------------------
string str1 = "ffmpeg -stream_loop "; // ffmpeg命令前缀，用于循环流
string txt_path, txt_name;			  // txt_path: 用户选择的TXT绝对路径, txt_name: 提取出的文件名
string mp3_out, str2;				  // mp3_out: 输出MP3的完整路径, str2: 拼接后的ffmpeg完整命令
string pic_path, pic_name, tmp2;	  // pic_path: 图片路径, pic_name: 图片文件名, tmp2: 分割模式下的临时文件名
string language = "zh", temp_set, encoding = "ANSI"; // language: 当前语言, encoding: 目标编码格式
string width_height = "null", width, height, tp; // 图片转字符画时的宽及高
vector<string> v_read_txt; // 内存缓冲区：用于保存读取的TXT内容，以便处理后写入LRC
vector<string> countn;	   // 辅助向量：主要用于计算TXT文件的行数
vector<string> v_pak;	   // 配置向量：保存main.pak中的设置（语言、编码）
PIMAGE bg = NULL;		   // EGE图片指针：用于存储背景图片
int line_number = 0, count_cd = 0, cycle_times, line_last; // line_number: 总行数, cycle_times/line_last: 分割文件时的循环次数和剩余行数
bool notANSI = false;      // 标记文件是否非ANSI编码

// ---------------------------------------------------------
// 类与结构体定义
// ---------------------------------------------------------
// 矩形按钮结构体，用于自定义GUI按钮
struct RectButton
{
	int x, y;          // 左上角坐标
	int width, height; // 宽和高
};
// 定义主菜单按钮区域
RectButton button = {50, 300, 140, 60};   // "开始转换"按钮
RectButton button2 = {450, 300, 140, 60}; // "ASCII图像显示"按钮
RectButton button3 = {280, 400, 80, 40};  // "设置"按钮
// 定义设置菜单按钮区域
RectButton chinese = {53, 210, 80, 40};   // 设置中文
RectButton english = {450, 210, 80, 40};  // 设置英文
RectButton ansi = {53, 330, 80, 40};      // 设置ANSI编码
RectButton utf_8 = {450, 330, 80, 40};    // 设置UTF-8编码
RectButton back = {280, 400, 80, 40};     // 返回按钮
// 换行符测试结构体（代码中似乎未显式调用，可能用于流处理判定）
struct TestEOL
{
	bool operator()(char c)
	{
		las = c;
		return las == '\n';
	}
	char las;
};

// ---------------------------------------------------------
// 函数前置声明
// ---------------------------------------------------------
bool checkAndPrepareResources();    // 资源检查
string open_file_dialog();          // 打开TXT文件对话框
string open_file_dialog_p();        // 打开图片文件对话框
void backdir();                     // 返回程序根目录
string checkFileEncoding(const string &filePath); // 检查文件编码
string change_utf_8(const string &inputPath);     // UTF-8 转 ANSI
string ANSItoUTF8(const string &inputPath);       // ANSI 转 UTF-8
bool insideRectButton(const RectButton *button, int x, int y); // 判定鼠标点击
void drawRectButton(const RectButton *button);    // 绘制按钮
void draw();                        // 绘制主菜单
void draw2();                       // 绘制设置菜单
void put_image();                   // 绘制背景图
void main_menu();                   // 主菜单逻辑循环
void enter_program();               // TXT转LRC流程入口
void create_music();                // 创建完整MP3
void create_music2();               // 创建分割MP3
void create_lrc();                  // 创建完整LRC
void create_lrc2();                 // 创建分割LRC
void create_pic();                  // 图片转字符画LRC流程
void delall();                      // 清空output目录
void setting_opinion();             // 设置菜单逻辑循环
void set_language();                // 设置语言
void set_encoding();                // 设置编码
void enter_width_height();          // 输入字符画尺寸
string format_timestamp(int seconds); // 格式化时间戳

// ---------------------------------------------------------
// 辅助工具函数
// ---------------------------------------------------------
// 1. 获取对应语言的字符串
// 根据 v_pak[0] 判断当前语言，返回中文或英文字符串
string Lang(const string &zh, const string &en)
{
	return (v_pak.size() > 0 && v_pak[0] == "zh") ? zh : en;
}

// 2. 封装 MessageBox，自动处理双语
// 根据当前语言设置显示消息框的标题和内容
int ShowMsg(const string &zhMsg, const string &enMsg, const string &zhTitle, const string &enTitle, UINT uType = MB_OK)
{
	return MessageBox(NULL, Lang(zhMsg, enMsg).c_str(), Lang(zhTitle, enTitle).c_str(), uType);
}
// 3. 封装控制台输出，自动添加前缀和换行
// isError决定前缀是 [Info] 还是 [Error]
void Log(const string &zhTxt, const string &enTxt, bool isError = false)
{
	string prefix = isError ? "[Error]" : "[Info]";
	cout << prefix << Lang(zhTxt, enTxt) << endl;
}

// ---------------------------------------------------------
// 核心功能实现
// ---------------------------------------------------------
// 检查程序依赖文件和文件夹是否存在
bool checkAndPrepareResources()
{
	// 1. 检查或创建 output 文件夹
	if (!exists("output"))
	{
		try
		{
			create_directory("output");
			ShowMsg("output文件夹被删除！程序已自动重新创建 output 文件夹！\n请勿随意删除程序内部文件，否则会导致程序无法运行或报错！",
					"The output folder has been deleted! The program will automatically create it!\nPlease do not delete internal files.",
					"错误", "Error", MB_ICONERROR | MB_OK);
		}
		catch (const filesystem_error &e)
		{
			ShowMsg("无法创建 output 文件夹！", "Cannot create output folder!", "错误", "Error", MB_ICONERROR | MB_OK);
			return false;
		}
	}
	else if (!is_directory("output"))
	{
		ShowMsg("output 被占用且不是文件夹，请重新安装程序！",
				"The output folder is occupied and is not a folder. Please reinstall!",
				"错误", "Error", MB_ICONERROR | MB_OK);
		return false;
	}

	// 2. 检查关键依赖文件是否存在 (ffmpeg, 图片转换器, 编码转换库等)
	vector<string> critical_files = {"bg.jpg", "silent.mp3", "silent2.mp3", "ffmpeg.exe", "ascii-image-converter.exe", "main.pak", "uchardet.exe", "iconv.exe", "libuchardet.dll", "libcharset-1.dll", "libiconv-2.dll", "libstdc++-6.dll","libgcc_s_dw2-1.dll","libwinpthread-1.dll"};
	for (const auto &file : critical_files)
	{
		if (!exists(file) || !is_regular_file(file))
		{
			ShowMsg("程序依赖文件被删除，请重新安装程序！",
					"The program dependency file has been deleted, please reinstall!",
					"错误", "Error", MB_ICONERROR | MB_OK);
			return false;
		}
	}
	return true;
}
// 打开TXT文件选择对话框
string open_file_dialog()
{
	OPENFILENAME ofn;
	char szFile[260] = {0}; // 文件名缓冲区
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	// 设置过滤器，只显示TXT文件
	ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	string title = Lang("选择TXT文件", "Select TXT file");
	ofn.lpstrTitle = title.c_str();

	ofn.lpstrInitialDir = _T("C:\\"); // 设置默认目录
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	if (GetOpenFileName(&ofn))
	{
		return szFile; // 返回选择的文件路径
	}
	return ""; // 未选择或取消
}
// 打开图片文件选择对话框
string open_file_dialog_p()
{
	OPENFILENAME ofn;
	char szFile[260] = {0}; // 文件名缓冲区
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	// 设置过滤器，支持常见图片格式
	ofn.lpstrFilter = "\0*.jpg\0\0*.png\0\0*.gif\0\0*.bmp\0";
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	string title = Lang("选择图片文件", "Select image file");
	ofn.lpstrTitle = title.c_str();

	ofn.lpstrInitialDir = _T("C:\\");
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	if (GetOpenFileName(&ofn))
	{
		return szFile;
	}
	return "";
}
// 将工作目录切换回程序所在目录（防止文件对话框改变当前目录导致读取不到依赖文件）
void backdir()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	string programPath = buffer;
	size_t pos = programPath.find_last_of("\\/");
	string programDir = programPath.substr(0, pos);
	SetCurrentDirectory(programDir.c_str());
}
// 使用外部工具 uchardet 检查文件编码类型
string checkFileEncoding(const string &filePath)
{
	backdir();
	string tempResultFile = "temp_encoding_result.txt";
	// 调用系统命令 uchardet 分析编码并重定向输出
	string command = "uchardet \"" + filePath + "\" > " + tempResultFile + " 2>&1";
	int result = system(command.c_str());

	if (result != 0)
	{
		remove(tempResultFile.c_str());
		return "false";
	}

	ifstream resultFile(tempResultFile);
	if (!resultFile)
		return "false";

	string encoding;
	getline(resultFile, encoding);
	resultFile.close();
	remove(tempResultFile.c_str());

	// 去除空白字符
	encoding.erase(0, encoding.find_first_not_of(" \t\r\n"));
	encoding.erase(encoding.find_last_not_of(" \t\r\n") + 1);

	// 归一化编码名称
	if (encoding == "UTF-8" || encoding == "utf-8")
		return "UTF-8";
	else if (encoding == "GB18030" || encoding == "gb18030" ||
			 encoding == "GB2312" || encoding == "gb2312" ||
			 encoding == "ISO-8859-16" || encoding == "iso-8859-16")
		return "GB18030";
	else
		return encoding;
}
// 使用 iconv 将 UTF-8 文件转换为 ANSI (GB18030)
string change_utf_8(const string &inputPath)
{
	ifstream in(inputPath, ios::in);
	if (!in.is_open())
		return "unvalid file";
	in.close();
	backdir();
	size_t dot_pos = inputPath.find_last_of('.');
	string outputPath = inputPath.substr(0, dot_pos) + "_ansi.txt";
	string formatcmd = "iconv.exe -f UTF-8 -t GB18030 \"" + inputPath + "\" > \"" + outputPath + "\"";
	int result = system(formatcmd.c_str());
	if (result != 0)
		return "failed";
	else
		return "success";
}
// 使用 iconv 将 ANSI (GB18030) 文件转换为 UTF-8
string ANSItoUTF8(const string &inputPath)
{
	ifstream in(inputPath, ios::in);
	if (!in.is_open())
		return "unvalid file";
	in.close();
	backdir();
	size_t dot_pos = inputPath.find_last_of('.');
	string outputPath = inputPath.substr(0, dot_pos) + "_utf8.txt";
	string formatcmd = "iconv.exe -f GB18030 -t UTF-8 \"" + inputPath + "\" > \"" + outputPath + "\"";
	int result = system(formatcmd.c_str());
	if (result != 0)
		return "failed";
	else
		return "success";
}

// ---------------------------------------------------------
// 绘图相关函数
// ---------------------------------------------------------
// 检测鼠标坐标是否在按钮区域内
bool insideRectButton(const RectButton *button, int x, int y)
{
	return (x >= button->x) && (y >= button->y) && (x < button->x + button->width) && (y < button->y + button->height);
}
// 绘制矩形按钮
void drawRectButton(const RectButton *button)
{
	setfillcolor(EGERGB(0x1E, 0x90, 0xFF)); // 设置填充色为深天蓝
	bar(button->x, button->y, button->x + button->width, button->y + button->height);
}
// 绘制主菜单界面
void draw()
{
	drawRectButton(&button);
	drawRectButton(&button2);
	drawRectButton(&button3);
	// 绘制按钮文字
	outtextxy(88, 322, Lang("开始转换", "Convert TXT file").c_str());
	outtextxy(470, 322, Lang("ASCII图像显示", "ASCII Image").c_str());

	// 绘制设置按钮文字
	if (v_pak[0] == "zh")
		outtextxy(303, 411, "设置");
	else
	{
		outtextxy(488, 337, "Display");
		outtextxy(293, 411, "Setting");
	}
}
// 绘制设置菜单界面
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
// 加载并绘制背景图片
void put_image()
{
	if (bg != NULL)
	{
		delimage(bg); // 释放旧图片内存
		bg = NULL;
	}
	bg = newimage();
	getimage(bg, "bg.jpg"); // 从文件加载
	putimage(0, 0, bg);     // 绘制到屏幕
}
// ---------------------------------------------------------
// 程序主入口
// ---------------------------------------------------------

int main()
{
	backdir(); // 确保目录正确
	// 读取配置文件 main.pak
	ifstream check_la("main.pak", ios::in);
	while (getline(check_la, temp_set))
	{
		v_pak.push_back(temp_set);
	}
	check_la.close();

	// 配置文件完整性检查与修复
	if (v_pak.size() < 2)
	{
		v_pak.clear();
		v_pak.push_back("zh");	 // 默认语言
		v_pak.push_back("ANSI"); // 默认编码
		ofstream reset_file("main.pak", ios::out);
		reset_file << "zh\nANSI\n";
		reset_file.close();
		Log("main.pak被错误修改，语言重新调整为中文！", "main.pak was incorrectly modified, language reset to Chinese.", true);
	}
	// 校验语言设置
	if (v_pak[0] != "zh" && v_pak[0] != "en")
	{
		v_pak[0] = "zh";
		Log("main.pak被错误修改，语言重新调整为中文！", "main.pak was incorrectly modified, language reset to Chinese.", true);
		ofstream reset_la("main.pak", ios::out);
		for (int i = 0; i < 2; i++)
			reset_la << v_pak[i] << "\n";
		reset_la.close();
	}
	// 校验编码设置
	if (v_pak[1] != "ANSI" && v_pak[1] != "UTF_8")
	{
		v_pak[1] = "ANSI";
		Log("main.pak被错误修改，编码重新调整为ANSI！", "main.pak has been incorrectly modified, encoding reset to ANSI.", true);
		ofstream reset_en("main.pak", ios::out);
		for (int i = 0; i < 2; i++)
			reset_en << v_pak[i] << "\n";
		reset_en.close();
	}

	// 检查资源，如果缺失则退出或提示
	if (!checkAndPrepareResources())
	{
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
		return 0; // 避免继续执行
	}

	init_console(); // 初始化控制台窗口（用于日志和输入）
	initgraph(640, 480); // 初始化图形窗口 640x480

	setcaption(Lang("LRC阅读器 v3.0.1", "LRC Reader v3.0.1").c_str());
	ege_enable_aa(true); // 开启抗锯齿

	// 在控制台输出版权和说明信息
	if (v_pak[0] == "zh")
	{
		cout << "LRC阅读器 v3.0.1\n\n";
		cout << "作者B站:武侠Tux,作者Github:WuXiaTux\n\n";
		cout << "感谢您使用本程序,敬请关注!\n\n";
		cout << "本程序完全免费开源,请勿进行倒卖传播,如果您花钱购买此软件,请您立即退款并举报\n\n";
		cout << "官方开源地址:https://github.com/WuXiaTux/LRC_Reader\n\n";
		cout << "官方下载地址:https://kali-linux.lanzn.com/b00pzrssfa 密码:wuxiatux\n\n\n";
		cout << "以下为程序日志输出\n";
	}
	else
	{
		cout << "LRC Reader v3.0.1\n\n";
		cout << "Author Bilibili:WuXiaTux,Author Github:WuXiaTux\n\n";
		cout << "Thank you for using this program,please follow&star!\n\n";
		cout << "This program is completely free and open source,please don't sell it.if you pay for this software,please refund and report the seller immediately.\n\n";
		cout << "Official open source address:https://github.com/WuXiaTux/LRC_Reader\n\n";
		cout << "Official download address:https://kali-linux.lanzn.com/b00pzrssfa password:wuxiatux\n\n\n";
		cout << "Program log output\n";
	}

	Log("当前语言为中文，当前LRC文件编码为" + v_pak[1],
		"Current language is English, Current LRC file encoding is " + v_pak[1]);

	put_image();
	main_menu(); // 进入主菜单循环
	return 0;
}
// 主菜单逻辑
void main_menu()
{
	// 设置图形界面样式
	setcolor(WHITE);
	setbkmode(TRANSPARENT);
	setfont(32, 0, "宋体");
	setcolor(EGERGB(3, 155, 229));
	outtextxy(185, 110, Lang("LRC阅读器 v3.0.1", "LRC Reader v3.0.1").c_str());

	setcolor(WHITESMOKE);
	setfont(16, 0, "宋体");
	// 绘制版权文字
	if (v_pak[0] == "zh")
	{
		outtextxy(20, 150, "作者B站:武侠Tux,作者Github:WuXiaTux");
		outtextxy(20, 170, "感谢您使用本程序,敬请关注!");
		outtextxy(20, 190, "本程序完全免费开源,请勿进行倒卖传播,如果您花钱购买此软件,请您立即退款并举报!");
		outtextxy(20, 210, "官方开源地址:https://github.com/WuXiaTux/LRC_Reader");
		outtextxy(20, 230, "官方下载地址:https://kali-linux.lanzn.com/b00pzrssfa 密码:wuxiatux");
	}
	else
	{
		outtextxy(20, 150, "Author Bilibili:WuXiaTux,Author Github:WuXiaTux");
		outtextxy(20, 170, "Thank you for using this program,please follow&star!");
		outtextxy(20, 190, "This program is completely free and open source,please don't sell it.");
		outtextxy(20, 210, "if you pay for this software,please refund and report the seller immediately!");
		outtextxy(20, 225, "Official open source address:https://github.com/WuXiaTux/LRC_Reader");
		outtextxy(20, 245, "Official download address:https://kali-linux.lanzn.com/b00pzrssfa");
		outtextxy(20, 260, "password:wuxiatux");
	}

	bool clickButton = false, clickButton2 = false, clickButton3 = false;
	bool redraw = true;
	// 事件循环
	for (; is_run(); delay_fps(60))
	{
		while (mousemsg())
		{
			mouse_msg msg = getmouse();
			if (msg.is_left())
			{
				if (msg.is_down())
				{
					// 检测按下哪个按钮
					clickButton = insideRectButton(&button, msg.x, msg.y);
					clickButton2 = insideRectButton(&button2, msg.x, msg.y);
					clickButton3 = insideRectButton(&button3, msg.x, msg.y);
				}
				else
				{
					// 鼠标抬起时触发功能
					if (clickButton) // 功能：TXT 转 LRC
					{
						clickButton = false;
						redraw = true;
						ShowMsg("请选择要转换的TXT文件位置", "Please select the location of the TXT file to be converted",
								"提示", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
						enter_program();
					}
					if (clickButton2) // 功能：图片 转 ASCII字符画
					{
						clickButton2 = false;
						redraw = true;
						ShowMsg("本功能不支持滚动播放LRC文件的播放器\n请在左侧命令行界面输入播放器显示LRC文件的长和宽，按下Enter键完成",
								"This function does not support the player that play the LRC file in scrolling mode.\nPlease enter the width and height of the player in console, press Enter to continue",
								"提示", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);

						ShowMsg("请输入播放器显示的长和宽，输入示例：宽x高(中间的x是小写的英文字母x)",
								"Please enter the length and width displayed by the player.Ex: widthxheight",
								"提示", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);

						delimage(bg);
						bg = NULL;
						enter_width_height(); // 获取尺寸输入
						create_pic();
					}
					if (clickButton3) // 功能：设置
					{
						clickButton3 = false;
						redraw = true;
						delimage(bg);
						bg = NULL;
						cleardevice();
						put_image();
						setting_opinion();
					}
				}
			}
		}
		if (redraw)
		{
			draw(); // 重绘界面
			redraw = false;
		}
	}
}
// 进入TXT转LRC的主逻辑
void enter_program()
{
	txt_path = open_file_dialog(); // 选择文件
	if (txt_path.empty())
	{
		Log("用户未选择文件！", "User did not select any file!", true);
		if (ShowMsg("您未选择文件,请重新选择\n选择继续生成LRC文件(是)或返回主界面(否)",
					"You did not select any file, please select again\nContinue to generate LRC file (Yes) or return to the main interface (No)",
					"错误", "Error", MB_YESNO | MB_ICONHAND) == IDYES)
		{
			enter_program();
		}
		else
		{
			delimage(bg);
			bg = NULL;
			cleardevice();
			put_image();
			main_menu();
		}
		return;
	}

	ifstream in(txt_path, ios::in);
	if (!in.is_open())
	{
		Log("文件打开失败！", "File opening failed!", true);
		ShowMsg("文件打开失败,请检查是否存在此文件并重新选择", "File opening failed, please check if the file exists and select again", "错误", "Error", MB_OK | MB_ICONHAND);
		enter_program();
		return;
	}

	// 统计行数
	ifstream inputFile(txt_path, ios::in);
	string temp;
	countn.clear();
	line_number = 0;
	while (getline(inputFile, temp))
	{
		temp.erase(0, temp.find_first_not_of(" \t\r"));
		if (temp.empty())
			continue;
		countn.push_back(temp);
	}
	inputFile.close();
	line_number = countn.size();

	// 路径格式统一化（将 \ 替换为 /）
	string x = "\\";
	string y = "/";
	size_t pos;
	size_t offset = 0;
	while ((pos = txt_path.find(x, offset)) != string::npos)
	{
		txt_path.replace(pos, 1, y);
		offset = pos + 1;
	}
	// 提取文件名
	txt_name = txt_path;
	txt_name.erase(0, txt_name.find_last_of("/") + 1);
	txt_name.erase(txt_name.find_last_of("."), txt_name.find_last_not_of(".txt") + 3);

	Log("用户选择文件的绝对路径：" + txt_path, "The user selects the absolute path of the file:" + txt_path);
	Log("用户选择文件的文件名:" + txt_name, "The user selects the file name:" + txt_name);

	tp = Lang("您选择的文件绝对路径是:", "The file you selected is:") + txt_path;

	if (MessageBox(NULL, tp.c_str(), Lang("检查文件是否正确？", "Check the file is correct?").c_str(), MB_YESNO | MB_ICONASTERISK) == IDNO)
	{
		Log("用户主动重新选择", "User actively re-selects");
		enter_program();
		return;
	}
	else
	{
		Log("文件选择成功", "File selection is successful");
	}

	// 编码检查与转换逻辑
	string encodingType = checkFileEncoding(txt_path);
	// 如果文件是UTF-8但设置要求ANSI
	if (encodingType == "UTF-8")
	{
		if (v_pak[1] == "ANSI")
		{
			Log("检查txt文件格式", "Check txt file format");
			ShowMsg("识别到您选择的txt文件格式不为ANSI格式！建议您将txt文件转换为ANSI格式",
					"Check the txt file format is not ANSI format! It is recommended that you convert to ANSI.",
					"提示", "Information", MB_OK | MB_ICONINFORMATION);

			if (ShowMsg("选择转换txt文件为ANSI格式(是)或继续生成LRC文件(否)",
						"Select to convert txt file to ANSI format (Yes) or continue (No)",
						"选择是否转换", "Select whether to convert", MB_YESNO | MB_ICONASTERISK) == IDYES)
			{
				Log("txt文件格式为UTF-8", "TXT file format is UTF-8");
				Log("开始转换txt文件为GB18030(ANSI)格式", "Start converting TXT to GB18030(ANSI) format.");
				string Result = change_utf_8(txt_path);
				if (Result == "unvalid file")
				{
					Log("文件打开失败！", "File opening failed!", true);
					ShowMsg("文件打开失败,请检查是否存在此文件并重新选择", "File opening failed, please check if the file exists.", "错误", "Error", MB_OK | MB_ICONHAND);
					enter_program();
				}
				else if (Result == "failed")
				{
					Log("文件转换失败！", "File format failed!", true);
					ShowMsg("文件转换失败,请使用记事本另存为方法修改此文件编码格式",
							"File format failed, please use Notepad's 'save as' to modify encoding.",
							"错误", "Error", MB_OK | MB_ICONHAND);
					// 失败后返回主菜单
					ShowMsg("三秒后返回主菜单", "Three seconds later, return to the main menu", "提示", "Information", MB_OK | MB_ICONINFORMATION);
					Sleep(3000);
					delimage(bg);
					bg = NULL;
					cleardevice();
					put_image();
					main_menu();
				}
				else
				{
					ShowMsg("转换完成！请进入选择的目录检查文件是否生成，然后重新选择",
							"Conversion completed! Please check and re-select.",
							"提示", "Information", MB_OK | MB_ICONINFORMATION);
					Log("重新选择txt文件", "Re-select txt file");
					enter_program();
				}
			}
		}
	}
	// 如果文件是ANSI但设置要求UTF-8
	else if (encodingType == "GB18030")
	{
		if (v_pak[1] == "UTF_8")
		{
			Log("检查txt文件格式", "Check txt file format");
			ShowMsg("识别到您选择的txt文件格式不为UTF-8格式！建议您将txt文件转换为UTF-8格式",
					"Check the txt file format is not UTF-8 format! It is recommended that you convert to UTF-8.",
					"提示", "Information", MB_OK | MB_ICONINFORMATION);

			if (ShowMsg("选择转换txt文件为UTF-8格式(是)或继续生成LRC文件(否)",
						"Select to convert txt file to UTF-8 format (Yes) or continue (No)",
						"选择是否转换", "Select whether to convert", MB_YESNO | MB_ICONASTERISK) == IDYES)
			{
				Log("txt文件格式为GB18030", "TXT file format is GB18030");
				Log("开始转换txt文件为UTF-8格式", "Start converting TXT to UTF-8 format.");
				string Result = ANSItoUTF8(txt_path);
				// (转换结果处理同上，省略注释)
				if (Result == "unvalid file")
				{
					Log("文件打开失败！", "File opening failed!", true);
					ShowMsg("文件打开失败,请检查是否存在此文件并重新选择", "File opening failed, please check if the file exists.", "错误", "Error", MB_OK | MB_ICONHAND);
					enter_program();
				}
				else if (Result == "failed")
				{
					Log("文件转换失败！", "File format failed!", true);
					ShowMsg("文件转换失败,请使用记事本另存为方法修改此文件编码格式",
							"File format failed, please use Notepad's 'save as' to modify encoding.",
							"错误", "Error", MB_OK | MB_ICONHAND);

					ShowMsg("三秒后返回主菜单", "Three seconds later, return to the main menu", "提示", "Information", MB_OK | MB_ICONINFORMATION);
					Sleep(3000);
					delimage(bg);
					bg = NULL;
					cleardevice();
					put_image();
					main_menu();
				}
				else
				{
					ShowMsg("转换完成！请进入选择的目录检查文件是否生成，然后重新选择",
							"Conversion completed! Please check and re-select.",
							"提示", "Information", MB_OK | MB_ICONINFORMATION);
					Log("重新选择txt文件", "Re-select txt file");
					enter_program();
				}
			}
		}
	}
	else
	{
		// 其他编码警告
		ShowMsg("识别到文件格式不为UTF-8或GB2312！建议使用记事本转换编码！",
				"Check the txt file format is not UTF-8 or GB2312! Recommended to use Notepad to convert.",
				"提示", "Information", MB_OK | MB_ICONINFORMATION);

		if (ShowMsg("是否重新选择文件？选择'是'重新选择文件，选择'否'继续",
					"Do you want to re-select the file? Yes to re-select, No to continue",
					"是否重选文件？", "Do you want to re-select?", MB_YESNO | MB_ICONASTERISK) == IDYES)
		{
			Log("重新选择文件", "Re-select txt file");
			enter_program();
		}
	}

	// 大文件处理逻辑（超过500行提示分割）
	if (line_number > 500)
	{
		ShowMsg("本文件较大,建议您选择将本文件分割处理，以便减少进入播放器的时长\n分割后，文件会分成多个音乐和LRC文件的组合",
				"This file is large, recommended to divide the file.\nAfter division, it will be multiple Music+LRC combinations.",
				"提示", "Information", MB_OK | MB_ICONINFORMATION);

		if (ShowMsg("选择'是'启用文件分割模式(推荐)，选择'否'进入普通模式",
					"Select 'Yes' to enable file division mode (recommended), select 'No' to enter normal mode",
					"选择模式", "Select mode", MB_YESNO | MB_ICONASTERISK) == IDYES)
		{
			Log("文件分割模式开启", "File division mode is enabled");
			create_lrc2(); // 进入分割模式
		}
	}

	Log("普通转换模式", "Normal conversion mode");
	create_lrc(); // 进入普通模式
}
// 普通模式：创建LRC文件
void create_lrc()
{
	v_read_txt.clear();
	ShowMsg("现在开始转换TXT文件为LRC文件和mp3文件,输出在程序文件夹的output目录下",
			"Now start to convert TXT file to LRC file and MP3 file, output in output directory.",
			"提示", "Information", MB_OK | MB_ICONINFORMATION);
	Log("开始生成歌词文件", "Start to convert TXT file to LRC file and MP3 file.");

	string last;
	ifstream inputFile2(txt_path, ios::in);
	string line;
	int line_count = 0;
	// 逐行读取TXT，添加时间戳
	// 默认每行对应1秒：[00:01.00], [00:02.00]...
	while (getline(inputFile2, line))
	{
		line.erase(0, line.find_first_not_of(" \t"));
		if (line.empty())
			continue;
		line_count++;
		last = format_timestamp(line_count) + line;
		v_read_txt.push_back(last);
	}
	inputFile2.close();

	backdir();
	string tmp = "output/" + txt_name + ".lrc";
	ofstream out(tmp, ios::out);
	// 写入output目录
	for (size_t i = 0; i < v_read_txt.size(); i++)
		out << v_read_txt[i] << "\n";
	out.close();

	// 验证生成结果
	ifstream op(tmp, ios::in);
	if (!op.is_open())
	{
		delall();
		ShowMsg("未能成功生成歌词文件\n请检查txt文件是否损坏或被删除,然后重试",
				"Failed to create LRC file.\nPlease check if the txt file is damaged or deleted.",
				"错误", "Error", MB_OK | MB_ICONERROR);
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
		return;
	}
	op.close();
	Log("成功生成歌词文件", "Success to create LRC file.");

	create_music(); // 生成配套MP3

	ShowMsg("成功生成LRC文件和音频文件\n现在打开output文件夹\n请将文件放入播放器中",
			"Successfully generated LRC file and audio file\nNow open the output folder\nPlease put files into your player.",
			"提示", "Information", MB_OK | MB_ICONINFORMATION);

	Log("打开output文件夹", "Open the output folder");
	system("start output"); // 打开文件夹
	Log("生成完成，选择是否继续生成", "Generation complete, ask user continue or back to main menu");

	if (ShowMsg("选择继续生成LRC文件(是)或返回主菜单(否)",
				"Do you want to continue generating LRC files?\nChoose 'Yes' continue, choose 'No' back.",
				"选择", "Choose", MB_YESNO | MB_ICONASTERISK) == IDYES)
	{
		enter_program();
	}
	else
	{
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
	}
}
// 分割模式：创建LRC文件（每500行分割一次）
void create_lrc2()
{
	v_read_txt.clear();
	ShowMsg("现在开始转换TXT文件为LRC文件,输出在程序文件夹的output目录下",
			"Now start to convert TXT file to LRC file, output in output directory.",
			"提示", "Information", MB_OK | MB_ICONINFORMATION);
	Log("开始生成歌词文件", "Start to convert TXT file to LRC file.");

	cycle_times = ((line_number - line_number % 500) / 500); // 计算需要完整的500行循环次数
	line_last = line_number % 500; // 剩余行数
	string temp, last;
	ifstream inputFile3(txt_path, ios::in);

	// 处理完整的500行块
	for (int cycle = 1; cycle <= cycle_times; cycle++)
	{
		int i = 1;
		while (i <= 500 && getline(inputFile3, temp))
		{
			temp.erase(0, temp.find_first_not_of(" \t\r"));
			if (temp.empty())
				continue;
			last = format_timestamp(i) + temp;
			v_read_txt.push_back(last);
			i++;
		}

		backdir();
		// 文件名增加序号 (1), (2) 等
		string tmp = "output/" + txt_name + "(" + to_string(cycle) + ")" + ".lrc";
		ofstream out(tmp, ios::out);
		for (size_t k = 0; k < v_read_txt.size(); k++)
			out << v_read_txt[k] << "\n";
		out.close();

		ifstream op(tmp, ios::in);
		if (!op.is_open())
		{
			delall();
			ShowMsg("未能成功生成歌词文件\n请检查txt文件是否损坏或被删除,然后重试",
					"Failed to create LRC file.\nPlease check if the txt file is damaged or deleted.",
					"错误", "Error", MB_OK | MB_ICONERROR);
			Log("三秒后返回主界面", "Three seconds later, return to the main menu");
			Sleep(3000);
			delimage(bg);
			bg = NULL;
			cleardevice();
			put_image();
			main_menu();
			return;
		}
		op.close();
		v_read_txt.clear();
	}

	// 处理剩余行数
	int i = 1;
	while (i <= line_last && getline(inputFile3, temp))
	{
		temp.erase(0, temp.find_first_not_of(" \t\r"));
		if (temp.empty())
			continue;
		last = format_timestamp(i) + temp;
		v_read_txt.push_back(last);
		i++;
	}

	backdir();
	int cycle_temp = cycle_times + 1;
	tmp2 = "output/" + txt_name + "(" + to_string(cycle_temp);
	string tmp22 = tmp2 + ").lrc";
	ofstream out(tmp22, ios::out);
	for (size_t k = 0; k < v_read_txt.size(); k++)
		out << v_read_txt[k] << "\n";
	out.close();

	// 验证剩余部分生成
	ifstream op(tmp22, ios::in);
	if (!op.is_open())
	{
		delall();
		ShowMsg("未能成功生成歌词文件", "Failed to create LRC file", "错误", "Error", MB_OK | MB_ICONERROR);
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
		return;
	}
	op.close();
	v_read_txt.clear();
	inputFile3.close();
	Log("成功生成歌词文件", "Successfully generated LRC file");

	create_music2(); // 生成分割的MP3

	ShowMsg("成功生成LRC文件和音频文件\n现在打开output文件夹",
			"Successfully generated LRC file and audio file\nNow open the output folder",
			"提示", "Information", MB_OK | MB_ICONINFORMATION);

	Log("成功生成音频文件", "Successfully generated audio file");
	Log("打开output文件夹", "Open the output folder");
	system("start output");
	Log("生成完成，选择是否继续生成", "Generation complete, ask user continue or back to main menu");

	if (ShowMsg("选择继续生成LRC文件(是)或返回主菜单(否)",
				"Do you want to continue generating LRC files?",
				"选择", "Choose", MB_YESNO | MB_ICONASTERISK) == IDYES)
	{
		enter_program();
	}
	else
	{
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
	}
}
// 普通模式：生成MP3
void create_music()
{
	Log("现在创建与LRC文件配套的音频文件\n本音频文件为静音文件,输出在output目录下\n请不要操作程序，等待提示",
		"Now create audio file\nThis audio file is a silent file, output in output directory\nDo not operate the program, wait for prompt");

	mp3_out = "output/" + txt_name + ".mp3";
	// 检查重复
	ifstream read(mp3_out, ios::in);
	if (read.is_open())
	{
		Log("重复文件！现在删除原文件，创建新文件", "Duplicate file! Now delete the original file and create a new file");
		remove(mp3_out.c_str());
	}
	read.close();
	// 拼接FFmpeg命令：循环生成静音音频，时长由 line_number 决定
	string temp1 = to_string(line_number) + " ";
	str2 = str1 + temp1 + "-i silent.mp3 -c copy \"output/" + txt_name + ".mp3\" -y";

	Log("ffmpeg开始创建音频文件", "ffmpeg start create audio files");

	int ret = system(str2.c_str());
	if (ret != 0)
	{
		ShowMsg("ffmpeg错误\n请查看ffmpeg输出，从中查看问题并重试", "ffmpeg error\nPlease check the ffmpeg output and retry", "错误", "Error", MB_OK | MB_ICONERROR);
		delall();
		ShowMsg("三秒后返回主菜单", "Three seconds later, return to the main menu", "提示", "Information", MB_OK | MB_ICONINFORMATION);
		Sleep(3000);
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
		return;
	}
	// 验证生成
	ifstream read2(mp3_out, ios::in);
	if (!read2.is_open())
	{
		ShowMsg("未能成功生成音频文件\n请查看ffmpeg输出", "Failed to generate audio file\nPlease check ffmpeg output", "错误", "Error", MB_OK | MB_ICONERROR);
		delall();
		ShowMsg("三秒后返回主菜单", "Three seconds later, return to the main menu", "提示", "Information", MB_OK | MB_ICONINFORMATION);
		Sleep(3000);
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
	}
	read2.close();
}
// 分割模式：生成MP3
void create_music2()
{
	Log("现在创建与LRC文件配套的音频文件\n本音频文件为静音文件,输出在output目录下\n请不要操作程序",
		"Now create audio file\nThis audio file is a silent file, output in output directory\nDo not operate the program");

	string silent2 = "silent2.mp3"; // 预制的500秒静音文件？或者长静音文件
	// 为每个完整循环复制静音文件
	for (int i = 1; i <= cycle_times; i++)
	{
		backdir();
		string temp_mp3 = "output/" + txt_name + "(" + to_string(i) + ")" + ".mp3";
		ifstream read(temp_mp3, ios::in);
		if (read.is_open())
		{
			Log("重复文件！现在删除原文件，创建新文件", "Duplicate file! Now delete the original file and create a new file");
			remove(temp_mp3.c_str());
		}
		read.close();
		CopyFile(silent2.c_str(), temp_mp3.c_str(), FALSE); // 直接复制文件

		// 验证
		ifstream read2(temp_mp3, ios::in);
		if (!read2.is_open())
		{
			ShowMsg("未能成功生成音频文件\n请查看ffmpeg输出", "Failed to generate audio file\nPlease check ffmpeg output", "错误", "Error", MB_OK | MB_ICONERROR);
			delall();
			ShowMsg("三秒后返回主菜单", "Three seconds later, return to the home page", "提示", "Information", MB_OK | MB_ICONINFORMATION);
			Sleep(3000);
			delimage(bg);
			bg = NULL;
			cleardevice();
			put_image();
			main_menu();
			return;
		}
		read2.close();
	}
	// 为剩余行数生成特定长度的音频
	if (line_last > 0)
	{
		str2 = str1 + to_string(line_last) + " -i silent.mp3 -c copy \"" + tmp2 + ").mp3\" -y";
		Log("ffmpeg开始创建音频文件", "ffmpeg start create audio files");

		int ret1 = system(str2.c_str());
		if (ret1 != 0)
		{
			ShowMsg("ffmpeg错误\n请查看ffmpeg输出", "ffmpeg error\nPlease check the ffmpeg output", "错误", "Error", MB_OK | MB_ICONERROR);
			delall();
			ShowMsg("三秒后返回主菜单", "Three seconds later, return to the home page", "提示", "Information", MB_OK | MB_ICONINFORMATION);
			Sleep(3000);
			delimage(bg);
			bg = NULL;
			cleardevice();
			put_image();
			main_menu();
			return;
		}
		// 验证
		string tmp3 = tmp2 + ").mp3";
		ifstream read2(tmp3, ios::in);
		if (!read2.is_open())
		{
			ShowMsg("未能成功生成音频文件\n请查看ffmpeg输出", "Failed to generate audio file\nPlease check ffmpeg output", "错误", "Error", MB_OK | MB_ICONERROR);
			delall();
			ShowMsg("三秒后返回主菜单", "Three seconds later, return to the home page", "提示", "Information", MB_OK | MB_ICONINFORMATION);
			Sleep(3000);
			delimage(bg);
			bg = NULL;
			cleardevice();
			put_image();
			main_menu();
		}
		read2.close();
	}
}
// 图片转字符画LRC流程
void create_pic()
{
	pic_path = open_file_dialog_p(); // 选择图片
	if (pic_path.empty())
	{
		Log("用户未选择文件！", "User did not select any file!", true);
		if (ShowMsg("您未选择文件,请重新选择\n选择继续生成LRC文件(是)或返回主界面(否)",
					"You did not select any file, please re-select.\nContinue (Yes) or return (No)",
					"错误", "Error", MB_YESNO | MB_ICONHAND) == IDYES)
		{
			create_pic();
		}
		else
		{
			delimage(bg);
			bg = NULL;
			cleardevice();
			put_image();
			main_menu();
		}
		return;
	}

	// 检查扩展名
	size_t dot_pos = pic_path.find_last_of('.');
	string ext = pic_path.substr(dot_pos);
	transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // 转小写
	if (ext != ".jpg" && ext != ".png" && ext != ".gif" && ext != ".bmp" && ext != ".jpeg" && ext != ".tiff" && ext != ".tif" && ext != ".webp")
	{
		if (ShowMsg("您选择的文件不是有效或支持的图片格式\n选择继续(是)或返回(否)",
					"Invalid image format.\nContinue (Yes) or return (No)",
					"错误", "Error", MB_YESNO | MB_ICONHAND) == IDYES)
		{
			create_pic();
		}
		else
		{
			delimage(bg);
			bg = NULL;
			cleardevice();
			put_image();
			main_menu();
		}
		return;
	}

	ifstream in(pic_path, ios::in);
	if (!in.is_open())
	{
		Log("文件打开失败！", "File opening failed!", true);
		if (ShowMsg("文件打开失败,请检查是否存在此文件并重新选择\n选择继续(是)或返回(否)",
					"File opening failed, re-select.\nContinue (Yes) or return (No)",
					"错误", "Error", MB_YESNO | MB_ICONHAND) == IDYES)
		{
			create_pic();
		}
		else
		{
			delimage(bg);
			bg = NULL;
			cleardevice();
			put_image();
			main_menu();
		}
		return;
	}

	// 路径标准化
	string x = "\\";
	string y = "/";
	size_t pos;
	while ((pos = pic_path.find(x)) != string::npos)
		pic_path.replace(pos, 1, y);

	Log("用户选择文件的绝对路径：" + pic_path, "User selected absolute path:" + pic_path);

	pic_name = pic_path;
	pic_name.erase(0, pic_name.find_last_of("/") + 1);
	pic_name.erase(pic_name.find_last_of("."), pic_name.find_last_not_of(".txt") + 3);

	Log("用户选择文件的文件名:" + pic_name, "User selected filename:" + pic_name);
	tp = Lang("您选择的文件绝对路径是:", "The user selected the file absolute path is:") + pic_path;

	if (MessageBox(NULL, tp.c_str(), Lang("检查文件是否正确？", "Check the file is correct?").c_str(), MB_YESNO | MB_ICONASTERISK) == IDNO)
	{
		Log("用户主动重新选择", "User actively re-select");
		create_pic();
		return;
	}
	else
	{
		Log("文件选择成功", "File selection successful");
	}

	ShowMsg("现在开始转换图片文件为LRC文件和MP3文件,输出在output目录下",
			"Start to convert image to LRC and MP3, output in output directory",
			"提示", "Information", MB_OK | MB_ICONINFORMATION);
	Log("开始生成歌词文件", "Start to generate LRC file");

	backdir();
	// 调用 ascii-image-converter 将图片转为字符画TXT
	string ascii = "ascii-image-converter.exe \"" + pic_path + "\" -d " + width + "," + height + " --save-txt . --only-save";
	string ascii_out = pic_name + "-ascii-art.txt";
	string pic_out_lrc = "output/" + pic_name + ".lrc";
	string pic_out_mp3 = "output/" + pic_name + ".mp3";

	system(ascii.c_str());

	// 将字符画TXT内容读取并加上LRC时间戳（仅第一行加时间戳，后续作为同一时刻的歌词？）
	ifstream inputFile5(ascii_out, ios::in);
	ofstream out(pic_out_lrc, ios::out);
	out << "[00:01.00]"; // 统一时间戳
	string temp;
	while (getline(inputFile5, temp))
	{
		temp.erase(0, temp.find_first_not_of(" \t"));
		out << temp; // 写入字符
	}
	inputFile5.close();
	out.close();
	remove(ascii_out.c_str()); // 删除中间文件

	// 验证
	ifstream op(pic_out_lrc, ios::in);
	if (!op.is_open())
	{
		delall();
		ShowMsg("未能成功生成歌词文件\n请检查图片文件是否损坏或被删除", "Failed to generate LRC file\nPlease check image file", "错误", "Error", MB_OK | MB_ICONERROR);
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
		return;
	}
	op.close();

	Log("成功生成歌词文件", "Successfully generated LRC file");
	Log("现在创建与LRC文件配套的音频文件\n本音频文件为静音文件\n请不要对程序做任何操作",
		"Now create audio file\nThis audio file is a silent file\nDo not do any operation");
	Log("开始创建音频文件", "Start to create audio file");

	// 复制静音文件作为MP3
	ifstream read(pic_out_mp3, ios::in);
	if (read.is_open())
	{
		Log("重复文件！现在删除原文件，创建新文件", "Duplicate file! Now delete original file and create new file");
		remove(pic_out_mp3.c_str());
	}
	read.close();
	string silent = "silent.mp3";
	CopyFile(silent.c_str(), pic_out_mp3.c_str(), FALSE);

	// 验证
	ifstream op2(pic_out_mp3, ios::in);
	if (!op2.is_open())
	{
		delall();
		ShowMsg("未能成功生成音频文件\n请检查图片文件是否损坏或被删除", "Failed to generate audio file\nPlease check image file", "错误", "Error", MB_OK | MB_ICONERROR);
		delall();
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
		return;
	}
	op2.close();

	ShowMsg("成功生成LRC文件和音频文件\n现在打开output文件夹",
			"Successfully generated LRC file and audio file\nNow open the output folder",
			"提示", "Information", MB_OK | MB_ICONINFORMATION);

	Log("成功生成音频文件", "Successfully generated audio file");
	Log("打开output文件夹", "Open the output folder");
	system("start output");
	Log("生成完成，选择是否继续生成", "Generation complete, ask user continue or back to main menu");

	if (ShowMsg("选择继续生成LRC文件(是)或返回主菜单(否)",
				"Do you want to continue generating LRC files?\nChoose 'Yes' continue, choose 'No' back.",
				"选择", "Choose", MB_YESNO | MB_ICONASTERISK) == IDYES)
	{
		create_pic();
	}
	else
	{
		delimage(bg);
		bg = NULL;
		cleardevice();
		put_image();
		main_menu();
	}
}
// 删除 output 目录下的所有文件
void delall()
{
	backdir();
	string rm = "del /q output\\*";
	system(rm.c_str());
}
// 设置菜单逻辑
void setting_opinion()
{
	// 绘制标题
	setcolor(WHITE);
	setbkmode(TRANSPARENT);
	setfont(32, 0, "宋体");
	setcolor(BLUE);
	outtextxy(200, 110, Lang("设置(Settings)", "Settings").c_str());
	setcolor(WHITE);
	setfont(16, 0, "宋体");
	outtextxy(53, 170, Lang("设置程序语言(Set Program Language)", "Set Program Language").c_str());
	outtextxy(53, 290, Lang("设置LRC文件编码(Set Output Encoding)", "Set Output Encoding").c_str());

	bool choosechinese = false, chooseenglish = false, chooseansi = false, chooseutf_8 = false, chooseback = false;
	bool redraw2 = true;
	// 事件循环
	for (; is_run(); delay_fps(60))
	{
		while (mousemsg())
		{
			mouse_msg msg2 = getmouse();
			if (msg2.is_left())
			{
				if (msg2.is_down())
				{
					// 检测点击
					choosechinese = insideRectButton(&chinese, msg2.x, msg2.y);
					chooseenglish = insideRectButton(&english, msg2.x, msg2.y);
					chooseansi = insideRectButton(&ansi, msg2.x, msg2.y);
					chooseutf_8 = insideRectButton(&utf_8, msg2.x, msg2.y);
					chooseback = insideRectButton(&back, msg2.x, msg2.y);
				}
				else
				{
					backdir();
					// 配置文件防篡改检查
					if (v_pak[0] != "zh" && v_pak[0] != "en")
					{
						v_pak[0] = "zh";
						Log("main.pak被错误修改，语言重新调整为中文！", "main.pak modified error, reset to Chinese.", true);
						ofstream reset_la("main.pak", ios::out);
						for (int i = 0; i < 2; i++)
							reset_la << v_pak[i] << "\n";
						reset_la.close();
					}
					if (v_pak[1] != "ANSI" && v_pak[1] != "UTF_8")
					{
						v_pak[1] = "ANSI";
						Log("main.pak被错误修改，编码重新调整为ANSI！", "main.pak modified error, reset to ANSI.", true);
						ofstream reset_en("main.pak", ios::out);
						for (int i = 0; i < 2; i++)
							reset_en << v_pak[i] << "\n";
						reset_en.close();
					}

					// 执行设置修改
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
						bg = NULL;
						cleardevice();
						put_image();
						main_menu(); // 返回主菜单
					}
				}
			}
		}
		if (redraw2)
		{
			draw2(); // 重绘设置界面
			redraw2 = false;
		}
	}
}
// 更改语言逻辑
void set_language()
{
	if (v_pak[0] == language)
	{
		ShowMsg("您选择的语言与之前重复，请重新选择",
				"The language you have selected is same as the previous one, please choose again.",
				"提示", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
	}
	else
	{
		v_pak[0] = (language == "zh") ? "zh" : "en";

		// 写入配置文件
		ofstream set_la("main.pak", ios::out);
		for (int i = 0; i < 2; i++)
			set_la << v_pak[i] << "\n";
		set_la.close();
		system("cls"); // 清空控制台

		// 重印头部信息
		if (v_pak[0] == "zh")
		{
			cout << "LRC阅读器 v3.0.1\n\n";
			cout << "作者B站:武侠Tux,作者Github:WuXiaTux\n\n";
			cout << "感谢您使用本程序,敬请关注!\n\n";
			cout << "本程序完全免费开源,请勿进行倒卖传播,如果您花钱购买此软件,请您立即退款并举报\n\n";
			cout << "官方开源地址:https://github.com/WuXiaTux/LRC_Reader\n\n";
			cout << "官方下载地址:https://kali-linux.lanzn.com/b00pzrssfa 密码:wuxiatux\n\n\n";
			cout << "以下为程序日志输出\n";
		}
		else
		{
			cout << "LRC Reader v3.0.1\n\n";
			cout << "Author Bilibili:WuXiaTux,Author Github:WuXiaTux\n\n";
			cout << "Thank you for using this program,please follow&star!\n\n";
			cout << "This program is completely free and open source,please don't sell it.if you pay for this software,please refund and report the seller immediately.\n\n";
			cout << "Official open source address:https://github.com/WuXiaTux/LRC_Reader\n\n";
			cout << "Official download address:https://kali-linux.lanzn.com/b00pzrssfa password:wuxiatux\n\n\n";
			cout << "Program log output\n";
		}

		Log("语言成功设定为" + string(v_pak[0] == "zh" ? "中文" : "English"),
			"Language successfully set to " + string(v_pak[0] == "zh" ? "Chinese" : "English"));
		Log("当前语言为" + string(v_pak[0] == "zh" ? "中文" : "English") + "，当前LRC文件编码为" + v_pak[1],
			"Current language is " + string(v_pak[0] == "zh" ? "Chinese" : "English") + ", Current LRC file encoding is " + v_pak[1]);

		ShowMsg("语言成功设定为" + string(v_pak[0] == "zh" ? "中文" : "English"),
				"Language successfully set to " + string(v_pak[0] == "zh" ? "Chinese" : "English"),
				"提示", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
	}
}
// 更改编码逻辑
void set_encoding()
{
	if (v_pak[1] == encoding)
	{
		ShowMsg("您选择的编码与之前重复，请重新选择",
				"The encoding you have selected is same as the previous one, please choose again.",
				"提示", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);
	}
	else
	{
		v_pak[1] = (encoding == "ANSI") ? "ANSI" : "UTF_8";

		// 写入配置文件
		ofstream set_en("main.pak", ios::out);
		for (int i = 0; i < 2; i++)
			set_en << v_pak[i] << "\n";
		set_en.close();

		string encName = (encoding == "ANSI") ? "ANSI" : "UTF-8";
		Log("编码成功设置为" + encName, "Encoding successfully set to " + encName);
		ShowMsg("编码成功设置为" + encName, "Encoding successfully set to " + encName,
				"提示", "Information", MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);

		Log("当前语言为" + string(v_pak[0] == "zh" ? "中文" : "English") + "，当前LRC文件编码为" + v_pak[1],
			"Current language is " + string(v_pak[0] == "zh" ? "Chinese" : "English") + ", Current LRC file encoding is " + v_pak[1]);
	}
}
// 获取字符画尺寸输入
void enter_width_height()
{
	Log("请输入播放器显示的长和宽，输入示例：宽x高(中间的x是小写的英文字母x)：",
		"Please enter the length and width displayed by the player.Ex: widthxheight");

	getline(cin, width_height);
	int x_tmp = width_height.find("x");

	// 简单验证格式：必须包含 'x' 且两边都是数字
	bool valid = true;
	if (x_tmp == -1)
		valid = false;
	else
	{
		width = width_height.substr(0, x_tmp);
		height = width_height.substr(x_tmp + 1);
		if (width.empty() || height.empty())
			valid = false;
		// 检查数字
		for (char c : width)
			if (!isdigit(c))
				valid = false;
		for (char c : height)
			if (!isdigit(c))
				valid = false;
	}

	if (!valid)
	{
		Log("您未输入正确的格式/数字，请重新输入", "Invalid format/number, please re-enter", true);
		ShowMsg("您未输入正确的格式，请重新输入", "Invalid format, please re-enter", "错误", "Error", MB_OK | MB_ICONERROR);
		enter_width_height(); // 递归重试
	}
}
// 格式化秒数为 LRC 时间戳字符串 [MM:SS.00]
string format_timestamp(int seconds)
{
	std::ostringstream oss;
	oss << "[" << std::setw(2) << std::setfill('0') << (seconds / 60) // 分
		<< ":" << std::setw(2) << std::setfill('0') << (seconds % 60) // 秒
		<< ".00]"; // 毫秒固定为00
	return oss.str();
}
