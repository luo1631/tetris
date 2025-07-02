#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <string.h>
#include <mmsystem.h>  // 添加多媒体库头文件
#pragma comment(lib, "winmm.lib")  // 链接winmm库
#include <direct.h>    // 添加目录操作库，用于创建文件夹
#include "embedded_audio.h"

// 游戏区域定义
#define WIDTH 12
#define HEIGHT 20
#define BLOCK_SIZE 2

// UI缩放比例
#define UI_SCALE 1.0  // 取消缩放，而是通过更大的控制台来提供更好的显示

// 按钮相关定义
#define MAX_BUTTONS 10
#define BUTTON_NORMAL_COLOR COLOR_WHITE
#define BUTTON_HIGHLIGHT_COLOR COLOR_YELLOW
#define BUTTON_CLICK_COLOR COLOR_CYAN
#define BUTTON_SOUND "click"

// 鼠标事件类型 - 避免与Windows API冲突
#define MY_MOUSE_MOVED 0
#define MY_MOUSE_LEFT_PRESSED 1
#define MY_MOUSE_LEFT_RELEASED 2
#define MY_MOUSE_RIGHT_PRESSED 3
#define MY_MOUSE_RIGHT_RELEASED 4

// 按钮状态
#define BTN_NORMAL 0
#define BTN_HOVER 1
#define BTN_PRESSED 2

// 按钮定义结构体
typedef struct {
    int x, y;              // 按钮左上角坐标
    int width, height;     // 按钮宽度和高度
    char text[50];         // 按钮文本
    int state;             // 按钮状态：正常、悬停、按下
    int id;                // 按钮ID，用于识别点击的是哪个按钮
    int visible;           // 按钮是否可见
} Button;

// 全局按钮数组
Button buttons[MAX_BUTTONS];
int buttonCount = 0;

// 方块显示符号选项
const char* BLOCK_OPTIONS[] = {"■", "██", "[]", "口","▣", "▤", "▥", "▦", "▧", "▨", "▩"};
#define BLOCK_OPTION_COUNT (sizeof(BLOCK_OPTIONS)/sizeof(BLOCK_OPTIONS[0]))
int currentBlockIndex = 0;

// 颜色定义
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_PURPLE 5
#define COLOR_YELLOW 6
#define COLOR_WHITE 7

// 按键冷却时间（毫秒）
#define KEY_COOLDOWN 100
clock_t lastKeyTime = 0;

// 难度级别和速度定义
#define DIFFICULTY_EASY 1
#define DIFFICULTY_NORMAL 2
#define DIFFICULTY_HARD 3
#define DIFFICULTY_HELL 4

#define SPEED_EASY 750
#define SPEED_NORMAL 500
#define SPEED_HARD 250
#define SPEED_HELL 100

int currentDifficulty = DIFFICULTY_NORMAL; // 默认难度为普通

// 音乐和音效相关
#define SOUND_ROTATE "rotate"    // 旋转音效
#define SOUND_MOVE "move"        // 移动音效
#define SOUND_DROP "drop"        // 方块落地音效
#define SOUND_CLEAR "clear"      // 消行音效
#define SOUND_GAMEOVER "gameover"// 游戏结束音效
#define MUSIC_THEME "theme1"      // 游戏背景音乐
#define MUSIC_MENU "theme2"      // 菜单音乐
int isMusicEnabled = 1;                      // 背景音乐开关
int isSoundEnabled = 1;                      // 音效开关

