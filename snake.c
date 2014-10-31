/**
 * Snake
 * author: dave_cn
 * date  : 2010/7/14
 * info  :
 *        @ ...... food
 */
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
#define GAMEWIN_YLEN     15
#define GAMEWIN_XLEN     60
#define LOGWIN_YLEN      7
#define LOGWIN_XLEN      (GAMEWIN_XLEN)
#define LOGBUF_NUM       (LOGWIN_YLEN-2)
#define LOGBUF_LEN       (GAMEWIN_XLEN-2)
#define MAXLEVEL         12
  
#define GetSnakeTail(s)  ((s)->head->front)
  
WINDOW *logwin; /* 声明一个窗口 */
#define INITRUNLOG()     logwin = newlogw()
#define RUNLOG(str)      runlog(logwin, str)
#define DESTROYRUNLOG()  delwin(logwin)
  
int g_level;
  
enum TDirection {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};
  
struct TFood {
    int y;
    int x;
};
  
struct TSnakeNode {
    int y;
    int x;
    struct TSnakeNode *front;
};
  
struct TSnake {
    int    length;
    struct TSnakeNode *head;
    enum   TDirection  dir;
};
  
int refreshgamew(WINDOW *win, struct TSnake *psnake);
void movesnake(struct TSnake *psnake);
int checksnake(struct TFood *pfood, struct TSnake *psnake);
void snakegrowup(struct TFood *pfood, struct TSnake *psnake);
void gameover(WINDOW *win, char *str);
struct TSnakeNode *newsnakenode(struct TSnakeNode **ppsnode, int y, int x);
WINDOW* newgamew();
struct TSnake* initsnake();
void destroysnake(struct TSnake *psnake);
void drawsnakew(WINDOW *win, struct TSnake *psnake);
void drawfoodw(WINDOW *win, struct TFood *pfood, struct TSnake *psnake);
TBool checkfood(struct TFood *pfood, struct TSnake *psnake);
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
    struct TSnake *psnake = initsnake(); /*  */
    drawsnakew(gwin, psnake);  /*  */
      
    /*  */  
    while (refreshgamew(gwin, psnake) >= 0)
        ;
    
    /*  */  
    getch();
     
    destroysnake(psnake); /*  */
    delwin(gwin);   /* 删除一个窗口，并释放存储窗口数据结构的内存和信息 */
    DESTROYRUNLOG(); /*  */
    endwin();       /* 退出ncurses模式 */
      
    return 0;
}
  
int refreshgamew(WINDOW *win, struct TSnake *psnake)
{
    static TBool ffood = False;
    struct TFood pfood;
      
    if (!ffood) {
        drawfoodw(win, &pfood, psnake);
        ffood = True;
    }
  
    int key = -1;
      
    fd_set set;
    FD_ZERO(&set);
    FD_SET(0, &set);
      
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec= (6 - (int)(g_level/3)) * 100*1000;
      
    if (select(1, &set, NULL, NULL, &timeout) < 0)
        return -1;
      
    if (FD_ISSET(0, &set)) {
        while ((key = getch()) == -1) ;
      
        switch (key) {
        case 'w':
        case 'W':
            (psnake->dir == DIR_DOWN) ? : (psnake->dir = DIR_UP);
            break;
          
        case 's':
        case 'S':
            (psnake->dir == DIR_UP) ? : (psnake->dir = DIR_DOWN);
            break;
          
        case 'a':
        case 'A':
            (psnake->dir == DIR_RIGHT) ? : (psnake->dir = DIR_LEFT);
            break;
          
        case 'd':
        case 'D':
            (psnake->dir == DIR_LEFT) ? : (psnake->dir = DIR_RIGHT);
            break;
          
        case 'q':
        case 'Q':
            RUNLOG("Quit Game!");
            gameover(win, "Quit Game!");
            return -1;
          
        default:
            break;
        }
    }
  
    movesnake(psnake);
    drawsnakew(win, psnake);
    switch (checksnake(&pfood, psnake)) {
    case 0:
        break;
    
    /* 吃到食物了， ffood置为0，以便重新绘制食物。
     * 同时，还要增加蛇身的长度，snakegrowup函数实现 
     * 实现手段比较容易，给队列添加一个新节点就行了
     */      
    case 1:
        ffood = False;
        if (++g_level > MAXLEVEL) {
            RUNLOG("Win!!!");
            gameover(win, "Win!!!");
            return -1;
        }
        mvwprintw(win, GAMEWIN_YLEN-1, 2, " Level: %d ", g_level);
        mvwprintw(win, GAMEWIN_YLEN-1, 30, " Speed: %d ", (int)(g_level/3));
        wrefresh(win);
        RUNLOG("Level UP!");
        snakegrowup(&pfood, psnake);
        break;
          
    default:
        RUNLOG("Game over!");
        gameover(win, "Game over!");
        return -1;
    }
      
    return 1;
}
  
/**
 * stuct TSnake是一个倒置的首尾相连的链表,head->front指向snake的尾部
 * 如: [a]<-[b]<-[c]<-[d]    a为head
 *      |              ^     snake移动的时候,只用head指向d,
 *      `--------------'     并且修改d的(y,x)为snake头移动到的位置.
 */
void movesnake(struct TSnake *psnake)
{
    int hy = psnake->head->y;
    int hx = psnake->head->x;
      
    psnake->head = GetSnakeTail(psnake);
      
    switch (psnake->dir) {
    case DIR_UP:
        psnake->head->y = hy - 1;
        psnake->head->x = hx;
        break;
      
    case DIR_DOWN:
        psnake->head->y = hy + 1;
        psnake->head->x = hx;
        break;
      
    case DIR_LEFT:
        psnake->head->y = hy;
        psnake->head->x = hx - 1;
        break;
      
    case DIR_RIGHT:
        psnake->head->y = hy;
        psnake->head->x = hx + 1;
        break;
          
    default:
        break;
    }
}
  
