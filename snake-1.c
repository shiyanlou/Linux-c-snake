#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ncurses.h>

  
#define TBool            int
#define True             1
#define False            0    
#define SHAPE_FOOD       '@'
#define SHAPE_SNAKE      '#'
#define GAMEWIN_YLEN     20
#define GAMEWIN_XLEN     60
#define LOGWIN_YLEN      7
#define LOGWIN_XLEN      (GAMEWIN_XLEN)
#define LOGBUF_NUM       (LOGWIN_YLEN-2)
#define LOGBUF_LEN       (GAMEWIN_XLEN-2)
#define MAXLEVEL         12
  
#define GetSnakeTail(s)  ((s)->head->front)
  
WINDOW *logwin; /* 声明一个信息展示窗口 */
#define INITRUNLOG()     logwin = newlogw()
#define RUNLOG(str)      runlog(logwin, str)
#define DESTROYRUNLOG()  delwin(logwin)
  
int g_level; // 玩家等级
  
WINDOW* newgamew();
WINDOW* newlogw();
void runlog(WINDOW *win, char *str);
void cleanline(WINDOW *win, int y, int x);
  
int main()
{
    initscr();  /* 初始化，进入ncurses模式 */
    
    raw();      /* 禁止行缓冲，可以立即见到结果 */
    noecho();   /* 不在终端上显示控制字符，比如Ctrl+c */
    keypad(stdscr, TRUE);   /* 允许用户在终端使用键盘 */
    curs_set(0);    /* 设置光标是否可见，0不可见，1可见，2完全可见 */
    refresh();      /* 将虚拟屏幕上的内容写到显示器上，并刷新 */
  
    g_level = 1;
    
    /* #define INITRUNLOG()  logwin = newlogw(); 创建log窗口，就是调用自定义函数newlogw() */
    INITRUNLOG(); 
    
    /* #define RUNLOG(str)  runlog(logwin, str); runlog窗口打印一下的游戏提示 */
    RUNLOG("  Press 'q' or 'Q' to quit.");
    RUNLOG("  Press 'w/s/a/d' or 'W/S/A/D' to move the snake.");
    RUNLOG("Info:");
      
    WINDOW *gwin = newgamew(); /* 创建游戏窗口，具体由自定义函数newgamew实现 */

    mvwprintw(gwin, GAMEWIN_YLEN / 2, GAMEWIN_XLEN / 4, "%s", "hello,world!");
    wrefresh(gwin);
    
    /* getch()和getchar()不一样的 */
    getch();
     
    delwin(gwin);    /* 清除游戏窗口，并释放存储窗口数据结构的内存和信息 */
    DESTROYRUNLOG(); /* 清除信息展示窗口  */
    endwin();        /* 退出ncurses模式 */
      
    return 0;
}
  
WINDOW* newlogw()
{
    WINDOW *win = newwin(LOGWIN_YLEN, LOGWIN_XLEN, GAMEWIN_YLEN + 2, 3);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " LOG ");
    wrefresh(win);
      
    return win;
}

WINDOW* newgamew()
{
    WINDOW *win = newwin(GAMEWIN_YLEN, GAMEWIN_XLEN, 1, 3);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " GAME ");
    mvwprintw(win, GAMEWIN_YLEN-1, 2, " Level: %d ", g_level);
    mvwprintw(win, GAMEWIN_YLEN-1, 30, " Speed: %d ", (int)(g_level/3));
    wrefresh(win);
                                  
    return win;
}
   
void runlog(WINDOW *win, char *str)
{
    static char logbuf[LOGBUF_NUM][LOGBUF_LEN] = {0};
    static int  index = 0;
      
    strcpy(logbuf[index], str);
    
    int i = 0;
    
    /* #define LOGBUF_NUM  (LOGWIN_YLEN-2); LOGBUF_NUM=5 */
    for (; i < LOGBUF_NUM; ++i) {
        cleanline(win, i+1, 1); /* 自定义函数，cleanline */
        /* 每行打印字符串 */
        mvwprintw(win, i+1, 1, logbuf[(index+i) % LOGBUF_NUM]);
        wrefresh(win);
    }
    index = (index + LOGBUF_NUM - 1) % LOGBUF_NUM;
}

/* 将窗口win的坐标点(x,y)清空 */
void cleanline(WINDOW *win, int y, int x)
{
    char EMPTYLINE[LOGBUF_LEN] = {0}; // LOGBUF_LEN=57
    
    /* 将数组的0-56位置为空字符 */
    memset(EMPTYLINE, ' ', LOGBUF_LEN-1);
    
    /* 将光标移动到窗口win的(y,x)然后打印字符串EMPTYLINE */
    mvwprintw(win, y, x, EMPTYLINE);

    /* 在指定的窗口上，显示内容 */
    wrefresh(win); 
}