// 方块形状定义
int shapes[7][4][4] = {
    // I形方块
    {
        {0,0,0,0},
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0}
    },
    // L形方块
    {
        {0,0,1,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    // J形方块
    {
        {1,0,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    // O形方块
    {
        {0,0,0,0},
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0}
    },
    // S形方块
    {
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    // Z形方块
    {
        {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    },
    // T形方块
    {
        {0,1,0,0},
        {1,1,1,0},
        {0,0,0,0},
        {0,0,0,0}
    }
};

// 游戏状态变量
int gameArea[HEIGHT][WIDTH] = {0};  // 游戏区域
int currentShape[4][4];            // 当前方块
int nextShape[4][4];               // 下一个方块
int currentX, currentY;            // 当前方块位置
int score = 0;                     // 当前分数
int highScore = 0;                 // 最高分
int gameOver = 0;                  // 游戏结束标志
int isPaused = 0;                  // 游戏暂停标志
int currentColor = COLOR_CYAN;     // 当前方块颜色
int nextColor = COLOR_YELLOW;      // 下一个方块颜色

// 双人模式变量
int gameArea2[HEIGHT][WIDTH] = {0};  // 玩家2游戏区域
int currentShape2[4][4];            // 玩家2当前方块
int nextShape2[4][4];               // 玩家2下一个方块
int currentX2, currentY2;           // 玩家2当前方块位置
int score2 = 0;                     // 玩家2当前分数
int gameOver2 = 0;                  // 玩家2游戏结束标志
int currentColor2 = COLOR_RED;      // 玩家2当前方块颜色
int nextColor2 = COLOR_GREEN;       // 玩家2下一个方块颜色
int isMultiplayerMode = 0;          // 是否为双人模式
int isAIMode = 0;                   // 是否为人机对战模式

// 单人模式起始位置参数
#define SINGLE_CENTER_X 17
#define SINGLE_CENTER_Y 5
// 双人模式起始位置参数
#define DOUBLE_CENTER_X 6
#define DOUBLE_CENTER_Y 5

// 用户信息结构体
typedef struct {
    char username[50];           // 用户名
    char password[50];           // 密码
    int highScoreEasy;           // 简单难度最高分
    int highScoreNormal;         // 普通难度最高分
    int highScoreHard;           // 困难难度最高分
    int highScoreHell;           // 地狱难度最高分
} UserInfo;

// 用于存储排行榜数据的结构
typedef struct {
    char username[50];
    int score;
} RankingEntry;

// 当前登录用户
UserInfo currentUser = {"Guest", "", 0, 0, 0, 0};
int isLoggedIn = 0;              // 是否已登录

// 函数声明
// 基础功能
void setUTF8(); // 设置控制台编码为UTF-8，确保中文正常显示
void setColor(int color); // 设置控制台文本颜色，用于显示不同颜色的方块和文字
void gotoxy(float x, float y); // 移动光标到指定位置，支持浮点数坐标
int getDisplayWidth(const char* str); // 计算UTF-8字符串的显示宽度
void drawTitleBorder(int x, int y, const char* title); // 绘制标题边框
void drawScreenBorder(int startX, int startY, int width, int height); // 绘制界面边框

// 新增界面函数声明
void showStartScreen(); // 显示主菜单界面
void gameStartScreen(); // 游戏模式选择界面
void gameSettingsScreen(); // 游戏设置界面
void accountScreen(); // 账号设置界面
void singlePlayerGameOverScreen(); // 单人游戏结束界面
void twoPlayerGameOverScreen(); // 双人游戏结束界面

// 按钮和鼠标相关函数
void initMouse(); // 初始化鼠标输入
void resetButtons(); // 重置所有按钮状态
Button createButton(int x, int y, int width, int height, const char* text, int id); // 创建一个新按钮
void drawButton(Button* button); // 绘制按钮
int isMouseInButton(int mouseX, int mouseY, Button* button); // 检测鼠标是否在按钮区域内
int handleMouseEvent(); // 处理鼠标事件并返回点击的按钮ID，如果没有点击则返回-1
void drawAllButtons(); // 绘制所有按钮
void showMousePosition(int x, int y); // 显示鼠标位置（调试用）

// 游戏界面
void difficultySelectScreen(); // 显示难度选择界面，提供简单到地狱四个难度级别
void styleChangeScreen(); // 显示方块样式设置界面，允许更改方块显示样式
void soundSettingsScreen(); // 显示音效设置界面，控制背景音乐和音效的开关

// 单人模式
void initGame(); // 初始化单人游戏，设置游戏区域和初始状态
void drawBorder(); // 绘制游戏边界，创建游戏区域的视觉边框
void drawGameArea(); // 绘制游戏区域，显示已固定的方块和当前方块
void drawNextShape(); // 显示下一个将出现的方块形状
void drawScore(); // 显示当前分数、最高分和难度等级
void drawControls(); // 显示游戏控制说明，包括按键指南
void drawPauseMessage(); // 显示游戏暂停信息
void generateNewShape(); // 生成新的方块，设置其初始位置和颜色
int canMove(int x, int y); // 检查当前方块是否可以移动到指定位置
int canMoveShape(int shape[4][4], int x, int y); // 检查指定形状是否可以移动到指定位置
void rotateShape(); // 旋转当前方块，检查旋转是否可行
void moveDown(); // 使当前方块向下移动一格
void clearLines(); // 检查并清除已填满的行，计算得分
void gameLoop(); // 单人模式主游戏循环，处理游戏逻辑和用户输入

// 双人模式
void initTwoPlayerGame(); // 初始化双人游戏，设置两个玩家的游戏区域
void drawTwoPlayerBorder(); // 绘制双人模式的游戏边界
void drawTwoPlayerGameArea(); // 绘制双人模式的游戏区域
void drawTwoPlayerNextShape(); // 显示两个玩家的下一个方块
void drawTwoPlayerScore(); // 显示两个玩家的分数
void drawTwoPlayerControls(); // 显示双人模式的控制说明
void drawTwoPlayerPauseMessage(); // 显示双人模式的暂停信息
void generateNewShape2(); // 为玩家2生成新的方块
void rotateShape2(); // 旋转玩家2的当前方块
int canMove2(int x, int y); // 检查玩家2的方块是否可以移动到指定位置
int canMoveShape2(int shape[4][4], int x, int y); // 检查玩家2的指定形状是否可以移动到指定位置
void moveDown2(); // 使玩家2的方块向下移动一格
void clearLines2(); // 检查并清除玩家2区域已填满的行
void twoPlayerGameLoop(); // 双人模式主游戏循环
void aiMakeMove(); // AI控制函数，决定AI的下一步移动

// 辅助功能
void switchBlockSymbol(); // 切换方块显示样式，在预定义样式中循环
int getSpeedByScore(); // 根据当前分数计算方块下落速度

// 音效相关
void initAudioResources(); // 初始化音频资源
void playBackgroundMusic(); // 播放游戏背景音乐
void stopBackgroundMusic(); // 停止游戏背景音乐
void playSound(const char* sound); // 播放指定的音效文件
void checkSoundFiles(); // 检查音效文件是否存在
void createSampleSoundFiles(); // 创建示例音效文件
void playMenuMusic(); // 播放菜单背景音乐
void stopMenuMusic(); // 停止菜单背景音乐

// 账号相关函数
void initUserSystem(); // 初始化用户系统，检查并创建users文件夹
void registerScreen(); // 用户注册界面
void loginScreen(); // 用户登录界面
void userRankingScreen(); // 用户排行榜界面
int checkUserExists(const char* username); // 检查用户是否存在
int createUserFile(UserInfo* user); // 创建用户文件
int readUserFile(const char* username, UserInfo* user); // 读取用户文件
void updateUserScore(); // 更新用户得分
void getInput(char* buffer, int maxLen, int isPassword); // 获取用户输入

// 排行榜相关函数
void loadRankings(RankingEntry* rankings, int* rankCount, int currentDifficultyView); // 加载排行榜数据
void drawRankings(int centerX, int centerY, int currentDifficultyView); // 绘制排行榜

// 根据分数和难度计算下落速度
int getSpeedByScore() {
    int baseSpeed;
    switch(currentDifficulty) {
        case DIFFICULTY_EASY:   baseSpeed = SPEED_EASY; break;
        case DIFFICULTY_NORMAL: baseSpeed = SPEED_NORMAL; break;
        case DIFFICULTY_HARD:   baseSpeed = SPEED_HARD; break;
        case DIFFICULTY_HELL:   baseSpeed = SPEED_HELL; break;
        default:               baseSpeed = SPEED_NORMAL;
    }
    
    double speed = 1.0 + 9.0 * (score / 10000.0); // 1倍到10倍线性增长
    return (int)(baseSpeed / speed);
}

// 设置控制台编码为UTF-8
void setUTF8() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
}

// 设置控制台颜色
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// 移动光标到指定位置，支持浮点数坐标
void gotoxy(float x, float y) {
    COORD coord;
    coord.X = (SHORT)(x * 2);  // 每个字符位置占用2个显示单位
    coord.Y = (SHORT)y;        // 行号不需要乘2
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 初始化鼠标输入
void initMouse() {
    // 获取标准输入句柄
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;
    
    // 获取当前输入模式
    GetConsoleMode(hInput, &prevMode);
    
    // 启用鼠标输入和窗口输入缓冲区
    SetConsoleMode(hInput, prevMode | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);
}

// 重置所有按钮
void resetButtons() {
    // 先清除所有按钮的显示
    for(int i = 0; i < buttonCount; i++) {
        if(buttons[i].visible) {
            int drawX = buttons[i].x - 1;
            for(int y = buttons[i].y; y < buttons[i].y + buttons[i].height; y++) {
                gotoxy(drawX, y);
                // 只清除按钮实际区域，不扩大范围
                for(int x = 0; x < buttons[i].width + 4; x++) {
                    printf(" ");
                }
            }
        }
    }
    
    // 重置按钮数组和计数
    buttonCount = 0;
    memset(buttons, 0, sizeof(buttons));
}

// 创建一个新按钮
Button createButton(int x, int y, int width, int height, const char* text, int id) {
    Button btn;
    btn.x = x;
    btn.y = y;
    
    // 计算实际需要的按钮宽度，考虑文本的显示宽度
    int textWidth = getDisplayWidth(text);
    btn.width = (width > textWidth) ? width : textWidth;  // 使用较大的宽度
    
    btn.height = height;
    strncpy(btn.text, text, sizeof(btn.text) - 1);
    btn.text[sizeof(btn.text) - 1] = '\0'; // 确保字符串以NULL结尾
    btn.state = BTN_NORMAL;
    btn.id = id;
    btn.visible = 1;
    
    // 如果还有空间，则添加到全局按钮数组
    if (buttonCount < MAX_BUTTONS) {
        buttons[buttonCount++] = btn;
    }
    
    return btn;
}

// 绘制按钮
void drawButton(Button* button) {
    if (!button->visible) return;
    
    int i, j;
    
    // 设置按钮颜色
    switch (button->state) {
        case BTN_NORMAL:
            setColor(BUTTON_NORMAL_COLOR);
            break;
        case BTN_HOVER:
            setColor(BUTTON_HIGHLIGHT_COLOR);
            break;
        case BTN_PRESSED:
            setColor(BUTTON_CLICK_COLOR);
            break;
    }
    
    // 绘制按钮边框 - 使用原始坐标，gotoxy会自动转换为显示单位
    gotoxy(button->x, button->y);
    printf("╔");
    for (i = 0; i < button->width; i++) printf("═");
    printf("╗");
    
    // 绘制按钮中间部分
    for (i = 1; i < button->height - 1; i++) {
        gotoxy(button->x, button->y + i);
        printf("║");
        for (j = 0; j < button->width; j++) printf(" ");
        printf("║");
    }
    
    // 绘制按钮底部边框
    gotoxy(button->x, button->y + button->height - 1);
    printf("╚");
    for (i = 0; i < button->width; i++) printf("═");
    printf("╝");
    
    // 居中显示文本
    int textLen = getDisplayWidth(button->text);
    int textX = button->x + (button->width - textLen) / 2;
    int textY = button->y + button->height / 2;
    gotoxy(textX, textY);
    printf("%s", button->text);
}

// 检测鼠标是否在按钮区域内
int isMouseInButton(int mouseX, int mouseY, Button* button) {
    if (!button->visible) return 0;
    
    // 由于mouseX已经是显示单位（每个字符占2个单位），这里不需要再乘2
    int buttonStartX = button->x * 2;  // 转换为显示单位
    int buttonEndX = (button->x + button->width + 2) * 2;  // +2是因为左右边框
    int buttonStartY = button->y;
    int buttonEndY = button->y + button->height;
    
    return (mouseX >= buttonStartX && mouseX < buttonEndX &&
            mouseY >= buttonStartY && mouseY < buttonEndY);
}

// 显示鼠标位置（调试用）
void showMousePosition(int x, int y) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    
    // 保存原来的颜色
    WORD originalAttrs = csbi.wAttributes;
    
    // 设置调试信息颜色
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
    
    // 显示鼠标坐标
    gotoxy(0, 0);
    printf("鼠标位置: X=%d, Y=%d   ", x, y);
    
    // 恢复原来的颜色
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), originalAttrs);
}

// 处理鼠标事件
int handleMouseEvent() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD inputBuffer;
    DWORD numEvents;
    static int lastButtonPressed = -1;  // 记录上一次按下的按钮ID
    
    // 检查是否有输入事件
    PeekConsoleInput(hInput, &inputBuffer, 1, &numEvents);
    if (numEvents == 0) return -1;
    
    // 读取事件
    ReadConsoleInput(hInput, &inputBuffer, 1, &numEvents);
    
    // 处理鼠标事件
    if (inputBuffer.EventType == MOUSE_EVENT) {
        MOUSE_EVENT_RECORD mouseEvent = inputBuffer.Event.MouseEvent;
        int mouseX = mouseEvent.dwMousePosition.X;  // 已经是显示单位
        int mouseY = mouseEvent.dwMousePosition.Y;
        
        // 检查每个按钮
        for (int i = 0; i < buttonCount; i++) {
            // 检查鼠标是否在按钮区域内
            int isInButton = isMouseInButton(mouseX, mouseY, &buttons[i]);
            
            // 根据鼠标状态更新按钮状态
            if (isInButton) {
                // 检查左键点击
                if (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
                    // 按钮被按下
                    buttons[i].state = BTN_PRESSED;
                    drawButton(&buttons[i]);
                    lastButtonPressed = buttons[i].id;
                    return -1;  // 等待释放
                } 
                else if (mouseEvent.dwEventFlags & MOUSE_MOVED) {
                    // 鼠标移动到按钮上
                    if (buttons[i].state != BTN_HOVER && buttons[i].state != BTN_PRESSED) {
                        buttons[i].state = BTN_HOVER;
                        drawButton(&buttons[i]);
                    }
                }
                else if (!(mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && 
                         lastButtonPressed == buttons[i].id) {
                    // 鼠标释放，且之前按下过这个按钮
                    buttons[i].state = BTN_HOVER;
                    drawButton(&buttons[i]);
                    if (isSoundEnabled) {
                        playSound(SOUND_MOVE);  // 使用移动音效作为点击音效
                    }
                    int clickedId = buttons[i].id;
                    lastButtonPressed = -1;
                    return clickedId;  // 返回点击的按钮ID
                }
            } else if (buttons[i].state != BTN_NORMAL) {
                // 如果鼠标不在按钮上，恢复正常状态
                buttons[i].state = BTN_NORMAL;
                drawButton(&buttons[i]);
            }
        }
        
        // 如果鼠标释放但不在任何按钮上
        if (!(mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) && lastButtonPressed != -1) {
            lastButtonPressed = -1;
        }
    }
    
    return -1; // 没有点击任何按钮
}

// 绘制所有按钮
void drawAllButtons() {
    for (int i = 0; i < buttonCount; i++) {
        if (buttons[i].visible) {
            drawButton(&buttons[i]);
        }
    }
}

// 切换方块符号
void switchBlockSymbol() {
    currentBlockIndex = (currentBlockIndex + 1) % BLOCK_OPTION_COUNT;
}

// 计算UTF-8字符串的显示宽度
int getDisplayWidth(const char* str) {
    int width = 0;
    int i = 0;
    while (str[i] != '\0') {
        if ((str[i] & 0x80) == 0) {
            // ASCII字符
            width += 1;
            i += 1;
        } else if ((str[i] & 0xE0) == 0xC0) {
            // 2字节UTF-8
            width += 2;
            i += 2;
        } else if ((str[i] & 0xF0) == 0xE0) {
            // 3字节UTF-8（中文）
            width += 2;
            i += 3;
        } else if ((str[i] & 0xF8) == 0xF0) {
            // 4字节UTF-8
            width += 2;
            i += 4;
        } else {
            // 无效UTF-8，跳过
            i += 1;
        }
    }
    return width;
}

// 绘制标题边框
void drawTitleBorder(int x, int y, const char* title) {
    float displayWidth = getDisplayWidth(title)/2.0f;
    
    // 设置边框颜色
    setColor(COLOR_CYAN);
    
    // 绘制上下边框，考虑到每个边框字符占用2个显示单位
    gotoxy(x - 1, y - 1);
    for(int i = 0; i < displayWidth*2 + 3; i++) {
        printf("═");
    }
    gotoxy(x - 1, y + 1);
    for(int i = 0; i < displayWidth*2 + 3; i++) {
        printf("═");
    }
    
    // 绘制左侧边框
    gotoxy(x - 1, y - 1);
    printf("╔");
    gotoxy(x - 1, y);
    printf("║");
    gotoxy(x - 1, y + 1);
    printf("╚");
    
    // 绘制右侧边框 - 根据实际显示宽度计算位置
    gotoxy(x + displayWidth + 0.5f, y - 1);  // 使用0.5个单位的偏移
    printf("╗");
    gotoxy(x + displayWidth + 0.5f, y);      // 使用0.5个单位的偏移
    printf("║");
    gotoxy(x + displayWidth + 0.5f, y + 1);  // 使用0.5个单位的偏移
    printf("╝");
    
    // 绘制标题文本
    gotoxy(x, y);
    setColor(COLOR_CYAN);
    printf("%s", title);
}

// 绘制界面边框
void drawScreenBorder(int startX, int startY, int width, int height) {
    setColor(COLOR_WHITE);

    // 绘制左上角
    gotoxy(startX, startY);
    printf("╔");
    
    // 绘制上边框
    for (int i = 0; i < width-1; i++) {
        printf("═");
    }
    
    // 绘制右上角
    printf("╗");
    
    // 绘制左右边框
    for (int i = 1; i < height; i++) {
        gotoxy(startX, startY + i);
        printf("║");
        
        gotoxy(startX + width/2, startY + i);
        printf("║");
    }
    
    // 绘制左下角
    gotoxy(startX, startY + height);
    printf("╚");
    
    // 绘制下边框
    for (int i = 0; i < width-1; i++) {
        printf("═");
    }
    
    // 绘制右下角
    printf("╝");

}

// 显示开始界面
void showStartScreen() {
    system("cls");  // 先清屏
    resetButtons();  // 然后重置按钮
    stopBackgroundMusic();
    playMenuMusic();
    
    // 计算居中的横坐标
    int centerX = 25;  // 适应100列的窗口
    
    int running = 1;
    int needRedraw = 1;  // 初始需要绘制
    
    while (running) {
        if (needRedraw) {
            system("cls");  // 在重绘之前清屏
            resetButtons();  // 重置按钮状态
            
            // 绘制整个界面的边框，调整参数
            drawScreenBorder(8, 0, 74, 30);
            
            // 创建主菜单按钮，减小垂直间距
            createButton(centerX-1, 8, 10, 3, "开始游戏", 1);
            createButton(centerX-1, 12, 10, 3, "游戏设置", 2);
            createButton(centerX-1, 16, 10, 3, "账号设置", 3);
            createButton(centerX-1, 20, 10, 3, "退出游戏", 4);
            
            // 绘制标题及边框
            drawTitleBorder(centerX - 2, 3, "俄罗斯方块小游戏");
            
            // 绘制所有按钮
            drawAllButtons();
            
            // 绘制提示文本
            setColor(COLOR_GREEN);
            gotoxy(centerX - 7, 25);
            printf("使用鼠标点击按钮，或按1-4数字键选择");
            
            needRedraw = 0;  // 重置重绘标志
        }
        
        // 处理键盘输入
        if(kbhit()) {
            int key = getch();
            if (key == '1') {
                gameStartScreen();
                needRedraw = 1;
            } else if (key == '2') {
                gameSettingsScreen();
                needRedraw = 1;
            } else if (key == '3') {
                accountScreen();
                needRedraw = 1;
            } else if (key == '4') {
                stopMenuMusic();
                exit(0);
            }
        }
        
        // 处理鼠标事件
        int btnId = handleMouseEvent();
        if (btnId > 0) {
            switch (btnId) {
                case 1: // 开始游戏
                    gameStartScreen();
                    needRedraw = 1;
                    break;
                case 2: // 游戏设置
                    gameSettingsScreen();
                    needRedraw = 1;
                    break;
                case 3: // 账号设置
                    accountScreen();
                    needRedraw = 1;
                    break;
                case 4: // 退出游戏
                    stopMenuMusic();
                    exit(0);
                    break;
            }
        }
        
        // 短暂延时，防止CPU占用过高
        Sleep(50);
    }
}

// 游戏模式选择界面
void gameStartScreen() {
    system("cls");
    resetButtons();
    
    int running = 1;
    int needRedraw = 1;
    int centerX = 25;
    
    while (running) {
        if (needRedraw) {
            system("cls");
            resetButtons();
            
            // 绘制整个界面的边框，调整参数
            drawScreenBorder(8, 0, 74, 30);
            
            // 创建游戏模式选择按钮
            createButton(centerX-1, 8, 10, 3, "单人游戏", 1);
            createButton(centerX-1, 12, 10, 3, "双人游戏", 2);
            createButton(centerX-1, 16, 10, 3, "人机对战", 3);
            createButton(centerX-1, 20, 10, 3, "返回菜单", 4);
            
            // 绘制标题及边框
            drawTitleBorder(centerX-1, 3, "游戏模式选择");
            
            // 绘制所有按钮
            drawAllButtons();
            
            // 绘制提示文本
            setColor(COLOR_GREEN);
            gotoxy(centerX - 7, 25);
            printf("使用鼠标点击按钮，或按1-4数字键选择");
            
            needRedraw = 0;
        }
        
        if(kbhit()) {
            int key = getch();
            if (key == '1') {
                isMultiplayerMode = 0;
                isAIMode = 0;
                stopMenuMusic();
                initGame();
                gameLoop();
                running = 0;
                return;
            } else if (key == '2') {
                isMultiplayerMode = 1;
                isAIMode = 0;
                stopMenuMusic();
                initTwoPlayerGame();
                twoPlayerGameLoop();
                running = 0;
                return;
            } else if (key == '3') {
                isMultiplayerMode = 1;
                isAIMode = 1;
                stopMenuMusic();
                initTwoPlayerGame();
                twoPlayerGameLoop(); // 后续需要实现AI模式的游戏循环
                running = 0;
                return;
            } else if (key == '4') {
                running = 0;
                return;
            }
        }
        
        int btnId = handleMouseEvent();
        if (btnId > 0) {
            switch (btnId) {
                case 1: // 单人游戏
                    isMultiplayerMode = 0;
                    isAIMode = 0;
                    stopMenuMusic();
                    initGame();
                    gameLoop();
                    running = 0;
                    return;
                case 2: // 双人游戏
                    isMultiplayerMode = 1;
                    isAIMode = 0;
                    stopMenuMusic();
                    initTwoPlayerGame();
                    twoPlayerGameLoop();
                    running = 0;
                    return;
                case 3: // 人机对战
                    isMultiplayerMode = 1;
                    isAIMode = 1;
                    stopMenuMusic();
                    initTwoPlayerGame();
                    twoPlayerGameLoop(); // 后续需要实现AI模式的游戏循环
                    running = 0;
                    return;
                case 4: // 返回菜单
                    running = 0;
                    return;
            }
        }
        
        Sleep(50);
    }
}

// 游戏设置界面
void gameSettingsScreen() {
    system("cls");
    resetButtons();
    
    int running = 1;
    int needRedraw = 1;
    int centerX = 25;
    
    while (running) {
        if (needRedraw) {
            system("cls");
            resetButtons();
            
            // 绘制整个界面的边框，调整参数
            drawScreenBorder(8, 0, 74, 30);
            
            // 绘制标题及边框
            drawTitleBorder(centerX, 3, "游戏设置");
            
            // 显示当前难度
            setColor(COLOR_WHITE);
            gotoxy(centerX - 2, 6);
            printf("当前难度:  ");
            switch(currentDifficulty) {
                case DIFFICULTY_EASY:   setColor(COLOR_GREEN);  printf("简单"); break;
                case DIFFICULTY_NORMAL: setColor(COLOR_YELLOW); printf("普通"); break;
                case DIFFICULTY_HARD:   setColor(COLOR_PURPLE); printf("困难"); break;
                case DIFFICULTY_HELL:   setColor(COLOR_RED);    printf("地狱"); break;
            }
            
            // 创建游戏设置按钮
            createButton(centerX-1, 8, 10, 3, "难度设置", 1);
            createButton(centerX-1, 12, 10, 3, "样式设置", 2);
            createButton(centerX-1, 16, 10, 3, "音效设置", 3);
            createButton(centerX-1, 20, 10, 3, "返回设置", 4);
            
            // 绘制所有按钮
            drawAllButtons();
            
            // 绘制提示文本
            setColor(COLOR_GREEN);
            gotoxy(centerX - 7, 25);
            printf("使用鼠标点击按钮，或按1-4数字键选择");
            
            needRedraw = 0;  // 重置重绘标志
        }
        
        if(kbhit()) {
            int key = getch();
            if (key == '1') {
                difficultySelectScreen();
                needRedraw = 1;
            } else if (key == '2') {
                styleChangeScreen();
                needRedraw = 1;
            } else if (key == '3') {
                soundSettingsScreen();
                needRedraw = 1;
            } else if (key == '4') {
                running = 0;
            }
        }
        
        int btnId = handleMouseEvent();
        if (btnId > 0) {
            switch (btnId) {
                case 1: // 难度设置
                    difficultySelectScreen();
                    needRedraw = 1;
                    break;
                case 2: // 样式设置
                    styleChangeScreen();
                    needRedraw = 1;
                    break;
                case 3: // 音效设置
                    soundSettingsScreen();
                    needRedraw = 1;
                    break;
                case 4: // 返回设置
                    running = 0;
                    break;
            }
        }
        
        Sleep(50);
    }
}

// 账号设置界面
void accountScreen() {
    system("cls");
    resetButtons();
    
    int running = 1;
    int needRedraw = 1;
    int centerX = 25;
    
    // 初始化用户系统
    initUserSystem();
    
    while (running) {
        if (needRedraw) {
            system("cls");
            resetButtons();
            
            // 绘制整个界面的边框，调整参数
            drawScreenBorder(8, 0, 74, 30);
            
            // 创建账号设置按钮
            createButton(centerX-1, 8, 10, 3, "账号注册", 1);
            createButton(centerX-1, 12, 10, 3, "账号登录", 2);
            createButton(centerX-1, 16, 10, 3, "游戏排行", 3);
            createButton(centerX-1, 20, 10, 3, "返回菜单", 4);
            
            // 绘制标题及边框
            drawTitleBorder(centerX , 3, "账号设置");
            
            // 绘制所有按钮
            drawAllButtons();
            
            // 绘制提示文本
            setColor(COLOR_GREEN);
            gotoxy(centerX - 7, 25);
            printf("使用鼠标点击按钮，或按1-4数字键选择");
            
            // 显示当前登录状态
            
            if (isLoggedIn) {
                gotoxy(centerX -5, 27);
                printf("当前登录账号: %s", currentUser.username);
            } else {
                gotoxy(centerX -1, 27);
                printf("当前未登录");
            }
            
            needRedraw = 0;
        }
        
        if(kbhit()) {
            int key = getch();
            switch(key) {
                case '1':
                    registerScreen();
                    needRedraw = 1;
                    break;
                case '2':
                    loginScreen();
                    needRedraw = 1;
                    break;
                case '3':
                    userRankingScreen();
                    needRedraw = 1;
                    break;
                case '4':
                case 27: // ESC键
                    running = 0;
                    break;
            }
        }
        
        int btnId = handleMouseEvent();
        if (btnId > 0) {
            switch (btnId) {
                case 1: // 账号注册
                    registerScreen();
                    needRedraw = 1;
                    break;
                case 2: // 账号登录
                    loginScreen();
                    needRedraw = 1;
                    break;
                case 3: // 游戏排行
                    userRankingScreen();
                    needRedraw = 1;
                    break;
                case 4: // 返回菜单
                    running = 0;
                    break;
            }
        }
        
        Sleep(50);
    }
}

// 初始化游戏
void initGame() {
    system("cls");
    srand(time(NULL));
    score = 0;
    gameOver = 0;
    isPaused = 0;
    memset(gameArea, 0, sizeof(gameArea));
    
    int shapeIndex = rand() % 7;
    memcpy(nextShape, shapes[shapeIndex], sizeof(nextShape));
    generateNewShape();
    
    stopMenuMusic();
    playBackgroundMusic();
}

// 检查是否可以移动
int canMove(int x, int y) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(currentShape[i][j]) {
                int newX = x + j;
                int newY = y + i;
                if(newX < 0 || newX >= WIDTH || newY >= HEIGHT) return 0;
                if(newY >= 0 && gameArea[newY][newX]) return 0;
            }
        }
    }
    return 1;
}