int checksnake(struct TFood *pfood, struct TSnake *psnake)
{
    if ( psnake->head->y <= 0 || psnake->head->y >= GAMEWIN_YLEN
      || psnake->head->x <= 0 || psnake->head->x >= GAMEWIN_XLEN)
    {
        return -1;
    }
  
    struct TSnakeNode *pnode = GetSnakeTail(psnake);
    int i = 0;
    for (; i < psnake->length - 1; ++i, pnode = pnode->front)
        if (psnake->head->y == pnode->y && psnake->head->x == pnode->x)
            return -1;
  
    if (psnake->head->y == pfood->y && psnake->head->x == pfood->x)
        return 1;
      
    return 0;
}
  
void snakegrowup(struct TFood *pfood, struct TSnake *psnake)
{
    struct TSnakeNode *pnode = (struct TSnakeNode *)malloc(sizeof(struct TSnakeNode));
      
    switch (psnake->dir) {
    case DIR_UP:
        pnode->y = psnake->head->y - 1;
        pnode->x = psnake->head->x;
        break;
      
    case DIR_DOWN:
        pnode->y = psnake->head->y + 1;
        pnode->x = psnake->head->x;
        break;
      
    case DIR_LEFT:
        pnode->y = psnake->head->y;
        pnode->x = psnake->head->x - 1;
        break;
      
    case DIR_RIGHT:
        pnode->y = psnake->head->y;
        pnode->x = psnake->head->x + 1;
        break;
          
    default:
        break;
    }
      
    pnode->front = GetSnakeTail(psnake);
    psnake->head->front = pnode;
    psnake->head = pnode;
    ++psnake->length;
}
  
void gameover(WINDOW *win, char *str)
{
    mvwprintw(win, (int)(GAMEWIN_YLEN/2), (GAMEWIN_XLEN/2 - strlen(str)/2), str);
    mvwprintw(win, (int)(GAMEWIN_YLEN/2 + 1), 20, "Press any key to quit...");
    wrefresh(win);
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
  
struct TSnake* initsnake()
{
    struct TSnake *psnake = (struct TSnake *)malloc(sizeof(struct TSnake));
  
    psnake->dir    = DIR_LEFT;
    psnake->length = 4;
      
    newsnakenode (
        &newsnakenode (
            &newsnakenode (
                &newsnakenode( &psnake->head, 4, 50 )->front, 4, 53
            )->front, 4, 52
        )->front, 4, 51
    )->front = psnake->head;
  
    return psnake;
}
  
struct TSnakeNode *newsnakenode(struct TSnakeNode **ppsnode, int y, int x)
{
    *ppsnode = (struct TSnakeNode *)malloc(sizeof(struct TSnakeNode));
    (*ppsnode)->y = y;
    (*ppsnode)->x = x;
    (*ppsnode)->front = NULL;
      
    return *ppsnode;
}
  
void destroysnake(struct TSnake *psnake)
{
    struct TSnakeNode *psnode = GetSnakeTail(psnake);
    struct TSnakeNode *ptmp   = NULL;
      
    int i = 0;
    for (; i < psnake->length; ++i) {
        ptmp   = psnode;
        psnode = psnode->front;
        free(ptmp);
    }
      
    free(psnake);
    psnake = NULL;
}
  
void drawsnakew(WINDOW *win, struct TSnake *psnake)
{
    static int taily = 0;
    static int tailx = 0;
    if (taily != 0 && tailx != 0) {
    /* mvwaddch是把光标移动到指定窗口中(win)的指定位置(taily,tailx)然后输出字符 */
        mvwaddch(win, taily, tailx, ' ');
    }
      
    /* #define GetSnakeTail(s)  ((s)->head->front) */  
    struct TSnakeNode *psnode = GetSnakeTail(psnake);
      
    int i = 0;
    for (; i < psnake->length; ++i) {
        mvwaddch(win, psnode->y, psnode->x, SHAPE_SNAKE);
        psnode = psnode->front;
    }
      
    taily = GetSnakeTail(psnake)->y;
    tailx = GetSnakeTail(psnake)->x;
  
    wrefresh(win);
}
  
void drawfoodw(WINDOW *win, struct TFood *pfood, struct TSnake *psnake)
{
    do {
        pfood->y = random() % (GAMEWIN_YLEN - 2) + 1;
        pfood->x = random() % (GAMEWIN_XLEN - 2) + 1;
    } while (False == checkfood(pfood, psnake));
    checkfood(pfood, psnake);
      
    mvwaddch(win, pfood->y, pfood->x, SHAPE_FOOD);
    wrefresh(win);
}

/* 检查food出现的位置不能在snake上 */
TBool checkfood(struct TFood *pfood, struct TSnake *psnake)
{
    struct TSnakeNode *pnode = GetSnakeTail(psnake);
  
    int i = 0;
    for (; i < psnake->length; ++i, pnode = pnode->front)
        if (pfood->y == pnode->y && pfood->x == pnode->x)
            return False;
  
    return True;
}
  
WINDOW* newlogw()
{
    WINDOW *win = newwin(LOGWIN_YLEN, LOGWIN_XLEN, GAMEWIN_YLEN + 2, 3);
    box(win, 0, 0);
    mvwprintw(win, 0, 2, " LOG ");
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
