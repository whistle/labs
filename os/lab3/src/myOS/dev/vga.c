#include "../i386/io.h"
#include "../lib/color.h"

//VGA地址
#define VGA_BASE (0xB8000)
//行列宽度
#define LineLenth (80)
#define ColumnLenth (23)
//记录读写地址
static unsigned char* VGA_Base;

//记录光标位置
static int m_CursorX;
static int m_CursorY;

//设置光标
void SetCursor(int x, int y)
{
	unsigned short pos = y * 80 + x;
	outb(0x3d4, 14);
	outb(0x3d5, (pos >> 8) & 0xff);
	outb(0x3d4, 15);
	outb(0x3d5, pos & 0xff);
	//同步记录的光标位置
	m_CursorX = x;
	m_CursorY = y;
}

//指定坐标的VGA输出字符
void VGAputcharAtpoint(int line, int column, unsigned char c, int color)
{
	//坐标校正
	if (column > ColumnLenth)
	{
		line += column / LineLenth;
		column %= LineLenth;
	}
	if (line > LineLenth)line = LineLenth;
	//计算光标位置
	unsigned int pos = line * LineLenth + column;
	//写入字符以及颜色
	*(char*)(VGA_BASE + pos * 2) = c;
	*(char*)(VGA_BASE + pos * 2 + 1) = color;
}

//指定坐标的VGA输出格式化字符串
void append2screenAtpoint(int line, int column, char* str, int color)
{
	int i = 0;
	//判断字符串的结尾
	while (*str != '\0')
	{
		VGAputcharAtpoint(line, column + i, *str, color);
		str++;
		i++;
	}
}

//实现滚屏一行
void ScrollUp()
{
	for (int line = 0; line < ColumnLenth; line++)
		for (int i = 0; i < 160; i++)
		{
			*(VGA_Base + line * 160 + i) = *(VGA_Base + (line + 1) * 160 + i);
		}
	m_CursorY = ColumnLenth - 1;
}

//VGA输出字符
void VGAputchar(unsigned char c, int color)
{
	switch (c)
	{
		//特殊字符的处理
	case '\r':
	case '\n':
		m_CursorX = 0;
		m_CursorY++;
		break;
	case '\t':
		m_CursorX = (m_CursorX / 8 + 1) * 8;
		if (m_CursorX >= LineLenth)
		{
			m_CursorX -= LineLenth;
			m_CursorY++;
		}
		break;
		//正常字符的输出
	default:
	{
		//计算光标位置
		unsigned int pos = m_CursorY * LineLenth + m_CursorX;
		//写入字符以及颜色
		*(VGA_Base + pos * 2) = c;
		*(VGA_Base + pos * 2 + 1) = color;
		pos++;
		m_CursorX = pos % LineLenth;
		m_CursorY = pos / LineLenth;
	}
	break;
	}
	//如果行数达到最大则滚屏
	if (m_CursorY == ColumnLenth)
		ScrollUp();
	//同步光标位置
	SetCursor(m_CursorX, m_CursorY);
}

//清屏
void clear_screen(void)
{
	//初始化记录的地址
	VGA_Base = (char*)VGA_BASE;
	unsigned char* p;
	p = (char*)VGA_BASE;
	//空格覆盖实现清屏
	for (int i = 0; i < 1000; i++)
		* (p + i * 2) = ' ';
}

//VGA输出格式化字符串
void append2screen(char* str, int color)
{
	//判断字符串的结尾
	while (*str != '\0')
	{
		VGAputchar(*str, color);
		str++;
	}
}