// 检查任意形状在指定位置是否可以移动
int canMoveShape(int shape[4][4], int x, int y) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(shape[i][j]) {
                int newX = x + j;
                int newY = y + i;
                if(newX < 0 || newX >= WIDTH || newY >= HEIGHT) return 0;
                if(newY >= 0 && gameArea[newY][newX]) return 0;
            }
        }
    }
    return 1;
}

// 生成新方块
void generateNewShape() {
    memcpy(currentShape, nextShape, sizeof(currentShape));
    currentX = WIDTH/2 - 2;
    currentY = 0;
    currentColor = nextColor;
    
    int shapeIndex = rand() % 7;
    memcpy(nextShape, shapes[shapeIndex], sizeof(nextShape));
    nextColor = 1 + rand() % 7; // 1~7，避免黑色
}

// 旋转方块
void rotateShape() {
    int temp[4][4];
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            temp[i][j] = currentShape[3-j][i];
    
    if(canMoveShape(temp, currentX, currentY)) {
        memcpy(currentShape, temp, sizeof(currentShape));
        playSound(SOUND_ROTATE);
    }
}

// 绘制边框
void drawBorder() {
    int centerX = SINGLE_CENTER_X;
    int centerY = SINGLE_CENTER_Y;
    setColor(COLOR_WHITE);
    // 上边框
    gotoxy(centerX, centerY); printf("╔");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("═");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("╗");
    gotoxy(centerX+WIDTH+1, centerY); printf("╦");

    // 侧边框
    for(int i = 1; i <= HEIGHT; i++) {
        gotoxy(centerX, centerY+i); printf("║"); // 左侧边界
        gotoxy(centerX+WIDTH+1, centerY+i); printf("║"); // 游戏区中线
        gotoxy(centerX+WIDTH+1+WIDTH, centerY+i); printf("║"); // 信息区右边界
    }

    // 下边框
    gotoxy(centerX, centerY+HEIGHT+1); printf("╚");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("═");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("╝");
    gotoxy(centerX+WIDTH+1, centerY+HEIGHT+1); printf("╩");
}

// 绘制游戏区域
void drawGameArea() {
    int centerX = SINGLE_CENTER_X;
    int centerY = SINGLE_CENTER_Y;
    for(int i = 0; i < HEIGHT; i++) {
        for(int j = 0; j < WIDTH; j++) {
            gotoxy(centerX+j+1, centerY+i+1);
            if(gameArea[i][j]) {
                setColor(gameArea[i][j]);
                printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            } else {
                printf("  ");
            }
        }
    }
}

// 绘制下一个方块
void drawNextShape() {
    int centerX = SINGLE_CENTER_X;
    int centerY = SINGLE_CENTER_Y;
    setColor(rand()%6+1);
    gotoxy(centerX+WIDTH+5, centerY+2);
    printf("下一个方块:");
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            gotoxy(centerX+WIDTH+5+j, centerY+i+4);
            if(nextShape[i][j]) {
                setColor(nextColor);
                printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            } else {
                printf("  ");
            }
        }
    }
}

// 绘制分数
void drawScore() {
    int centerX = SINGLE_CENTER_X;
    int centerY = SINGLE_CENTER_Y;
    setColor(COLOR_WHITE);
    gotoxy(centerX+WIDTH+3, centerY+8);
    printf("当前分数: %d", score);
    gotoxy(centerX+WIDTH+3, centerY+9);
    printf("最高分数: %d", highScore);
    gotoxy(centerX+WIDTH+3, centerY+10);
    printf("当前难度: ");
    switch(currentDifficulty) {
        case DIFFICULTY_EASY:
            setColor(COLOR_GREEN);
            printf("简单");
            break;
        case DIFFICULTY_NORMAL:
            setColor(COLOR_YELLOW);
            printf("普通");
            break;
        case DIFFICULTY_HARD:
            setColor(COLOR_PURPLE);
            printf("困难");
            break;
        case DIFFICULTY_HELL:
            setColor(COLOR_RED);
            printf("地狱");
            break;
    }
}

// 绘制控制说明
void drawControls() {
    int centerX = SINGLE_CENTER_X;
    int centerY = SINGLE_CENTER_Y;
    setColor(COLOR_WHITE);
    gotoxy(centerX+WIDTH+3, centerY+12);
    printf("控制说明:");
    gotoxy(centerX+WIDTH+3, centerY+13);
    printf("←→: 左右移动");
    gotoxy(centerX+WIDTH+3, centerY+14);
    printf("↑: 旋转");
    gotoxy(centerX+WIDTH+3, centerY+15);
    printf("空格: 快速下落");
    gotoxy(centerX+WIDTH+3, centerY+16);
    printf("Z: 暂停");
    gotoxy(centerX+WIDTH+3, centerY+17);
    printf("Q: 退出");
    gotoxy(centerX+WIDTH+3, centerY+19);
    printf("注意: 按键有冷却");
    gotoxy(centerX+WIDTH+3, centerY+20);
    printf("建议单击按键");
}

// 绘制暂停提示
void drawPauseMessage() {
    int centerX = SINGLE_CENTER_X;
    int centerY = SINGLE_CENTER_Y;
    if(isPaused) {
        setColor(COLOR_YELLOW);
        gotoxy(centerX+WIDTH/2-2, centerY+HEIGHT/2-4);
        printf("游戏暂停");
        
        // 创建暂停菜单按钮，减小间距和宽度
        resetButtons();
        createButton(centerX+WIDTH/2-3, centerY+HEIGHT/2-2, 10, 3, "继续游戏", 1);
        createButton(centerX+WIDTH/2-3, centerY+HEIGHT/2+2, 10, 3, "返回菜单", 2);
        
        // 绘制按钮
        drawAllButtons();
        
        // 重新绘制边框，修复被覆盖的问题
        drawBorder();
    }
}

// 方块下落
void moveDown() {
    if(canMove(currentX, currentY + 1)) {
        currentY++;
    } else {
        // 固定当前方块
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                if(currentShape[i][j] && currentY + i >= 0) {
                    gameArea[currentY + i][currentX + j] = currentColor;
                }
            }
        }
        playSound(SOUND_DROP);
        clearLines();
        generateNewShape();
        if(!canMove(currentX, currentY)) {
            gameOver = 1;
            playSound(SOUND_GAMEOVER);
            stopBackgroundMusic();
        }
    }
}

// 清除已满行
void clearLines() {
    int linesCleared = 0;
    for(int i = HEIGHT-1; i >= 0; i--) {
        int isFull = 1;
        for(int j = 0; j < WIDTH; j++) {
            if(!gameArea[i][j]) {
                isFull = 0;
                break;
            }
        }
        if(isFull) {
            linesCleared++;
            // 从当前行向下移动所有行
            for(int k = i; k > 0; k--) {
                for(int j = 0; j < WIDTH; j++) {
                    gameArea[k][j] = gameArea[k-1][j];
                }
            }
            // 清空最底行
            for(int j = 0; j < WIDTH; j++) {
                gameArea[0][j] = 0;
            }
            i++; // 重新检查当前行(已被上一行替换)
        }
    }
    
    if(linesCleared > 0) {
        playSound(SOUND_CLEAR);
        score += linesCleared * 10;
        if(score > highScore) {
            highScore = score;
        }
    }
}

