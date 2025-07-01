#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <string.h>
#include <mmsystem.h>  // 添加多媒体库头文件
#pragma comment(lib, "winmm.lib")  // 链接winmm库
#include "embedded_audio.h"

// 游戏区域定义
#define WIDTH 12
#define HEIGHT 20
#define BLOCK_SIZE 2

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

// 函数声明
// 基础功能
void setUTF8(); // 设置控制台编码为UTF-8，确保中文正常显示
void setColor(int color); // 设置控制台文本颜色，用于显示不同颜色的方块和文字
void gotoxy(int x, int y); // 移动光标到指定位置，用于在屏幕上精确绘制内容

// 游戏界面
void showStartScreen(); // 显示游戏开始界面，包含游戏模式选择和设置选项
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

// 移动光标到指定位置
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x * 2;  // 乘以2是因为每个方块占两个字符宽度
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 切换方块符号
void switchBlockSymbol() {
    currentBlockIndex = (currentBlockIndex + 1) % BLOCK_OPTION_COUNT;
}

// 难度选择界面
void difficultySelectScreen() {
    int running = 1;
    while (running) {
        system("cls");
        setColor(COLOR_CYAN);
        gotoxy(14, 6);
        printf("难度选择");
        
        setColor(COLOR_WHITE);
        gotoxy(10, 9);
        printf("1. 简单 (下落间隔: 750ms)");
        gotoxy(10, 11);
        printf("2. 普通 (下落间隔: 500ms)");
        gotoxy(10, 13);
        printf("3. 困难 (下落间隔: 250ms)");
        gotoxy(10, 15);
        printf("4. 地狱 (下落间隔: 100ms)");
        
        gotoxy(13, 17);
        printf("当前难度: ");
        switch(currentDifficulty) {
            case DIFFICULTY_EASY:   setColor(COLOR_GREEN);  printf("简单"); break;
            case DIFFICULTY_NORMAL: setColor(COLOR_YELLOW); printf("普通"); break;
            case DIFFICULTY_HARD:   setColor(COLOR_PURPLE); printf("困难"); break;
            case DIFFICULTY_HELL:   setColor(COLOR_RED);    printf("地狱"); break;
        }
        
        setColor(COLOR_GREEN);
        gotoxy(10, 20);
        printf("按1-4选择难度，按Q返回主菜单");
        
        int key = getch();
        if (key == '1')      currentDifficulty = DIFFICULTY_EASY;
        else if (key == '2') currentDifficulty = DIFFICULTY_NORMAL;
        else if (key == '3') currentDifficulty = DIFFICULTY_HARD;
        else if (key == '4') currentDifficulty = DIFFICULTY_HELL;
        else if (key == 'q' || key == 'Q') running = 0;
    }
}

// 方块样式更改界面
void styleChangeScreen() {
    int running = 1;
    while (running) {
        system("cls");
        setColor(COLOR_CYAN);
        gotoxy(10, 6);
        printf(" %s <--这是当前方块样式", BLOCK_OPTIONS[currentBlockIndex]);
        setColor(COLOR_WHITE);
        gotoxy(10, 10);
        printf("（按X键切换方块，按Q键退出，若显示为空，请按x进行样式更改）");
        int key = getch();
        if (key == 'x' || key == 'X') {
            switchBlockSymbol();
        } else if (key == 'q' || key == 'Q') {
            running = 0;
        }
    }
}

// 音效设置界面
void soundSettingsScreen() {
    int running = 1;
    while (running) {
        system("cls");
        setColor(COLOR_CYAN);
        gotoxy(14, 6);
        printf("音效设置");
        
        setColor(COLOR_WHITE);
        gotoxy(10, 9);
        printf("1. 背景音乐: %s", isMusicEnabled ? "开" : "关");
        gotoxy(10, 11);
        printf("2. 游戏音效: %s", isSoundEnabled ? "开" : "关");
        
        setColor(COLOR_GREEN);
        gotoxy(10, 14);
        printf("按1-2切换设置，按Q返回主菜单");
        
        int key = getch();
        if (key == '1') {
            isMusicEnabled = !isMusicEnabled;
            if (isMusicEnabled) {
                playMenuMusic();
            } else {
                stopMenuMusic();
                stopBackgroundMusic();
            }
        } else if (key == '2') {
            isSoundEnabled = !isSoundEnabled;
        } else if (key == 'q' || key == 'Q') {
            running = 0;
        }
    }
}