// 游戏主循环
void gameLoop() {
    clock_t lastMove = clock();
    int prevX = currentX, prevY = currentY;
    int needRedraw = 1; // 是否需要重绘
    int centerX = SINGLE_CENTER_X;
    int centerY = SINGLE_CENTER_Y;
    
    while(!gameOver) {
        // 达到胜利条件
        if(score >= 10000) {
            gameOver = 1;
            stopBackgroundMusic();
            system("cls");
            gotoxy(15, 10); printf("你赢了！最终得分：%d", score);
            gotoxy(15, 11); printf("最高分纪录：%d", highScore);
            gotoxy(15, 13); printf("按任意键退出...");
            getch();
            break;
        }
        
        needRedraw = 0; // 默认不需要重绘
        
        // 处理暂停菜单的鼠标点击和q键
        if(isPaused) {
            // 先处理键盘输入，确保按键响应
            if (kbhit()) {
                int key = getch();
                if (key == 'z' || key == 'Z' || key == '1') {
                    isPaused = 0;
                    needRedraw = 1;
                    playBackgroundMusic();
                } else if (key == 'q' || key == 'Q' || key == '2') {
                    gameOver = 1;
                    stopBackgroundMusic();
                    singlePlayerGameOverScreen(); // 显示游戏结束界面
                    return;
                }
                continue;  // 处理完按键后直接进入下一循环
            }
            
            // 再处理鼠标事件
            int btnId = handleMouseEvent();
            if (btnId > 0) {
                switch (btnId) {
                    case 1: // 继续游戏
                        isPaused = 0;
                        needRedraw = 1;
                        playBackgroundMusic(); // 继续时播放背景音乐
                        break;
                    case 2: // 返回主菜单
                        gameOver = 1;
                        stopBackgroundMusic();
                        break;
                }
            }
            
            // 短暂延时，减少CPU占用，但不要太长以免影响按键响应
            Sleep(10);
            continue;
        }
        
        // 处理键盘输入
        if(kbhit()) {
            // 按键冷却检查
            clock_t currentTime = clock();
            if (currentTime - lastKeyTime < KEY_COOLDOWN) {
                // 如果冷却时间未到，忽略按键
                getch(); // 清除按键缓冲
                continue;
            }
            
            lastKeyTime = currentTime; // 更新上次按键时间
            int key = getch();
            
            // 处理方向键
            if(key == 224) {  // 方向键的第一个字符
                key = getch();  // 获取方向键的第二个字符
                if(!isPaused) {  // 只有在非暂停状态才处理方向键
                    switch(key) {
                        case 72: // 上箭头：旋转
                            needRedraw = 1;
                            rotateShape();
                            break;
                        case 80: // 下箭头：下移
                            if(canMove(currentX, currentY + 1)) {
                                needRedraw = 1;
                                currentY++;
                            }
                            break;
                        case 75: // 左箭头：左移
                            if(canMove(currentX-1, currentY)) {
                                needRedraw = 1;
                                currentX--;
                                playSound(SOUND_MOVE);
                            }
                            break;
                        case 77: // 右箭头：右移
                            if(canMove(currentX+1, currentY)) {
                                needRedraw = 1;
                                currentX++;
                                playSound(SOUND_MOVE);
                            }
                            break;
                    }
                }
            } else {
                // 处理其他按键
                switch(key) {
                    case 'a': case 'A': // 左移
                        if(!isPaused && canMove(currentX-1, currentY)) {
                            needRedraw = 1;
                            currentX--;
                            playSound(SOUND_MOVE); // 播放移动音效
                        }
                        break;
                    case 'd': case 'D': // 右移
                        if(!isPaused && canMove(currentX+1, currentY)) {
                            needRedraw = 1;
                            currentX++;
                            playSound(SOUND_MOVE); // 播放移动音效
                        }
                        break;
                    case 'w': case 'W': // 旋转
                        if(!isPaused) {
                            needRedraw = 1;
                            rotateShape();
                        }
                        break;
                    case 's': case 'S': // 下移
                        if(!isPaused && canMove(currentX, currentY + 1)) {
                            needRedraw = 1;
                            currentY++;
                        }
                        break;
                    case ' ': // 快速下落
                        if(!isPaused) {
                            needRedraw = 1;
                            while(canMove(currentX, currentY+1)) currentY++;
                        }
                        break;
                    case 'z': case 'Z': // 暂停/继续
                        needRedraw = 1;
                        isPaused = !isPaused;
                        if(isPaused) {
                            stopBackgroundMusic(); // 暂停时停止背景音乐
                        } else {
                            playBackgroundMusic(); // 继续时播放背景音乐
                        }
                        break;
                    case 'q': case 'Q': // 退出
                        gameOver = 1;
                        stopBackgroundMusic();
                        singlePlayerGameOverScreen(); // 显示游戏结束界面
                        return;
                    case 'm': case 'M': // 音乐开关
                        isMusicEnabled = !isMusicEnabled;
                        if(isMusicEnabled) {
                            playBackgroundMusic();
                        } else {
                            stopBackgroundMusic();
                        }
                        break;
                }
            }
        }
        
        // 自动下落
        if(!isPaused && clock() - lastMove > getSpeedByScore()) {
            needRedraw = 1;
            moveDown();
            lastMove = clock();
        }
        
        // 渲染游戏画面
        if(needRedraw || prevX != currentX || prevY != currentY) {
            // 擦除旧位置
            if(prevX != currentX || prevY != currentY) {
                setColor(COLOR_BLACK);
                for(int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        if(currentShape[i][j]) {
                            gotoxy(centerX+prevX+j+1, centerY+prevY+i+1);
                            printf("  ");
                        }
                    }
                }
            }
            
            // 绘制游戏界面
            drawBorder();
            drawGameArea();
            drawNextShape();
            drawScore();
            drawControls();
            
            // 绘制当前方块
            if(!isPaused) {
                setColor(currentColor);
                for(int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        if(currentShape[i][j]) {
                            gotoxy(centerX+currentX+j+1, centerY+currentY+i+1);
                            printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
                        }
                    }
                }
            }
            
            // 更新上一次位置
            prevX = currentX;
            prevY = currentY;
        }
        
        // 显示暂停信息
        drawPauseMessage();
        
        Sleep(75);
    }
}

// 初始化双人游戏
void initTwoPlayerGame() {
    system("cls");
    srand(time(NULL));
    score = 0;
    score2 = 0;
    gameOver = 0;
    gameOver2 = 0;
    isPaused = 0;
    
    // 清空两个游戏区域
    memset(gameArea, 0, sizeof(gameArea));
    memset(gameArea2, 0, sizeof(gameArea2));
    
    // 初始化两位玩家的方块
    int shapeIndex = rand() % 7;
    memcpy(nextShape, shapes[shapeIndex], sizeof(nextShape));
    generateNewShape();
    
    shapeIndex = rand() % 7;
    memcpy(nextShape2, shapes[shapeIndex], sizeof(nextShape2));
    generateNewShape2();
    
    // 停止菜单音乐，播放游戏背景音乐
    stopMenuMusic();
    playBackgroundMusic();
}

// 绘制双人模式边框
void drawTwoPlayerBorder() {
    int centerX = DOUBLE_CENTER_X;
    int centerY = DOUBLE_CENTER_Y;
    setColor(COLOR_WHITE);
    // 玩家1区域
    gotoxy(centerX, centerY); printf("╔");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("═");
    printf("╗");
    // 侧边框
    for(int i = 1; i <= HEIGHT; i++) {
        gotoxy(centerX, centerY+i); printf("║");
        gotoxy(centerX+WIDTH+1, centerY+i); printf("║");
    }
    gotoxy(centerX, centerY+HEIGHT+1); printf("╚");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("═");
    printf("╝");
    // 玩家2区域
    gotoxy(centerX+WIDTH+3, centerY); printf("╔");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("═");
    printf("╗");
    for(int i = 1; i <= HEIGHT; i++) {
        gotoxy(centerX+WIDTH+3, centerY+i); printf("║");
        gotoxy(centerX+WIDTH*2+4, centerY+i); printf("║");
    }
    gotoxy(centerX+WIDTH+3, centerY+HEIGHT+1); printf("╚");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("═");
    printf("╝");
    // 信息区域
    gotoxy(centerX+WIDTH*2+6, centerY); printf("╔");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("═");
    printf("╗");
    for(int i = 1; i <= HEIGHT; i++) {
        gotoxy(centerX+WIDTH*2+6, centerY+i); printf("║");
        gotoxy(centerX+WIDTH*3+7, centerY+i); printf("║");
    }
    gotoxy(centerX+WIDTH*2+6, centerY+HEIGHT+1); printf("╚");
    for(int i = 0; i < WIDTH; i++) printf("══");
    printf("═");
    printf("╝");
}

// 绘制双人模式游戏区域
void drawTwoPlayerGameArea() {
    int centerX = DOUBLE_CENTER_X;
    int centerY = DOUBLE_CENTER_Y;
    // 玩家1
    for(int i = 0; i < HEIGHT; i++) {
        for(int j = 0; j < WIDTH; j++) {
            gotoxy(centerX+j+1, centerY+i+1);
            if(gameArea[i][j]) {
                setColor(gameArea[i][j]);
                printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            } else {
                printf("  ");
            }
        }
    }
    // 玩家2
    for(int i = 0; i < HEIGHT; i++) {
        for(int j = 0; j < WIDTH; j++) {
            gotoxy(centerX+WIDTH+4+j, centerY+i+1);
            if(gameArea2[i][j]) {
                setColor(gameArea2[i][j]);
                printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            } else {
                printf("  ");
            }
        }
    }
}

// 绘制双人模式下一个方块
void drawTwoPlayerNextShape() {
    int centerX = DOUBLE_CENTER_X;
    int centerY = DOUBLE_CENTER_Y;
    // 玩家1下一个方块
    setColor(COLOR_CYAN);
    gotoxy(centerX+WIDTH*2+10, centerY+2);
    printf("玩家1下一个:");
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            gotoxy(centerX+WIDTH*2+10+j, centerY+i+3);
            if(nextShape[i][j]) {
                setColor(nextColor);
                printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            } else {
                printf("  ");
            }
        }
    }
    // 玩家/电脑下一个方块
    setColor(COLOR_RED);
    gotoxy(centerX+WIDTH*2+10, centerY+6);
    if (isAIMode) {
        printf("电脑下一个:");
    } else {
        printf("玩家下一个:");
    }
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            gotoxy(centerX+WIDTH*2+10+j, centerY+i+7);
            if(nextShape2[i][j]) {
                setColor(nextColor2);
                printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            } else {
                printf("  ");
            }
        }
    }
}

// 绘制双人模式分数
void drawTwoPlayerScore() {
    int centerX = DOUBLE_CENTER_X;
    int centerY = DOUBLE_CENTER_Y;
    setColor(COLOR_WHITE);
    gotoxy(centerX+WIDTH*2+8, centerY+11);
    
    if (isAIMode) {
        printf("玩家分数: %d", score);
        gotoxy(centerX+WIDTH*2+8, centerY+12);
        printf("电脑分数: %d", score2);
    } else {
        printf("玩家1分数: %d", score);
        gotoxy(centerX+WIDTH*2+8, centerY+12);
        printf("玩家2分数: %d", score2);
    }
    
    gotoxy(centerX+WIDTH*2+8, centerY+13);
    printf("最高分数: %d", highScore);
    gotoxy(centerX+WIDTH*2+8, centerY+14);
    printf("当前难度: ");
    switch(currentDifficulty) {
        case DIFFICULTY_EASY:
            setColor(COLOR_GREEN);
            printf("简单");
            break;
        case DIFFICULTY_NORMAL:
            setColor(COLOR_YELLOW);
            printf("普通");
            break;
        case DIFFICULTY_HARD:
            setColor(COLOR_PURPLE);
            printf("困难");
            break;
        case DIFFICULTY_HELL:
            setColor(COLOR_RED);
            printf("地狱");
            break;
    }
}

// 绘制双人模式控制说明
void drawTwoPlayerControls() {
    int centerX = DOUBLE_CENTER_X;
    int centerY = DOUBLE_CENTER_Y;
    setColor(COLOR_WHITE);
    gotoxy(centerX+WIDTH*2+10, centerY+16);
    printf("控制说明:");
    gotoxy(centerX+WIDTH*2+8, centerY+17);
    printf("玩家1: A/D左右, W旋转");
    gotoxy(centerX+WIDTH*2+8, centerY+18);
    printf("玩家2: ←→左右, ↑旋转");
    gotoxy(centerX+WIDTH*2+8, centerY+19);
    printf("Z: 暂停, Q: 退出");
    gotoxy(centerX+WIDTH*2+8, centerY+20);
    printf("注意:建议单击按键");
}

// 为玩家2生成新方块
void generateNewShape2() {
    memcpy(currentShape2, nextShape2, sizeof(currentShape2));
    currentX2 = WIDTH/2 - 2;
    currentY2 = 0;
    currentColor2 = nextColor2;
    
    int shapeIndex = rand() % 7;
    memcpy(nextShape2, shapes[shapeIndex], sizeof(nextShape2));
    nextColor2 = 1 + rand() % 7;
}

// 玩家2旋转方块
void rotateShape2() {
    int temp[4][4];
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            temp[i][j] = currentShape2[3-j][i];
    
    if(canMoveShape2(temp, currentX2, currentY2)) {
        memcpy(currentShape2, temp, sizeof(currentShape2));
    }
}

// 检查玩家2是否可以移动
int canMove2(int x, int y) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(currentShape2[i][j]) {
                int newX = x + j;
                int newY = y + i;
                if(newX < 0 || newX >= WIDTH || newY >= HEIGHT) return 0;
                if(newY >= 0 && gameArea2[newY][newX]) return 0;
            }
        }
    }
    return 1;
}

// 检查玩家2任意形状在指定位置是否可以移动
int canMoveShape2(int shape[4][4], int x, int y) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(shape[i][j]) {
                int newX = x + j;
                int newY = y + i;
                if(newX < 0 || newX >= WIDTH || newY >= HEIGHT) return 0;
                if(newY >= 0 && gameArea2[newY][newX]) return 0;
            }
        }
    }
    return 1;
}

// 玩家2方块下落
void moveDown2() {
    if(canMove2(currentX2, currentY2 + 1)) {
        currentY2++;
    } else {
        // 固定方块
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                if(currentShape2[i][j] && currentY2 + i >= 0) {
                    gameArea2[currentY2 + i][currentX2 + j] = currentColor2;
                }
            }
        }
        clearLines2();
        generateNewShape2();
        if(!canMove2(currentX2, currentY2)) {
            gameOver2 = 1;
        }
    }
}

// 清除玩家2已满行
void clearLines2() {
    int linesCleared = 0;
    for(int i = HEIGHT-1; i >= 0; i--) {
        int isFull = 1;
        for(int j = 0; j < WIDTH; j++) {
            if(!gameArea2[i][j]) {
                isFull = 0;
                break;
            }
        }
        if(isFull) {
            linesCleared++;
            // 从当前行向下移动所有行
            for(int k = i; k > 0; k--) {
                for(int j = 0; j < WIDTH; j++) {
                    gameArea2[k][j] = gameArea2[k-1][j];
                }
            }
            // 清空最底行
            for(int j = 0; j < WIDTH; j++) {
                gameArea2[0][j] = 0;
            }
            i++; // 重新检查当前行
        }
    }
    score2 += linesCleared * 10;
    if(score2 > highScore) {
        highScore = score2;
    }
}

// 绘制双人模式暂停提示
void drawTwoPlayerPauseMessage() {
    int centerX = DOUBLE_CENTER_X;
    int centerY = DOUBLE_CENTER_Y;
    int p2_left = centerX + WIDTH + 4;
    int p2_center = p2_left + WIDTH/2;
    if(isPaused) {
        setColor(COLOR_YELLOW);
        gotoxy(p2_center-2, centerY+HEIGHT/2-4);
        printf("游戏暂停");
        
        // 创建暂停菜单按钮，减小间距和宽度
        resetButtons();
        createButton(p2_center-3, centerY+HEIGHT/2-2, 10, 3, "继续游戏", 1);
        createButton(p2_center-3, centerY+HEIGHT/2+2, 10, 3, "返回菜单", 2);
        
        // 绘制按钮
        drawAllButtons();
        
        // 重新绘制边框，修复被覆盖的问题
        drawTwoPlayerBorder();
    }
}

// 双人游戏主循环
void twoPlayerGameLoop() {
    clock_t lastMove = clock();
    clock_t lastAIMove = clock(); // AI移动的计时器
    int prevX = currentX, prevY = currentY;
    int prevX2 = currentX2, prevY2 = currentY2;
    int needRedraw = 1;
    int centerX = DOUBLE_CENTER_X;
    int centerY = DOUBLE_CENTER_Y;
    
    while(!(gameOver && gameOver2)) {
        needRedraw = 0;
        
        // 处理暂停菜单的鼠标点击和q键
        if(isPaused) {
            // 先处理键盘输入，确保按键响应
            if (kbhit()) {
                int key = getch();
                if (key == 'z' || key == 'Z' || key == '1') {
                    isPaused = 0;
                    needRedraw = 1;
                    playBackgroundMusic();
                } else if (key == 'q' || key == 'Q' || key == '2') {
                    gameOver = 1;
                    gameOver2 = 1;
                    stopBackgroundMusic();
                    twoPlayerGameOverScreen(); // 显示游戏结束界面
                    return;
                }
                continue;  // 处理完按键后直接进入下一循环
            }
            
            // 再处理鼠标事件
            int btnId = handleMouseEvent();
            if (btnId > 0) {
                switch (btnId) {
                    case 1: // 继续游戏
                        isPaused = 0;
                        needRedraw = 1;
                        playBackgroundMusic(); // 继续时播放背景音乐
                        break;
                    case 2: // 返回主菜单
                        gameOver = 1;
                        gameOver2 = 1;
                        stopBackgroundMusic();
                        break;
                }
            }
            
            // 短暂延时，减少CPU占用，但不要太长以免影响按键响应
            Sleep(10);
            continue;
        }
        
        // 处理键盘输入
        if(kbhit()) {
            // 按键冷却检查
            clock_t currentTime = clock();
            if (currentTime - lastKeyTime < KEY_COOLDOWN) {
                // 如果冷却时间未到，忽略按键
                getch(); // 清除按键缓冲
                continue;
            }
            
            lastKeyTime = currentTime; // 更新上次按键时间
            int key = getch();
            if(key == 224) {  // 方向键的第一个字符
                key = getch();  // 获取方向键的第二个字符
                if(!isPaused && !isAIMode) { // 在人机对战模式下，方向键不控制玩家2
                    switch(key) {
                        case 72:  // 上箭头 - 玩家2旋转
                            needRedraw = 1;
                            if(!gameOver2) rotateShape2();
                            break;
                        case 80:  // 下箭头 - 玩家2下移
                            if(!gameOver2 && canMove2(currentX2, currentY2 + 1)) {
                                needRedraw = 1;
                                currentY2++;
                            }
                            break;
                        case 75:  // 左箭头 - 玩家2左移
                            if(!gameOver2 && canMove2(currentX2-1, currentY2)) {
                                needRedraw = 1;
                                currentX2--;
                            }
                            break;
                        case 77:  // 右箭头 - 玩家2右移
                            if(!gameOver2 && canMove2(currentX2+1, currentY2)) {
                                needRedraw = 1;
                                currentX2++;
                            }
                            break;
                    }
                }
            } else {
                switch(key) {
                    case 'a': case 'A':  // 玩家1左移
                        if(!isPaused && !gameOver && canMove(currentX-1, currentY)) {
                            needRedraw = 1;
                            currentX--;
                        }
                        break;
                    case 'd': case 'D':  // 玩家1右移
                        if(!isPaused && !gameOver && canMove(currentX+1, currentY)) {
                            needRedraw = 1;
                            currentX++;
                        }
                        break;
                    case 'w': case 'W':  // 玩家1旋转
                        if(!isPaused && !gameOver) {
                            needRedraw = 1;
                            rotateShape();
                        }
                        break;
                    case 's': case 'S':  // 玩家1下移
                        if(!isPaused && !gameOver && canMove(currentX, currentY + 1)) {
                            needRedraw = 1;
                            currentY++;
                        }
                        break;
                    case 'z': case 'Z':  // 暂停
                        needRedraw = 1;
                        isPaused = !isPaused;
                        break;
                    case 'q': case 'Q':  // 退出
                        gameOver = 1;
                        gameOver2 = 1;
                        stopBackgroundMusic();
                        twoPlayerGameOverScreen(); // 显示游戏结束界面
                        return;
                    case 'm': case 'M': // 音乐开关
                        isMusicEnabled = !isMusicEnabled;
                        if(isMusicEnabled) {
                            playBackgroundMusic();
                        } else {
                            stopBackgroundMusic();
                        }
                        break;
                }
            }
        }
        
        // AI控制逻辑
        if(isAIMode && !isPaused && !gameOver2) {
            // 每隔一段时间让AI做一次移动决策
            clock_t currentTime = clock();
            if (currentTime - lastAIMove > 500) { // 500毫秒做一次决策
                aiMakeMove();
                needRedraw = 1;
                lastAIMove = currentTime;
            }
        }
        
        // 自动下落
        if(!isPaused && clock() - lastMove > getSpeedByScore()) {
            needRedraw = 1;
            if(!gameOver) moveDown();
            if(!gameOver2) moveDown2();
            lastMove = clock();
        }
        
        // 渲染游戏画面
        if(needRedraw || prevX != currentX || prevY != currentY || prevX2 != currentX2 || prevY2 != currentY2) {
            // 擦除玩家1旧位置
            if(prevX != currentX || prevY != currentY) {
                setColor(COLOR_BLACK);
                for(int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        if(currentShape[i][j]) {
                            gotoxy(centerX+prevX+j+1, centerY+prevY+i+1);
                            printf("  ");
                        }
                    }
                }
            }
            
            // 擦除玩家2旧位置
            if(prevX2 != currentX2 || prevY2 != currentY2) {
                setColor(COLOR_BLACK);
                for(int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        if(currentShape2[i][j]) {
                            gotoxy(centerX+WIDTH+4+prevX2+j, centerY+prevY2+i+1);
                            printf("  ");
                        }
                    }
                }
            }
            
            // 绘制游戏界面
            drawTwoPlayerBorder();
            drawTwoPlayerGameArea();
            drawTwoPlayerNextShape();
            drawTwoPlayerScore();
            
            // 根据游戏模式显示不同的控制说明
            if (isAIMode) {
                // 在人机对战模式下显示不同的控制说明
                setColor(COLOR_WHITE);
                int infoX = centerX+WIDTH*2+10;
                int infoY = centerY+16;
                
                gotoxy(infoX, infoY);
                printf("控制说明:");
                gotoxy(infoX-2, infoY+1);
                printf("玩家: A/D左右, W旋转");
                gotoxy(infoX-2, infoY+2);
                printf("电脑: 自动控制");
                gotoxy(infoX-2, infoY+3);
                printf("Z: 暂停, Q: 退出");
                gotoxy(infoX-2, infoY+4);
                printf("注意:建议单击按键");
            } else {
                // 在双人模式下显示标准控制说明
                drawTwoPlayerControls();
            }
            
            // 绘制玩家1当前方块
            if(!isPaused && !gameOver) {
                setColor(currentColor);
                for(int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        if(currentShape[i][j]) {
                            gotoxy(centerX+currentX+j+1, centerY+currentY+i+1);
                            printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
                        }
                    }
                }
            }
            
            // 绘制玩家2当前方块
            if(!isPaused && !gameOver2) {
                setColor(currentColor2);
                for(int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        if(currentShape2[i][j]) {
                            gotoxy(centerX+WIDTH+4+currentX2+j, centerY+currentY2+i+1);
                            printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
                        }
                    }
                }
            }
            
            // 更新上一次位置
            prevX = currentX;
            prevY = currentY;
            prevX2 = currentX2;
            prevY2 = currentY2;
        }
        
        // 显示暂停信息
        drawTwoPlayerPauseMessage();
        
        Sleep(75);
    }
}

// 初始化音频资源
void initAudioResources() {
    // 音频资源初始化逻辑
}

// 播放背景音乐
void playBackgroundMusic() {
    if (isMusicEnabled) {
        // 创建临时文件
        char tempPath[MAX_PATH];
        char tempFileName[MAX_PATH];
        GetTempPath(MAX_PATH, tempPath);
        sprintf(tempFileName, "%s\\bgMusic.mp3", tempPath);
        
        // 写入临时文件
        FILE* file = fopen(tempFileName, "wb");
        if (file) {
            fwrite(theme1_data, 1, theme1_size, file);
            fclose(file);
            
            // 使用MCI播放
            char command[256];
            sprintf(command, "open \"%s\" type mpegvideo alias bgMusic", tempFileName);
            if (mciSendString(command, NULL, 0, NULL) == 0) {
                mciSendString("play bgMusic repeat", NULL, 0, NULL);
            }
            // 删除临时文件
            DeleteFile(tempFileName);
        }
    }
}

// 播放菜单音乐
void playMenuMusic() {
    if (isMusicEnabled) {
        // 创建临时文件
        char tempPath[MAX_PATH];
        char tempFileName[MAX_PATH];
        GetTempPath(MAX_PATH, tempPath);
        sprintf(tempFileName, "%s\\menuMusic.mp3", tempPath);
        
        // 写入临时文件
        FILE* file = fopen(tempFileName, "wb");
        if (file) {
            fwrite(theme2_data, 1, theme2_size, file);
            fclose(file);
            
            // 使用MCI播放
            char command[256];
            sprintf(command, "open \"%s\" type mpegvideo alias menuMusic", tempFileName);
            if (mciSendString(command, NULL, 0, NULL) == 0) {
                mciSendString("play menuMusic repeat", NULL, 0, NULL);
            }
            // 删除临时文件
            DeleteFile(tempFileName);
        }
    }
}