// 显示开始界面
void showStartScreen() {
    stopBackgroundMusic();
    playMenuMusic();
    
    while (1) {
        system("cls");
        setColor(COLOR_CYAN);
        gotoxy(13, 6);
        printf("俄罗斯方块--游戏");
        
        setColor(COLOR_WHITE);
        gotoxy(15, 9);  printf("开始游戏");
        gotoxy(15, 11); printf("双人模式");
        gotoxy(15, 13); printf("难度选择");
        
        // 显示当前难度
        gotoxy(28, 13);
        printf("(当前: ");
        switch(currentDifficulty) {
            case DIFFICULTY_EASY:   setColor(COLOR_GREEN);  printf("简单)"); break;
            case DIFFICULTY_NORMAL: setColor(COLOR_YELLOW); printf("普通)"); break;
            case DIFFICULTY_HARD:   setColor(COLOR_PURPLE); printf("困难)"); break;
            case DIFFICULTY_HELL:   setColor(COLOR_RED);    printf("地狱)"); break;
        }
        
        setColor(COLOR_WHITE);
        gotoxy(15, 15); printf("样式更改");
        gotoxy(15, 17); printf("音效设置");
        gotoxy(15, 19); printf("退出游戏");
        
        setColor(COLOR_GREEN);
        gotoxy(1, 22);
        printf("（1~6键进行选择，若为首次游戏，开始游戏前，请先按4查看方块是否显示正常）");
        
        int key = getch();
        if (key == '1') {
            isMultiplayerMode = 0;
            stopMenuMusic();
            break;
        } else if (key == '2') {
            isMultiplayerMode = 1;
            stopMenuMusic();
            break;
        } else if (key == '3') {
            difficultySelectScreen();
        } else if (key == '4') {
            styleChangeScreen();
        } else if (key == '5') {
            soundSettingsScreen();
        } else if (key == '6') {
            stopMenuMusic();
            exit(0);
        }
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
    setColor(COLOR_WHITE);
    // 上边框
    gotoxy(0, 0); printf("+");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("-");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("+");
    gotoxy(WIDTH+1, 0); printf("+");

    // 侧边框
    for(int i = 1; i <= HEIGHT; i++) {
        gotoxy(0, i); printf("|"); // 左侧边界（向右移动一个单位）
        gotoxy(WIDTH+1, i); printf("|"); // 游戏区中线
        gotoxy(WIDTH+1+WIDTH, i); printf("|"); // 信息区右边界
    }

    // 下边框
    gotoxy(0, HEIGHT+1); printf("+");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("-");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("+");
    gotoxy(WIDTH+1, HEIGHT+1); printf("+");
}

// 绘制游戏区域
void drawGameArea() {
    for(int i = 0; i < HEIGHT; i++) {
        for(int j = 0; j < WIDTH; j++) {
            gotoxy(j+1, i+1);
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
    setColor(rand()%6+1);
    gotoxy(WIDTH+5, 2);
    printf("下一个方块:");
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            gotoxy(WIDTH+5+j, i+4);
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
    setColor(COLOR_WHITE);
    gotoxy(WIDTH+5, 8);
    printf("当前分数: %d", score);
    gotoxy(WIDTH+5, 9);
    printf("最高分数: %d", highScore);
    gotoxy(WIDTH+5, 10);
    
    // 显示当前难度
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
    setColor(COLOR_WHITE);
    gotoxy(WIDTH+5, 12);
    printf("控制说明:");
    gotoxy(WIDTH+5, 13);
    printf("←→: 左右移动");
    gotoxy(WIDTH+5, 14);
    printf("↑: 旋转");
    gotoxy(WIDTH+5, 15);
    printf("空格: 快速下落");
    gotoxy(WIDTH+5, 16);
    printf("Z: 暂停");
    gotoxy(WIDTH+5, 17);
    printf("Q: 退出");
    gotoxy(WIDTH+5, 19);
    printf("注意: 按键有冷却");
    gotoxy(WIDTH+5, 20);
    printf("建议单击按键");
}

// 绘制暂停提示
void drawPauseMessage() {
    if(isPaused) {
        setColor(COLOR_YELLOW);
        gotoxy(WIDTH/2-5, HEIGHT/2);
        printf("游戏已暂停");
        gotoxy(WIDTH/2-9, HEIGHT/2+1);
        printf("按Z键继续游戏");
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
                        stopBackgroundMusic(); // 退出时停止背景音乐
                        break;
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
                            gotoxy(prevX+j+1, prevY+i+1);
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
                            gotoxy(currentX+j+1, currentY+i+1);
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
    setColor(COLOR_WHITE);
    
    // 玩家1区域
    // 上边框
    gotoxy(0, 0); printf("+");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("-");
    printf("+");
    
    // 侧边框
    for(int i = 1; i <= HEIGHT; i++) {
        gotoxy(0, i); printf("|");
        gotoxy(WIDTH+1, i); printf("|");
    }
    
    // 下边框
    gotoxy(0, HEIGHT+1); printf("+");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("-");
    printf("+");
    
    // 玩家2区域
    // 上边框
    gotoxy(WIDTH+3, 0); printf("+");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("-");
    printf("+");
    
    // 侧边框
    for(int i = 1; i <= HEIGHT; i++) {
        gotoxy(WIDTH+3, i); printf("|");
        gotoxy(WIDTH*2+4, i); printf("|");
    }
    
    // 下边框
    gotoxy(WIDTH+3, HEIGHT+1); printf("+");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("-");
    printf("+");
    
    // 信息区域边框
    // 上边框
    gotoxy(WIDTH*2+6, 0); printf("+");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("-");
    printf("+");
    
    // 侧边框
    for(int i = 1; i <= HEIGHT; i++) {
        gotoxy(WIDTH*2+6, i); printf("|");
        gotoxy(WIDTH*3+7, i); printf("|");
    }
    
    // 下边框
    gotoxy(WIDTH*2+6, HEIGHT+1); printf("+");
    for(int i = 0; i < WIDTH; i++) printf("--");
    printf("-");
    printf("+");
}

// 绘制双人模式游戏区域
void drawTwoPlayerGameArea() {
    // 绘制玩家1区域
    for(int i = 0; i < HEIGHT; i++) {
        for(int j = 0; j < WIDTH; j++) {
            gotoxy(j+1, i+1);
            if(gameArea[i][j]) {
                setColor(gameArea[i][j]);
                printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            } else {
                printf("  ");
            }
        }
    }
    
    // 绘制玩家2区域
    for(int i = 0; i < HEIGHT; i++) {
        for(int j = 0; j < WIDTH; j++) {
            gotoxy(WIDTH+4+j, i+1);
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
    // 玩家1下一个方块
    setColor(COLOR_CYAN);
    gotoxy(WIDTH*2+10, 2);
    printf("玩家1下一个:");
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            gotoxy(WIDTH*2+10+j, i+3);
            if(nextShape[i][j]) {
                setColor(nextColor);
                printf("%s", BLOCK_OPTIONS[currentBlockIndex]);
            } else {
                printf("  ");
            }
        }
    }
    
    // 玩家2下一个方块
    setColor(COLOR_RED);
    gotoxy(WIDTH*2+10, 6);
    printf("玩家2下一个:");
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            gotoxy(WIDTH*2+10+j, i+7);
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
    setColor(COLOR_WHITE);
    gotoxy(WIDTH*2+8, 11);
    printf("玩家1分数: %d", score);
    gotoxy(WIDTH*2+8, 12);
    printf("玩家2分数: %d", score2);
    gotoxy(WIDTH*2+8, 13);
    printf("最高分数: %d", highScore);
    gotoxy(WIDTH*2+8, 14);
    
    // 显示当前难度
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
    setColor(COLOR_WHITE);
    gotoxy(WIDTH*2+10, 16);
    printf("控制说明:");
    gotoxy(WIDTH*2+8, 17);
    printf("玩家1: A/D左右, W旋转");
    gotoxy(WIDTH*2+8, 18);
    printf("玩家2: ←→左右, ↑旋转");
    gotoxy(WIDTH*2+8, 19);
    printf("Z: 暂停, Q: 退出");
    gotoxy(WIDTH*2+8, 20);
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
    if(isPaused) {
        setColor(COLOR_YELLOW);
        gotoxy(WIDTH+WIDTH/2-2, HEIGHT/2);
        printf("游戏已暂停");
        gotoxy(WIDTH+WIDTH/2-6, HEIGHT/2+1);
        printf("按Z键继续游戏");
    }
}

// 双人游戏主循环
void twoPlayerGameLoop() {
    clock_t lastMove = clock();
    int prevX = currentX, prevY = currentY;
    int prevX2 = currentX2, prevY2 = currentY2;
    int needRedraw = 1;
    
    while(!(gameOver && gameOver2)) {
        needRedraw = 0;
        
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
                if(!isPaused) {
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
                        break;
                }
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
                            gotoxy(prevX+j+1, prevY+i+1);
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
                            gotoxy(WIDTH+4+prevX2+j, prevY2+i+1);
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
            drawTwoPlayerControls();
            
            // 绘制玩家1当前方块
            if(!isPaused && !gameOver) {
                setColor(currentColor);
                for(int i = 0; i < 4; i++) {
                    for(int j = 0; j < 4; j++) {
                        if(currentShape[i][j]) {
                            gotoxy(currentX+j+1, currentY+i+1);
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
                            gotoxy(WIDTH+4+currentX2+j, currentY2+i+1);
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

int main() {
    // 设置控制台编码
    setUTF8();
    // 设置控制台窗口大小 - 扩大窗口以适应双人模式
    system("mode con cols=120 lines=30");
    // 隐藏光标
    CONSOLE_CURSOR_INFO cursor_info = {1, 0};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

    // 菜单界面
    showStartScreen();
    while (1) {
        if (isMultiplayerMode) {
            // 双人模式
            initTwoPlayerGame();
            twoPlayerGameLoop();
            stopBackgroundMusic(); // 游戏结束停止背景音乐
            system("cls");
            gotoxy(15, 10);
            if (gameOver && gameOver2) {
                playSound(SOUND_GAMEOVER);
                if (score > score2) {
                    printf("游戏结束！玩家1获胜！");
                } else if (score2 > score) {
                    printf("游戏结束！玩家2获胜！");
                } else {
                    printf("游戏结束！平局！");
                }
            } else if (gameOver) {
                printf("玩家1失败，游戏继续...");
            } else {
                printf("玩家2失败，游戏继续...");
            }
            
            gotoxy(15, 12); printf("玩家1得分：%d", score);
            gotoxy(15, 13); printf("玩家2得分：%d", score2);
            gotoxy(15, 14); printf("最高分纪录：%d", highScore);
            gotoxy(15, 16);
            printf("按任意键重新开始，Q键退至菜单，Esc键退出游戏");
        } else {
            // 单人模式
            initGame();
            gameLoop();
            stopBackgroundMusic(); // 游戏结束停止背景音乐
            system("cls");
            gotoxy(15, 10); printf("游戏结束！最终得分：%d", score);
            gotoxy(15, 11); printf("最高分纪录：%d", highScore);
            gotoxy(15, 13);
            printf("按任意键重新开始，Q键退至菜单，Esc键退出游戏");
        }
        
        int key = getch();
        if (key == 'q' || key == 'Q') {
            playMenuMusic(); // 返回菜单时播放菜单音乐
            showStartScreen(); // 返回菜单
        } else if (key == 27) { // ESC键
            break; // 退出游戏
        }
    }
    
    // 确保退出时停止所有音乐
    stopBackgroundMusic();
    stopMenuMusic();
    
    return 0;
} 