// 播放音效
void playSound(const char* sound) {
    if (!isSoundEnabled) return;
    
    const unsigned char* data = NULL;
    size_t size = 0;
    
    if (strcmp(sound, SOUND_ROTATE) == 0) {
        data = rotate_data;
        size = rotate_size;
    } else if (strcmp(sound, SOUND_MOVE) == 0) {
        data = move_data;
        size = move_size;
    } else if (strcmp(sound, SOUND_DROP) == 0) {
        data = drop_data;
        size = drop_size;
    } else if (strcmp(sound, SOUND_CLEAR) == 0) {
        data = clear_data;
        size = clear_size;
    } else if (strcmp(sound, SOUND_GAMEOVER) == 0) {
        data = gameover_data;
        size = gameover_size;
    }
    
    if (data) {
        PlaySound((LPCSTR)data, NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    }
}

// 检查音效文件是否存在 - 不再需要，因为音频已嵌入
void checkSoundFiles() {
    // 音频已嵌入，无需检查
}

// 创建示例音效文件 - 不再需要，因为音频已嵌入
void createSampleSoundFiles() {
    // 音频已嵌入，无需创建示例文件
}

// 停止背景音乐
void stopBackgroundMusic() {
    char status[50];
    if (mciSendString("status bgMusic mode", status, sizeof(status), NULL) == 0) {
        mciSendString("stop bgMusic", NULL, 0, NULL);
        mciSendString("close bgMusic", NULL, 0, NULL);
    }
}

// 停止菜单音乐
void stopMenuMusic() {
    char status[50];
    if (mciSendString("status menuMusic mode", status, sizeof(status), NULL) == 0) {
        mciSendString("stop menuMusic", NULL, 0, NULL);
        mciSendString("close menuMusic", NULL, 0, NULL);
    }
}

// 难度选择界面
void difficultySelectScreen() {
    system("cls");  // 先清屏
    resetButtons();  // 然后重置按钮
    
    int running = 1;
    int needRedraw = 1;
    
    // 计算居中的横坐标
    int centerX = 25;  // 适应100列的窗口
    
    while (running) {
        if (needRedraw) {
            system("cls");  // 在重绘之前清屏
            resetButtons();  // 重置按钮状态
            
            // 绘制整个界面的边框，调整参数
            drawScreenBorder(8, 0, 74, 30);
            
            // 创建难度选择按钮，调整大小和间距
            createButton(centerX-1, 7, 10, 3, " 简  单 ", 1);
            createButton(centerX-1, 11, 10, 3, " 普  通 ", 2);
            createButton(centerX-1, 15, 10, 3, " 困  难 ", 3);
            createButton(centerX-1, 19, 10, 3, " 地  狱 ", 4);
            createButton(centerX-1, 23, 10, 3, "返回设置", 5);
            
            // 绘制标题及边框
            drawTitleBorder(centerX, 3, "难度选择");
            
            // 绘制所有按钮
            drawAllButtons();
            
            setColor(COLOR_GREEN);
            gotoxy(centerX - 9, 28);
            printf("使用鼠标点击按钮，或按1-5数字键选择");
            
            needRedraw = 0;  // 重置重绘标志
        }
        
        // 处理键盘输入
        if(kbhit()) {
            int key = getch();
            if (key == '1') {
                currentDifficulty = DIFFICULTY_EASY;
                needRedraw = 1;
            } else if (key == '2') {
                currentDifficulty = DIFFICULTY_NORMAL;
                needRedraw = 1;
            } else if (key == '3') {
                currentDifficulty = DIFFICULTY_HARD;
                needRedraw = 1;
            } else if (key == '4') {
                currentDifficulty = DIFFICULTY_HELL;
                needRedraw = 1;
            } else if (key == '5') {
                running = 0;
            }
        }
        
        // 处理鼠标事件
        int btnId = handleMouseEvent();
        if (btnId > 0) {
            switch (btnId) {
                case 1: // 简单
                    currentDifficulty = DIFFICULTY_EASY;
                    needRedraw = 1;
                    break;
                case 2: // 普通
                    currentDifficulty = DIFFICULTY_NORMAL;
                    needRedraw = 1;
                    break;
                case 3: // 困难
                    currentDifficulty = DIFFICULTY_HARD;
                    needRedraw = 1;
                    break;
                case 4: // 地狱
                    currentDifficulty = DIFFICULTY_HELL;
                    needRedraw = 1;
                    break;
                case 5: // 返回设置
                    running = 0;
                    break;
            }
        }
        
        // 短暂延时，防止CPU占用过高
        Sleep(50);
    }
}

// 方块样式更改界面
void styleChangeScreen() {
    system("cls");  // 先清屏
    resetButtons();  // 然后重置按钮
    
    int running = 1;
    int needRedraw = 1;
    
    // 计算居中的横坐标
    int centerX = 25;  // 与主菜单保持一致
    
    while (running) {
        if (needRedraw) {
            system("cls");  // 在重绘之前清屏
            resetButtons();  // 重置按钮状态
            
            // 绘制整个界面的边框，调整参数
            drawScreenBorder(8, 0, 74, 30);
            
            // 创建样式选择按钮，调整大小和间距
            createButton(centerX-1, 14, 10, 3, "切换样式", 1);
            createButton(centerX-1, 17, 10, 3, "返回设置", 2);
            
            // 绘制标题及边框
            drawTitleBorder(centerX, 4, "样式设置");
            
            setColor(COLOR_YELLOW);
            gotoxy(centerX -1, 8);
            printf("当前样式:  ");
            
            setColor(COLOR_WHITE);
            printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            
            // 在样式选择界面添加示例显示
            setColor(COLOR_CYAN);
            gotoxy(centerX , 11);
            printf("示例效果:");
            
            // 显示不同颜色的方块示例
            for (int i = 0; i < 7; i++) {
                setColor(i+1);
                gotoxy(centerX - 9 + i*4, 12);
                printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            }
            
            // 绘制所有按钮
            drawAllButtons();
            
            // 显示提示信息，位置与主菜单一致
            setColor(COLOR_GREEN);
            gotoxy(centerX - 6, 24);
            printf("使用鼠标点击按钮，或按1-2数字键选择");
            gotoxy(centerX - 3, 26);
            printf("按2返回设置界面");
            
            needRedraw = 0;  // 重置重绘标志
        }
        
        // 处理键盘输入
        if(kbhit()) {
            int key = getch();
            if (key == '1') {
                switchBlockSymbol();
                needRedraw = 1;
            } else if (key == '2') {
                running = 0;
            }
        }
        
        // 处理鼠标事件
        int btnId = handleMouseEvent();
        if (btnId > 0) {
            switch (btnId) {
                case 1: // 切换样式
                    switchBlockSymbol();
                    needRedraw = 1;
                    break;
                case 2: // 返回设置
                    running = 0;
                    break;
            }
        }
        
        // 短暂延时，防止CPU占用过高
        Sleep(50);
    }
}

// 音效设置界面
void soundSettingsScreen() {
    system("cls");  // 先清屏
    resetButtons();  // 然后重置按钮
    
    int running = 1;
    int needRedraw = 1;
    
    // 计算居中的横坐标
    int centerX = 25;  // 与主菜单保持一致
    
    while (running) {
        if (needRedraw) {
            system("cls");  // 在重绘之前清屏
            resetButtons();  // 重置按钮状态
            
            // 绘制整个界面的边框，调整参数
            drawScreenBorder(8, 0, 74, 30);
            
            // 创建音效设置按钮
            resetButtons();
            char musicBtnText[50], soundBtnText[50];
            sprintf(musicBtnText, "音乐: %s", isMusicEnabled ? "开" : "关");
            sprintf(soundBtnText, "音效: %s", isSoundEnabled ? "开" : "关");
            
            // 调整按钮大小和间距，与主菜单保持一致
            createButton(centerX-1, 10, 10, 3, musicBtnText, 1);
            createButton(centerX-1, 13, 10, 3, soundBtnText, 2);
            createButton(centerX-1, 16, 10, 3, "返回设置", 3);
            
            // 绘制标题及边框
            drawTitleBorder(centerX, 4, "音效设置");
            
            // 绘制所有按钮
            drawAllButtons();
            
            // 显示提示信息，位置与主菜单一致
            setColor(COLOR_GREEN);
            gotoxy(centerX - 6, 23);
            printf("使用鼠标点击按钮，或按1-3数字键选择");
            gotoxy(centerX - 3, 25);
            printf("按3返回设置界面");
            
            needRedraw = 0;  // 重置重绘标志
        }
        
        // 处理键盘输入
        if(kbhit()) {
            int key = getch();
            if (key == '1') {
                isMusicEnabled = !isMusicEnabled;
                needRedraw = 1;
            } else if (key == '2') {
                isSoundEnabled = !isSoundEnabled;
                needRedraw = 1;
            } else if (key == '3') {
                running = 0;
            }
        }
        
        // 处理鼠标事件
        int btnId = handleMouseEvent();
        if (btnId > 0) {
            switch (btnId) {
                case 1: // 切换音乐
                    isMusicEnabled = !isMusicEnabled;
                    needRedraw = 1;
                    break;
                case 2: // 切换音效
                    isSoundEnabled = !isSoundEnabled;
                    needRedraw = 1;
                    break;
                case 3: // 返回设置
                    running = 0;
                    break;
            }
        }
        
        // 短暂延时，防止CPU占用过高
        Sleep(50);
    }
}

// 单人游戏结束界面
void singlePlayerGameOverScreen() {
    // 更新用户分数
    updateUserScore();
    
    int centerX = SINGLE_CENTER_X;
    int centerY = SINGLE_CENTER_Y;
    int shouldReturnToMenu = 0;  // 使用局部变量代替全局变量
    
    system("cls");
    resetButtons();
    
    // 绘制整个界面的边框，调整参数
    drawScreenBorder(centerX+2, centerY, 30, 20);
    
    // 绘制游戏结束信息
    setColor(COLOR_YELLOW);
    gotoxy(centerX+5, centerY+5); 
    if (score >= 10000) {
        printf("你赢了！最终得分：%d", score);
    } else {
        printf("游戏结束！最终得分：%d", score);
    }
    
    gotoxy(centerX+5, centerY+7); 
    printf("最高分纪录：%d", highScore);
    
    // 创建按钮
    createButton(centerX+7, centerY+10, 10, 3, "重新开始", 1);
    createButton(centerX+7, centerY+14, 10, 3, "返回菜单", 2);
    
    // 绘制按钮
    drawAllButtons();
    
    // 清空输入缓冲区，防止提示界面一闪而过
    while (kbhit()) getch();
    
    // 等待用户选择
    int running = 1;
    while (running) {
        // 处理键盘输入
        if (kbhit()) {
            int key = getch();
            if (key == '1') {
                // 重新开始
                running = 0;
            } else if (key == '2' || key == 'q' || key == 'Q') {
                // 返回菜单
                shouldReturnToMenu = 1;
                running = 0;
            } else if (key == 27) { // ESC
                exit(0);
            }
        }
        
        // 处理鼠标事件
        int btnId = handleMouseEvent();
        if (btnId == 1) {
            // 重新开始，返回主循环
            running = 0;
        } else if (btnId == 2) {
            // 返回菜单
            shouldReturnToMenu = 1;
            running = 0;
        }
        
        Sleep(50);
    }
    
    // 返回菜单还是重新开始
    if (shouldReturnToMenu) {
        playMenuMusic();
        gameStartScreen();
    } else {
        // 重新开始游戏
        initGame();
        gameLoop();
    }
}

// 双人游戏结束界面
void twoPlayerGameOverScreen() {
    // 更新用户分数
    updateUserScore();
    
    int centerX = DOUBLE_CENTER_X;
    int centerY = DOUBLE_CENTER_Y;
    int shouldReturnToMenu = 0;  // 使用局部变量代替全局变量
    
    system("cls");
    resetButtons();
    
    // 绘制整个界面的边框，调整参数
    drawScreenBorder(centerX+11, centerY, 30, 20);
    
    // 绘制游戏结束信息
    setColor(COLOR_YELLOW);
    gotoxy(centerX+15, centerY+5);
    
    if (gameOver && gameOver2) {
        if (isAIMode) {
            // 人机对战模式的结束信息
            if (score > score2) {
                printf("游戏结束！玩家获胜！");
            } else if (score2 > score) {
                printf("游戏结束！电脑获胜！");
            } else {
                printf("游戏结束！平局！");
            }
            gotoxy(centerX+15, centerY+7); printf("玩家得分 ：%d", score);
            gotoxy(centerX+15, centerY+8); printf("电脑得分 ：%d", score2);
        } else {
            // 双人模式的结束信息
            if (score > score2) {
                printf("游戏结束！玩家1获胜！");
            } else if (score2 > score) {
                printf("游戏结束！玩家2获胜！");
            } else {
                printf("游戏结束！平局！");
            }
            gotoxy(centerX+15, centerY+7); printf("玩家1得分：%d", score);
            gotoxy(centerX+15, centerY+8); printf("玩家2得分：%d", score2);
        }
        gotoxy(centerX+15, centerY+9); printf("最高分纪录：%d", highScore);
    }
    
    // 创建按钮
    createButton(centerX+16, centerY+12, 10, 3, "重新开始", 1);
    createButton(centerX+16, centerY+16, 10, 3, "返回菜单", 2);
    
    // 绘制按钮
    drawAllButtons();
    
    // 清空输入缓冲区，防止提示界面一闪而过
    while (kbhit()) getch();
    
    // 等待用户选择
    int running = 1;
    while (running) {
        // 处理键盘输入
        if (kbhit()) {
            int key = getch();
            if (key == '1') {
                // 重新开始
                running = 0;
            } else if (key == '2' || key == 'q' || key == 'Q') {
                // 返回菜单
                shouldReturnToMenu = 1;
                running = 0;
            } else if (key == 27) { // ESC
                exit(0);
            }
        }
        
        // 处理鼠标事件
        int btnId = handleMouseEvent();
        if (btnId == 1) {
            // 重新开始，返回主循环
            running = 0;
        } else if (btnId == 2) {
            // 返回菜单
            shouldReturnToMenu = 1;
            running = 0;
        }
        
        Sleep(50);
    }
    
    // 返回菜单还是重新开始
    if (shouldReturnToMenu) {
        playMenuMusic();
        gameStartScreen();
    } else {
        // 重新开始游戏
        initTwoPlayerGame();
        twoPlayerGameLoop();
    }
}

// AI控制函数，决定AI的下一步移动
void aiMakeMove() {
    // 简单AI策略：
    // 1. 尝试找到最佳的水平位置（尽量靠近左侧）
    // 2. 尝试旋转方块以获得更好的放置
    // 3. 快速下落到底部
    
    // 记录当前状态
    int bestX = currentX2;
    int bestRotation = 0;
    int bestScore = -1;
    int tempShape[4][4];
    
    // 复制当前方块形状用于尝试旋转
    memcpy(tempShape, currentShape2, sizeof(tempShape));
    
    // 尝试不同的旋转次数（0-3次）
    for (int rot = 0; rot < 4; rot++) {
        // 尝试不同的水平位置
        for (int x = 0; x < WIDTH; x++) {
            // 检查是否可以放置在当前位置
            if (canMoveShape2(tempShape, x, currentY2)) {
                // 模拟下落到底部
                int y = currentY2;
                while (canMoveShape2(tempShape, x, y + 1)) {
                    y++;
                }
                
                // 计算这个位置的得分（简单策略：优先选择靠左侧且接触已有方块的位置）
                int score = WIDTH - x; // 优先选择靠左的位置
                
                // 检查是否接触已有方块或底部
                int hasContact = 0;
                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        if (tempShape[i][j]) {
                            // 检查下方是否有方块或到达底部
                            if (y + i + 1 >= HEIGHT || 
                                (y + i + 1 < HEIGHT && gameArea2[y + i + 1][x + j])) {
                                hasContact++;
                            }
                            // 检查左侧是否有方块
                            if (x + j - 1 >= 0 && x + j - 1 < WIDTH && 
                                gameArea2[y + i][x + j - 1]) {
                                hasContact++;
                            }
                            // 检查右侧是否有方块
                            if (x + j + 1 >= 0 && x + j + 1 < WIDTH && 
                                gameArea2[y + i][x + j + 1]) {
                                hasContact++;
                            }
                        }
                    }
                }
                
                // 接触越多越好
                score += hasContact * 2;
                
                // 更新最佳位置
                if (score > bestScore) {
                    bestScore = score;
                    bestX = x;
                    bestRotation = rot;
                }
            }
        }
        
        // 旋转方块以尝试下一个旋转状态
        int rotatedShape[4][4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                rotatedShape[i][j] = tempShape[3-j][i];
            }
        }
        memcpy(tempShape, rotatedShape, sizeof(tempShape));
    }
    
    // 执行最佳移动
    // 1. 先旋转到最佳状态
    for (int i = 0; i < bestRotation; i++) {
        rotateShape2();
    }
    
    // 2. 移动到最佳水平位置
    if (bestX < currentX2) {
        // 需要向左移动
        while (currentX2 > bestX && canMove2(currentX2 - 1, currentY2)) {
            currentX2--;
        }
    } else if (bestX > currentX2) {
        // 需要向右移动
        while (currentX2 < bestX && canMove2(currentX2 + 1, currentY2)) {
            currentX2++;
        }
    }
    
    // 3. 快速下落
    while (canMove2(currentX2, currentY2 + 1)) {
        currentY2++;
    }
}

// 初始化用户系统，检查并创建users文件夹
void initUserSystem() {
    // 检查users文件夹是否存在，不存在则创建
    if (_mkdir("users") == -1) {
        // 如果文件夹已存在，mkdir会返回-1
        // 这里我们不需要做任何处理，因为文件夹已经存在
    }
}

// 获取用户输入
void getInput(char* buffer, int maxLen, int isPassword) {
    int pos = 0;
    int key;
    
    // 清空缓冲区
    memset(buffer, 0, maxLen);
    
    while (1) {
        key = getch();
        
        if (key == 13) { // 回车键
            buffer[pos] = '\0';
            break;
        } else if (key == 8 && pos > 0) { // 退格键
            pos--;
            buffer[pos] = '\0';
            printf("\b \b"); // 删除一个字符
        } else if (key >= 32 && key <= 126 && pos < maxLen - 1) { // 可打印字符
            buffer[pos++] = key;
            if (isPassword) {
                printf("*"); // 密码显示为星号
            } else {
                printf("%c", key); // 正常显示字符
            }
        }
    }
    printf("\n"); // 输入完成后换行
}

// 检查用户是否存在
int checkUserExists(const char* username) {
    char filePath[100];
    sprintf(filePath, "users/%s.dat", username);
    
    FILE* file = fopen(filePath, "rb");
    if (file) {
        fclose(file);
        return 1; // 用户存在
    }
    return 0; // 用户不存在
}

// 创建用户文件
int createUserFile(UserInfo* user) {
    char filePath[100];
    sprintf(filePath, "users/%s.dat", user->username);
    
    FILE* file = fopen(filePath, "wb");
    if (!file) {
        return 0; // 创建失败
    }
    
    // 写入用户信息
    fwrite(user, sizeof(UserInfo), 1, file);
    fclose(file);
    return 1; // 创建成功
}

// 读取用户文件
int readUserFile(const char* username, UserInfo* user) {
    char filePath[100];
    sprintf(filePath, "users/%s.dat", username);
    
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        return 0; // 读取失败
    }
    
    // 读取用户信息
    fread(user, sizeof(UserInfo), 1, file);
    fclose(file);
    return 1; // 读取成功
}

// 用户注册界面
void registerScreen() {
    system("cls");
    resetButtons();
    
    int centerX = 30;
    int centerY = 10;
    char username[50] = {0};
    char password[50] = {0};
    int registerResult = -1; // -1: 未注册, 0: 注册失败, 1: 注册成功
    
    // 初始化用户系统
    initUserSystem();
    
    // 绘制界面边框
    drawScreenBorder(centerX-10, centerY-5, 32, 20);
    
    // 绘制标题
    setColor(COLOR_CYAN);
    gotoxy(centerX-4, centerY-4);
    printf("用户注册");
    
    // 绘制输入框和按钮
    setColor(COLOR_WHITE);
    gotoxy(centerX-8, centerY);
    printf("账号：");
    
    // 绘制账号输入框边框
    gotoxy(centerX-4, centerY-1);
    printf("┌");
    for(int i = 0; i < 15; i++) printf("─");
    printf("┐");
    
    gotoxy(centerX-4, centerY);
    printf("│");
    gotoxy(centerX+4, centerY);
    printf("│");
    
    gotoxy(centerX-4, centerY+1);
    printf("└");
    for(int i = 0; i < 15; i++) printf("─");
    printf("┘");
    
    gotoxy(centerX-8, centerY+4);
    printf("密码：");
    
    // 绘制密码输入框边框
    gotoxy(centerX-4, centerY+3);
    printf("┌");
    for(int i = 0; i < 15; i++) printf("─");
    printf("┐");
    
    gotoxy(centerX-4, centerY+4);
    printf("│");
    gotoxy(centerX+4, centerY+4);
    printf("│");
    
    gotoxy(centerX-4, centerY+5);
    printf("└");
    for(int i = 0; i < 15; i++) printf("─");
    printf("┘");
    
    // 创建注册按钮
    createButton(centerX-4, centerY+7, 6, 3, "注册", 1);
    createButton(centerX-4, centerY+11, 6, 3, "返回", 2);
    
    // 绘制按钮
    drawAllButtons();
    
    int running = 1;
    int inputFocus = 0; // 0: 无焦点, 1: 账号, 2: 密码
    
    while (running) {
        // 处理键盘输入
        if (kbhit()) {
            int key = getch();
            switch (key) {
                case '1': // 注册
                    // 检查用户名和密码是否为空
                    if (strlen(username) == 0 || strlen(password) == 0) {
                        setColor(COLOR_RED);
                        gotoxy(centerX-8, centerY+18);
                        printf("                                      ");
                        gotoxy(centerX-8, centerY+18);
                        printf("账号和密码不能为空，请重新输入！");
                        continue;
                    }
                    
                    // 检查用户是否已存在
                    if (checkUserExists(username)) {
                        registerResult = 0; // 注册失败，用户已存在
                        setColor(COLOR_RED);
                        gotoxy(centerX-8, centerY+18);
                        printf("                                      ");
                        gotoxy(centerX-8, centerY+18);
                        printf("注册失败：用户名已存在！");
                    } else {
                        // 创建新用户
                        UserInfo newUser;
                        strcpy(newUser.username, username);
                        strcpy(newUser.password, password);
                        newUser.highScoreEasy = 0;
                        newUser.highScoreNormal = 0;
                        newUser.highScoreHard = 0;
                        newUser.highScoreHell = 0;
                        
                        if (createUserFile(&newUser)) {
                            registerResult = 1; // 注册成功
                            setColor(COLOR_GREEN);
                            gotoxy(centerX-8, centerY+18);
                            printf("                                      ");
                            gotoxy(centerX-8, centerY+18);
                            printf("注册成功！请返回登录。");
                        } else {
                            registerResult = 0; // 注册失败，文件创建失败
                            setColor(COLOR_RED);
                            gotoxy(centerX-8, centerY+18);
                            printf("                                      ");
                            gotoxy(centerX-8, centerY+18);
                            printf("注册失败：无法创建用户文件！");
                        }
                    }
                    break;
                case '2': // 返回
                case 27:  // ESC键
                    running = 0;
                    break;
            }
        }
        
        // 处理鼠标事件
        int btnId = handleMouseEvent();
        
        // 处理输入框点击
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        INPUT_RECORD inputBuffer;
        DWORD numEvents;
        
        // 检查是否有输入事件
        PeekConsoleInput(hInput, &inputBuffer, 1, &numEvents);
        if (numEvents > 0) {
            ReadConsoleInput(hInput, &inputBuffer, 1, &numEvents);
            
            // 处理鼠标事件
            if (inputBuffer.EventType == MOUSE_EVENT) {
                MOUSE_EVENT_RECORD mouseEvent = inputBuffer.Event.MouseEvent;
                int mouseX = mouseEvent.dwMousePosition.X;
                int mouseY = mouseEvent.dwMousePosition.Y;
                
                // 检查是否点击了账号输入框
                if (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
                    if (mouseY == centerY && mouseX >= (centerX-3)*2 && mouseX <= (centerX+12)*2) {
                        // 点击了账号输入框
                        inputFocus = 1;
                        
                        // 清除输入框内容
                        gotoxy(centerX-3, centerY);
                        for (int i = 0; i < 15; i++) printf(" ");
                        
                        // 设置光标位置
                        gotoxy(centerX-3, centerY);
                        
                        // 获取用户输入
                        getInput(username, sizeof(username), 0);
                        
                        inputFocus = 0;
                    }
                    // 检查是否点击了密码输入框
                    else if (mouseY == centerY+4 && mouseX >= (centerX-3)*2 && mouseX <= (centerX+12)*2) {
                        // 点击了密码输入框
                        inputFocus = 2;
                        
                        // 清除输入框内容
                        gotoxy(centerX-3, centerY+4);
                        for (int i = 0; i < 15; i++) printf(" ");
                        
                        // 设置光标位置
                        gotoxy(centerX-3, centerY+4);
                        
                        // 获取用户输入
                        getInput(password, sizeof(password), 1);
                        
                        inputFocus = 0;
                    }
                }
            }
        }
        
        // 处理按钮点击
        if (btnId == 1) { // 注册按钮
            // 检查用户名和密码是否为空
            if (strlen(username) == 0 || strlen(password) == 0) {
                setColor(COLOR_RED);
                gotoxy(centerX-8, centerY+18);
                printf("                                      ");
                gotoxy(centerX-8, centerY+18);
                printf("账号和密码不能为空，请重新输入！");
                continue;
            }
            
            // 检查用户是否已存在
            if (checkUserExists(username)) {
                registerResult = 0; // 注册失败，用户已存在
                setColor(COLOR_RED);
                gotoxy(centerX-8, centerY+18);
                printf("                                      ");
                gotoxy(centerX-8, centerY+18);
                printf("注册失败：用户名已存在！");
            } else {
                // 创建新用户
                UserInfo newUser;
                strcpy(newUser.username, username);
                strcpy(newUser.password, password);
                newUser.highScoreEasy = 0;
                newUser.highScoreNormal = 0;
                newUser.highScoreHard = 0;
                newUser.highScoreHell = 0;
                
                if (createUserFile(&newUser)) {
                    registerResult = 1; // 注册成功
                    setColor(COLOR_GREEN);
                    gotoxy(centerX-8, centerY+18);
                    printf("                                      ");
                    gotoxy(centerX-8, centerY+18);
                    printf("注册成功！请返回登录。");
                } else {
                    registerResult = 0; // 注册失败，文件创建失败
                    setColor(COLOR_RED);
                    gotoxy(centerX-8, centerY+18);
                    printf("                                      ");
                    gotoxy(centerX-8, centerY+18);
                    printf("注册失败：无法创建用户文件！");
                }
            }
        } else if (btnId == 2) { // 返回按钮
            running = 0;
        }
        
        Sleep(50);
    }
}

// 用户登录界面
void loginScreen() {
    system("cls");
    resetButtons();
    
    int centerX = 30;
    int centerY = 10;
    char username[50] = {0};
    char password[50] = {0};
    int loginResult = -1; // -1: 未登录, 0: 登录失败, 1: 登录成功
    
    // 初始化用户系统
    initUserSystem();
    
    // 绘制界面边框
    drawScreenBorder(centerX-10, centerY-5, 32, 20);
    
    // 绘制标题
    setColor(COLOR_CYAN);
    gotoxy(centerX-4, centerY-4);
    printf("用户登录");
    
    // 绘制输入框和按钮
    setColor(COLOR_WHITE);
    gotoxy(centerX-8, centerY);
    printf("账号：");
    
    // 绘制账号输入框边框
    gotoxy(centerX-4, centerY-1);
    printf("┌");
    for(int i = 0; i < 15; i++) printf("─");
    printf("┐");
    
    gotoxy(centerX-4, centerY);
    printf("│");
    gotoxy(centerX+4, centerY);
    printf("│");
    
    gotoxy(centerX-4, centerY+1);
    printf("└");
    for(int i = 0; i < 15; i++) printf("─");
    printf("┘");
    
    gotoxy(centerX-8, centerY+4);
    printf("密码：");
    
    // 绘制密码输入框边框
    gotoxy(centerX-4, centerY+3);
    printf("┌");
    for(int i = 0; i < 15; i++) printf("─");
    printf("┐");
    
    gotoxy(centerX-4, centerY+4);
    printf("│");
    gotoxy(centerX+4, centerY+4);
    printf("│");
    
    gotoxy(centerX-4, centerY+5);
    printf("└");
    for(int i = 0; i < 15; i++) printf("─");
    printf("┘");
    
    // 创建登录按钮
    createButton(centerX-4, centerY+7, 6, 3, "登录", 1);
    createButton(centerX-4, centerY+11, 6, 3, "返回", 2);
    
    // 绘制按钮
    drawAllButtons();
    
    int running = 1;
    int inputFocus = 0; // 0: 无焦点, 1: 账号, 2: 密码
    
    while (running) {
        // 处理键盘输入
        if (kbhit()) {
            int key = getch();
            switch (key) {
                case '1': // 登录
                    // 检查用户名和密码是否为空
                    if (strlen(username) == 0 || strlen(password) == 0) {
                        setColor(COLOR_RED);
                        gotoxy(centerX-10, centerY+18);
                        printf("                                      ");
                        gotoxy(centerX-10, centerY+18);
                        printf("账号和密码不能为空，请重新输入！");
                        continue;
                    }
                    
                    // 检查用户是否存在
                    if (!checkUserExists(username)) {
                        loginResult = 0; // 登录失败，用户不存在
                        setColor(COLOR_RED);
                        gotoxy(centerX-10, centerY+18);
                        printf("                                      ");
                        gotoxy(centerX-10, centerY+18);
                        printf("登录失败：用户不存在！");
                    } else {
                        // 读取用户信息
                        UserInfo user;
                        if (readUserFile(username, &user)) {
                            // 验证密码
                            if (strcmp(user.password, password) == 0) {
                                loginResult = 1; // 登录成功
                                
                                // 更新当前用户信息
                                memcpy(&currentUser, &user, sizeof(UserInfo));
                                isLoggedIn = 1;
                                
                                setColor(COLOR_GREEN);
                                gotoxy(centerX-10, centerY+18);
                                printf("                                      ");
                                gotoxy(centerX-10, centerY+18);
                                printf("登录成功！欢迎回来，%s！", username);
                                
                                // 短暂延时后返回
                                Sleep(1500);
                                running = 0;
                            } else {
                                loginResult = 0; // 登录失败，密码错误
                                setColor(COLOR_RED);
                                gotoxy(centerX-10, centerY+18);
                                printf("                                      ");
                                gotoxy(centerX-10, centerY+18);
                                printf("登录失败：密码错误！");
                            }
                        } else {
                            loginResult = 0; // 登录失败，无法读取用户文件
                            setColor(COLOR_RED);
                            gotoxy(centerX-10, centerY+18);
                            printf("                                      ");
                            gotoxy(centerX-10, centerY+18);
                            printf("登录失败：无法读取用户信息！");
                        }
                    }
                    break;
                case '2': // 返回
                case 27:  // ESC键
                    running = 0;
                    break;
            }
        }
        
        // 处理鼠标事件
        int btnId = handleMouseEvent();
        
        // 处理输入框点击
        HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
        INPUT_RECORD inputBuffer;
        DWORD numEvents;
        
        // 检查是否有输入事件
        PeekConsoleInput(hInput, &inputBuffer, 1, &numEvents);
        if (numEvents > 0) {
            ReadConsoleInput(hInput, &inputBuffer, 1, &numEvents);
            
            // 处理鼠标事件
            if (inputBuffer.EventType == MOUSE_EVENT) {
                MOUSE_EVENT_RECORD mouseEvent = inputBuffer.Event.MouseEvent;
                int mouseX = mouseEvent.dwMousePosition.X;
                int mouseY = mouseEvent.dwMousePosition.Y;
                
                // 检查是否点击了账号输入框
                if (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
                    if (mouseY == centerY && mouseX >= (centerX-3)*2 && mouseX <= (centerX+12)*2) {
                        // 点击了账号输入框
                        inputFocus = 1;
                        
                        // 清除输入框内容
                        gotoxy(centerX-3, centerY);
                        for (int i = 0; i < 15; i++) printf(" ");
                        
                        // 设置光标位置
                        gotoxy(centerX-3, centerY);
                        
                        // 获取用户输入
                        getInput(username, sizeof(username), 0);
                        
                        inputFocus = 0;
                    }
                    // 检查是否点击了密码输入框
                    else if (mouseY == centerY+4 && mouseX >= (centerX-3)*2 && mouseX <= (centerX+12)*2) {
                        // 点击了密码输入框
                        inputFocus = 2;
                        
                        // 清除输入框内容
                        gotoxy(centerX-3, centerY+4);
                        for (int i = 0; i < 15; i++) printf(" ");
                        
                        // 设置光标位置
                        gotoxy(centerX-3, centerY+4);
                        
                        // 获取用户输入
                        getInput(password, sizeof(password), 1);
                        
                        inputFocus = 0;
                    }
                }
            }
        }
        
        // 处理按钮点击
        if (btnId == 1) { // 登录按钮
            // 检查用户名和密码是否为空
            if (strlen(username) == 0 || strlen(password) == 0) {
                setColor(COLOR_RED);
                gotoxy(centerX-10, centerY+18);
                printf("                                      ");
                gotoxy(centerX-10, centerY+18);
                printf("账号和密码不能为空，请重新输入！");
                continue;
            }
            
            // 检查用户是否存在
            if (!checkUserExists(username)) {
                loginResult = 0; // 登录失败，用户不存在
                setColor(COLOR_RED);
                gotoxy(centerX-10, centerY+18);
                printf("                                      ");
                gotoxy(centerX-10, centerY+18);
                printf("登录失败：用户不存在！");
            } else {
                // 读取用户信息
                UserInfo user;
                if (readUserFile(username, &user)) {
                    // 验证密码
                    if (strcmp(user.password, password) == 0) {
                        loginResult = 1; // 登录成功
                        
                        // 更新当前用户信息
                        memcpy(&currentUser, &user, sizeof(UserInfo));
                        isLoggedIn = 1;
                        
                        setColor(COLOR_GREEN);
                        gotoxy(centerX-10, centerY+18);
                        printf("                                      ");
                        gotoxy(centerX-10, centerY+18);
                        printf("登录成功！欢迎回来，%s！", username);
                        
                        // 短暂延时后返回
                        Sleep(1500);
                        running = 0;
                    } else {
                        loginResult = 0; // 登录失败，密码错误
                        setColor(COLOR_RED);
                        gotoxy(centerX-10, centerY+18);
                        printf("                                      ");
                        gotoxy(centerX-10, centerY+18);
                        printf("登录失败：密码错误！");
                    }
                } else {
                    loginResult = 0; // 登录失败，无法读取用户文件
                    setColor(COLOR_RED);
                    gotoxy(centerX-10, centerY+18);
                    printf("                                      ");
                    gotoxy(centerX-10, centerY+18);
                    printf("登录失败：无法读取用户信息！");
                }
            }
        } else if (btnId == 2) { // 返回按钮
            running = 0;
        }
        
        Sleep(50);
    }
}

// 更新用户得分
void updateUserScore() {
    // 如果未登录，不更新分数
    if (!isLoggedIn) return;
    
    // 根据当前难度更新对应的最高分
    switch(currentDifficulty) {
        case DIFFICULTY_EASY:
            if (score > currentUser.highScoreEasy) {
                currentUser.highScoreEasy = score;
            }
            break;
        case DIFFICULTY_NORMAL:
            if (score > currentUser.highScoreNormal) {
                currentUser.highScoreNormal = score;
            }
            break;
        case DIFFICULTY_HARD:
            if (score > currentUser.highScoreHard) {
                currentUser.highScoreHard = score;
            }
            break;
        case DIFFICULTY_HELL:
            if (score > currentUser.highScoreHell) {
                currentUser.highScoreHell = score;
            }
            break;
    }
    
    // 将更新后的用户信息写回文件
    char filePath[100];
    sprintf(filePath, "users/%s.dat", currentUser.username);
    
    FILE* file = fopen(filePath, "wb");
    if (file) {
        fwrite(&currentUser, sizeof(UserInfo), 1, file);
        fclose(file);
    }
}

// 读取排行榜数据
void loadRankings(RankingEntry* rankings, int* rankCount, int currentDifficultyView) {
    *rankCount = 0;
    
    // 打开users目录
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("users\\*.dat", &findFileData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // 读取用户文件
            UserInfo user;
            char filePath[100];
            sprintf(filePath, "users\\%s", findFileData.cFileName);
            
            FILE* file = fopen(filePath, "rb");
            if (file) {
                fread(&user, sizeof(UserInfo), 1, file);
                fclose(file);
                
                // 根据当前查看的难度选择对应的分数
                int userScore = 0;
                switch(currentDifficultyView) {
                    case DIFFICULTY_EASY:
                        userScore = user.highScoreEasy;
                        break;
                    case DIFFICULTY_NORMAL:
                        userScore = user.highScoreNormal;
                        break;
                    case DIFFICULTY_HARD:
                        userScore = user.highScoreHard;
                        break;
                    case DIFFICULTY_HELL:
                        userScore = user.highScoreHell;
                        break;
                }
                
                // 只添加有分数的用户
                if (userScore > 0) {
                    strcpy(rankings[*rankCount].username, user.username);
                    rankings[*rankCount].score = userScore;
                    (*rankCount)++;
                }
            }
            
            if (*rankCount >= 100) break; // 最多显示100个用户
            
        } while (FindNextFile(hFind, &findFileData));
        
        FindClose(hFind);
    }
    
    // 按分数排序（冒泡排序）
    for (int i = 0; i < *rankCount - 1; i++) {
        for (int j = 0; j < *rankCount - i - 1; j++) {
            if (rankings[j].score < rankings[j + 1].score) {
                // 交换
                RankingEntry temp = rankings[j];
                rankings[j] = rankings[j + 1];
                rankings[j + 1] = temp;
            }
        }
    }
}

// 绘制排行榜
void drawRankings(int centerX, int centerY, int currentDifficultyView) {
    RankingEntry rankings[100]; // 最多显示100个用户
    int rankCount = 0;
    
    system("cls");
    resetButtons();
    
    // 绘制界面边框
    drawScreenBorder(centerX-15, centerY-10, 52, 25);
    
    // 绘制标题
    setColor(COLOR_CYAN);
    gotoxy(centerX-4, centerY-8);
    printf("用户排行榜");
    
    // 显示当前查看的难度
    setColor(COLOR_WHITE);
    gotoxy(centerX-12, centerY-6);
    printf("当前难度: ");
    switch(currentDifficultyView) {
        case DIFFICULTY_EASY:
            setColor(COLOR_GREEN);
            printf("简单");
            break;
        case DIFFICULTY_NORMAL:
            setColor(COLOR_YELLOW);
            printf("普通");
            break;
        case DIFFICULTY_HARD:
            setColor(COLOR_PURPLE);
            printf("困难");
            break;
        case DIFFICULTY_HELL:
            setColor(COLOR_RED);
            printf("地狱");
            break;
    }
    
    // 创建难度切换和返回按钮
    createButton(centerX-13, centerY+8, 6, 3, "简单", 1);
    createButton(centerX-4, centerY+8, 6, 3, "普通", 2);
    createButton(centerX+5, centerY+8, 6, 3, "困难", 3);
    createButton(centerX-13, centerY+12, 6, 3, "地狱", 4);
    createButton(centerX+5, centerY+12, 6, 3, "返回", 5);
    
    // 绘制按钮
    drawAllButtons();
    
    // 加载排行榜数据
    loadRankings(rankings, &rankCount, currentDifficultyView);
    
    // 绘制排行榜表头
    setColor(COLOR_WHITE);
    gotoxy(centerX-12, centerY-4);
    printf("排名");
    gotoxy(centerX-6, centerY-4);
    printf("用户名");
    gotoxy(centerX+8, centerY-4);
    printf("分数");
    
    // 绘制分隔线
    gotoxy(centerX-12, centerY-3);
    for (int i = 0; i < 44; i++) printf("-");
    
    // 绘制排行榜数据
    int displayCount = rankCount < 5 ? rankCount : 5; // 最多显示5条
    for (int i = 0; i < displayCount; i++) {
        setColor(i == 0 ? COLOR_YELLOW : (i == 1 ? COLOR_CYAN : (i == 2 ? COLOR_PURPLE : COLOR_WHITE)));
        gotoxy(centerX-12, centerY-2+i);
        printf("%2d", i+1);
        gotoxy(centerX-6, centerY-2+i);
        printf("%-15s", rankings[i].username);
        gotoxy(centerX+8, centerY-2+i);
        printf("%6d", rankings[i].score);
    }
    
    // 如果没有数据，显示提示
    if (rankCount == 0) {
        setColor(COLOR_RED);
        gotoxy(centerX-10, centerY);
        printf("暂无该难度的排行数据");
    }
}

// 用户排行榜界面
void userRankingScreen() {
    system("cls");
    resetButtons();
    
    int centerX = 30;
    int centerY = 10;
    int running = 1;
    int currentDifficultyView = currentDifficulty; // 默认显示当前难度的排行
    
    // 初始化用户系统
    initUserSystem();
    
    // 初始绘制
    drawRankings(centerX, centerY, currentDifficultyView);
    
    while (running) {
        // 处理键盘输入
        if (kbhit()) {
            int key = getch();
            switch(key) {
                case '1': // 简单难度
                    currentDifficultyView = DIFFICULTY_EASY;
                    drawRankings(centerX, centerY, currentDifficultyView);
                    break;
                case '2': // 普通难度
                    currentDifficultyView = DIFFICULTY_NORMAL;
                    drawRankings(centerX, centerY, currentDifficultyView);
                    break;
                case '3': // 困难难度
                    currentDifficultyView = DIFFICULTY_HARD;
                    drawRankings(centerX, centerY, currentDifficultyView);
                    break;
                case '4': // 地狱难度
                    currentDifficultyView = DIFFICULTY_HELL;
                    drawRankings(centerX, centerY, currentDifficultyView);
                    break;
                case '5': // 返回
                case 27:  // ESC键
                    running = 0;
                    break;
            }
        }
        
        // 处理鼠标事件
        int btnId = handleMouseEvent();
        if (btnId > 0) {
            switch (btnId) {
                case 1: // 简单难度
                    currentDifficultyView = DIFFICULTY_EASY;
                    drawRankings(centerX, centerY, currentDifficultyView);
                    break;
                case 2: // 普通难度
                    currentDifficultyView = DIFFICULTY_NORMAL;
                    drawRankings(centerX, centerY, currentDifficultyView);
                    break;
                case 3: // 困难难度
                    currentDifficultyView = DIFFICULTY_HARD;
                    drawRankings(centerX, centerY, currentDifficultyView);
                    break;
                case 4: // 地狱难度
                    currentDifficultyView = DIFFICULTY_HELL;
                    drawRankings(centerX, centerY, currentDifficultyView);
                    break;
                case 5: // 返回
                    running = 0;
                    break;
            }
        }
        
        Sleep(50);
    }
}

int main() {
    // 设置控制台编码
    setUTF8();
    
    // 获取控制台窗口句柄
    HWND hWnd = GetConsoleWindow();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // 设置控制台字体
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 8;                  // 字体宽度
    cfi.dwFontSize.Y = 16;                 // 字体高度
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy(cfi.FaceName, L"Consolas");    // 使用Consolas字体
    SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);
    
    // 设置控制台窗口和缓冲区大小
    COORD bufferSize = {100, 40};          // 更合适的缓冲区大小
    SMALL_RECT windowSize = {0, 0, 99, 39}; // 更合适的窗口大小
    
    // 调整顺序很重要：先设置足够大的缓冲区，再设置窗口大小，最后调整缓冲区到最终大小
    COORD maxSize = GetLargestConsoleWindowSize(hConsole);
    COORD largeBuffer = {max(bufferSize.X, maxSize.X), max(bufferSize.Y, maxSize.Y)};
    SetConsoleScreenBufferSize(hConsole, largeBuffer);
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    
    // 禁用窗口关闭按钮和调整大小
    HMENU hmenu = GetSystemMenu(hWnd, FALSE);
    DeleteMenu(hmenu, SC_MAXIMIZE, MF_BYCOMMAND);
    DeleteMenu(hmenu, SC_SIZE, MF_BYCOMMAND);
    
    // 设置窗口样式
    LONG style = GetWindowLong(hWnd, GWL_STYLE);
    style &= ~(WS_MAXIMIZEBOX | WS_SIZEBOX);
    SetWindowLong(hWnd, GWL_STYLE, style);
    
    // 计算并设置窗口位置（居中）
    RECT rect;
    GetWindowRect(hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenWidth - width) / 2;
    int posY = (screenHeight - height) / 2;
    
    // 移动窗口到屏幕中央
    SetWindowPos(hWnd, NULL, posX, posY, width, height, SWP_NOZORDER);
    
    // 隐藏光标
    CONSOLE_CURSOR_INFO cursor_info = {1, 0};
    SetConsoleCursorInfo(hConsole, &cursor_info);
    
    // 初始化鼠标支持
    initMouse();
    
    // 初始化用户系统
    initUserSystem();
    
    // 菜单界面
    showStartScreen();
    while (1) {
        if (isMultiplayerMode) {
            // 双人模式
            initTwoPlayerGame();
            twoPlayerGameLoop();
            stopBackgroundMusic(); // 游戏结束停止背景音乐
            twoPlayerGameOverScreen(); // 显示游戏结束界面
        } else {
            // 单人模式
            initGame();
            gameLoop();
            stopBackgroundMusic(); // 游戏结束停止背景音乐
            singlePlayerGameOverScreen(); // 显示游戏结束界面
        }
    }
    
    // 确保退出时停止所有音乐
    stopBackgroundMusic();
    stopMenuMusic();
    
    return 0;
} 
