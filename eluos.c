#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include <string.h>
// 定义游戏区域大小
#define WIDTH 12
#define HEIGHT 20
#define BLOCK_SIZE 2
// 定义方块显示符号，初始为"■"，后续可切换为多种样式
const char* BLOCK_OPTIONS[] = {"■", "██", "[]", "口","▣", "▤", "▥", "▦", "▧", "▨", "▩"};
#define BLOCK_OPTION_COUNT (sizeof(BLOCK_OPTIONS)/sizeof(BLOCK_OPTIONS[0]))
int currentBlockIndex = 0; // 当前选中的方块样式索引

// 定义颜色
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_PURPLE 5
#define COLOR_YELLOW 6
#define COLOR_WHITE 7

// 按键冷却时间（毫秒）
#define KEY_COOLDOWN 200
clock_t lastKeyTime = 0; // 上次按键时间

// 难度级别
#define DIFFICULTY_EASY 1
#define DIFFICULTY_NORMAL 2
#define DIFFICULTY_HARD 3
#define DIFFICULTY_HELL 4

// 不同难度的初始下落间隔（毫秒）
#define SPEED_EASY 750
#define SPEED_NORMAL 500
#define SPEED_HARD 250
#define SPEED_HELL 100

int currentDifficulty = DIFFICULTY_NORMAL; // 默认难度为普通

// 定义方块形状
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
void initGame();                   // 初始化游戏
void drawBorder();                 // 绘制边框
void drawGameArea();               // 绘制游戏区域
void drawNextShape();              // 绘制下一个方块
void drawScore();                  // 绘制分数
void drawControls();               // 绘制控制说明
void generateNewShape();           // 生成新方块
void rotateShape();                // 旋转方块
int canMove(int x, int y);         // 检查是否可以移动
void moveDown();                   // 方块下落
void clearLines();                 // 清除已满行
void gameLoop();                   // 游戏主循环
void showStartScreen();            // 显示开始界面
void drawPauseMessage();           // 绘制暂停提示
int canMoveShape(int shape[4][4], int x, int y);  // 检查任意形状在指定位置是否可以移动（用于旋转判定）
void styleChangeScreen();           // 方块样式更改界面
void switchBlockSymbol();            // 切换方块符号
int getSpeedByScore();             // 根据分数获取下落速度
void setUTF8();                    // 设置UTF-8编码
void setColor(int color);          // 设置颜色
void gotoxy(int x, int y);         // 移动光标到指定位置
void difficultySelectScreen();     // 难度选择界面

// 双人模式函数声明
void initTwoPlayerGame();          // 初始化双人游戏
void drawTwoPlayerBorder();        // 绘制双人模式边框
void drawTwoPlayerGameArea();      // 绘制双人模式游戏区域
void drawTwoPlayerNextShape();     // 绘制双人模式下一个方块
void drawTwoPlayerScore();         // 绘制双人模式分数
void drawTwoPlayerControls();      // 绘制双人模式控制说明
void generateNewShape2();          // 为玩家2生成新方块
void rotateShape2();               // 玩家2旋转方块
int canMove2(int x, int y);        // 检查玩家2是否可以移动
void moveDown2();                  // 玩家2方块下落
void clearLines2();                // 清除玩家2已满行
void twoPlayerGameLoop();          // 双人游戏主循环
void drawTwoPlayerPauseMessage();  // 绘制双人模式暂停提示
int canMoveShape2(int shape[4][4], int x, int y);  // 检查玩家2任意形状在指定位置是否可以移动

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
        
        setColor(COLOR_GREEN);
        gotoxy(10, 20);
        printf("按1-4选择难度，按Q返回主菜单");
        
        int key = getch();
        if (key == '1') {
            currentDifficulty = DIFFICULTY_EASY;
        } else if (key == '2') {
            currentDifficulty = DIFFICULTY_NORMAL;
        } else if (key == '3') {
            currentDifficulty = DIFFICULTY_HARD;
        } else if (key == '4') {
            currentDifficulty = DIFFICULTY_HELL;
        } else if (key == 'q' || key == 'Q') {
            running = 0;
        }
    }
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
    coord.X = x *2;  // 乘以2是因为每个方块占两个字符宽度
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 显示开始界面（菜单样式）
void showStartScreen() {
    int menuSelect = 0;
    while (1) {
        system("cls");
        setColor(COLOR_CYAN);
        gotoxy(13, 6);
        printf("俄罗斯方块--游戏");
        setColor(COLOR_WHITE);
        gotoxy(15, 9);
        printf("开始游戏");
        gotoxy(15, 11);
        printf("双人模式");
        gotoxy(15, 13);
        printf("难度选择");
        
        // 显示当前难度
        gotoxy(28, 13);
        printf("(当前: ");
        switch(currentDifficulty) {
            case DIFFICULTY_EASY:
                printf("简单)");
                break;
            case DIFFICULTY_NORMAL:
                printf("普通)");
                break;
            case DIFFICULTY_HARD:
                printf("困难)");
                break;
            case DIFFICULTY_HELL:
                printf("地狱)");
                break;
        }
        
        gotoxy(15, 15);
        printf("样式更改");
        gotoxy(15, 17);
        printf("退出游戏");
        setColor(COLOR_GREEN);
        gotoxy(1, 22);
        printf("（1~5键进行选择，若为首次游戏，开始游戏前，请先按4查看方块是否显示正常）");
        int key = getch();
        if (key == '1') {
            menuSelect = 1;
            isMultiplayerMode = 0;
            break;
        } else if (key == '2') {
            menuSelect = 2;
            isMultiplayerMode = 1;
            break;
        } else if (key == '3') {
            difficultySelectScreen();
        } else if (key == '4') {
            styleChangeScreen();
        } else if (key == '5') {
            exit(0);
        }
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
    gotoxy(WIDTH+5, 10);
    printf("当前分数: %d", score);
    gotoxy(WIDTH+5, 11);
    printf("最高分数: %d", highScore);
    gotoxy(WIDTH+5, 12);
    
    // 显示当前难度
    printf("当前难度: ");
    switch(currentDifficulty) {
        case DIFFICULTY_EASY:
            printf("简单");
            break;
        case DIFFICULTY_NORMAL:
            printf("普通");
            break;
        case DIFFICULTY_HARD:
            printf("困难");
            break;
        case DIFFICULTY_HELL:
            printf("地狱");
            break;
    }
}

// 绘制控制说明
void drawControls() {
    setColor(COLOR_WHITE);
    gotoxy(WIDTH+5, 13);
    printf("控制说明:");
    gotoxy(WIDTH+5, 14);
    printf("←→: 左右移动");
    gotoxy(WIDTH+5, 15);
    printf("↑: 旋转");
    gotoxy(WIDTH+5, 16);
    printf("空格: 快速下落");
    gotoxy(WIDTH+5, 17);
    printf("Z: 暂停");
    gotoxy(WIDTH+5, 18);
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

// 生成新方块
void generateNewShape() {
    // 把 nextShape 赋值给 currentShape
    memcpy(currentShape, nextShape, sizeof(currentShape));
    currentX = WIDTH/2 - 2;
    currentY = 0;
    currentColor = nextColor; // 切换颜色
    // 生成新的 nextShape 和 nextColor
    int shapeIndex = rand() % 7;
    memcpy(nextShape, shapes[shapeIndex], sizeof(nextShape));
    nextColor = 1 + rand() % 7; // 1~7，避免黑色
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

// 检查任意形状在指定位置是否可以移动（用于旋转判定）
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

// 旋转方块
void rotateShape() {
    int temp[4][4];
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            temp[i][j] = currentShape[3-j][i];
    
    // 只有旋转后不越界且不重叠才允许旋转
    if(canMoveShape(temp, currentX, currentY)) {
        memcpy(currentShape, temp, sizeof(currentShape));
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
                if(currentShape[i][j]) {
                    if(currentY + i >= 0) {
                        gameArea[currentY + i][currentX + j] = currentColor;
                    }
                }
            }
        }
        clearLines();
        generateNewShape();
        if(!canMove(currentX, currentY)) {
            gameOver = 1;
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
            for(int k = i; k > 0; k--) {
                for(int j = 0; j < WIDTH; j++) {
                    gameArea[k][j] = gameArea[k-1][j];
                }
            }
            for(int j = 0; j < WIDTH; j++) {
                gameArea[0][j] = 0;
            }
            i++;
        }
    }
    score += linesCleared * 10;
    if(score > highScore) {
        highScore = score;
    }
}

// 根据分数获取下落速度
int getSpeedByScore() {
    int baseSpeed;
    
    // 根据难度级别设置基础速度
    switch(currentDifficulty) {
        case DIFFICULTY_EASY:
            baseSpeed = SPEED_EASY;
            break;
        case DIFFICULTY_NORMAL:
            baseSpeed = SPEED_NORMAL;
            break;
        case DIFFICULTY_HARD:
            baseSpeed = SPEED_HARD;
            break;
        case DIFFICULTY_HELL:
            baseSpeed = SPEED_HELL;
            break;
        default:
            baseSpeed = SPEED_NORMAL;
    }
    
    // 随着分数增加，速度逐渐加快
    double speed = 1.0 + 9.0 * (score / 10000.0); // 1倍到10倍线性增长
    return (int)(baseSpeed / speed);
}

// 切换方块符号
void switchBlockSymbol() {
    currentBlockIndex = (currentBlockIndex + 1) % BLOCK_OPTION_COUNT;
}

// 方块样式更改界面
void styleChangeScreen() {
    system("cls");
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

// 初始化游戏
void initGame() {
    system("cls");
    srand(time(NULL));
    score =0;
    gameOver = 0;
    isPaused = 0;
    memset(gameArea, 0, sizeof(gameArea));
    // 先生成 nextShape
    int shapeIndex = rand() % 7;
    memcpy(nextShape, shapes[shapeIndex], sizeof(nextShape));
    // 生成第一个 currentShape
    generateNewShape();
}

// 游戏主循环
void gameLoop() {
    clock_t lastMove = clock();
    int prevX = currentX, prevY = currentY;
    int needRedraw = 1; // 是否需要重绘
    
    while(!gameOver) {
        if(score >= 10000) {
            gameOver = 1;
            system("cls");
            gotoxy(15, 10);
            printf("你赢了！最终得分：%d", score);
            gotoxy(15, 11);
            printf("最高分纪录：%d", highScore);
            gotoxy(15, 13);
            printf("按任意键退出...");
            getch();
            break;
        }
        
        needRedraw = 0; // 默认不需要重绘
        
        if(kbhit()) {
            // 检查按键冷却时间
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
                        case 72:  // 上箭头
                            needRedraw = 1;
                            rotateShape();
                            break;
                        case 80:  // 下箭头
                            if(canMove(currentX, currentY + 1)) {
                                needRedraw = 1;
                                currentY++;
                            }
                            break;
                        case 75:  // 左箭头
                            if(canMove(currentX-1, currentY)) {
                                needRedraw = 1;
                                currentX--;
                            }
                            break;
                        case 77:  // 右箭头
                            if(canMove(currentX+1, currentY)) {
                                needRedraw = 1;
                                currentX++;
                            }
                            break;
                    }
                }
            } else {
                // 处理其他按键
                switch(key) {
                    case 'a':
                    case 'A':
                        if(!isPaused && canMove(currentX-1, currentY)) {
                            needRedraw = 1;
                            currentX--;
                        }
                        break;
                    case 'd':
                    case 'D':
                        if(!isPaused && canMove(currentX+1, currentY)) {
                            needRedraw = 1;
                            currentX++;
                        }
                        break;
                    case 'w':
                    case 'W':
                        if(!isPaused) {
                            needRedraw = 1;
                            rotateShape();
                        }
                        break;
                    case 's':
                    case 'S':
                        if(!isPaused && canMove(currentX, currentY + 1)) {
                            needRedraw = 1;
                            currentY++;
                        }
                        break;
                    case ' ':
                        if(!isPaused) {
                            needRedraw = 1;
                            while(canMove(currentX, currentY+1)) currentY++;
                        }
                        break;
                    case 'z':
                    case 'Z':
                        needRedraw = 1;
                        isPaused = !isPaused;
                        break;
                    case 'q':
                    case 'Q':
                        gameOver = 1;
                        break;
                }
            }
        }
        
        if(!isPaused) {
            if(clock() - lastMove > getSpeedByScore()) {
                needRedraw = 1;
                moveDown();
                lastMove = clock();
            }
        }
        
        // 只有在需要重绘时才绘制
        if(needRedraw || prevX != currentX || prevY != currentY) {
            // 如果位置变化，先擦除旧位置
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
        
        // 如果游戏暂停，显示暂停信息
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
    
    // 为玩家1生成方块
    int shapeIndex = rand() % 7;
    memcpy(nextShape, shapes[shapeIndex], sizeof(nextShape));
    generateNewShape();
    
    // 为玩家2生成方块
    shapeIndex = rand() % 7;
    memcpy(nextShape2, shapes[shapeIndex], sizeof(nextShape2));
    generateNewShape2();
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
            printf("简单");
            break;
        case DIFFICULTY_NORMAL:
            printf("普通");
            break;
        case DIFFICULTY_HARD:
            printf("困难");
            break;
        case DIFFICULTY_HELL:
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
        // 固定当前方块
        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                if(currentShape2[i][j]) {
                    if(currentY2 + i >= 0) {
                        gameArea2[currentY2 + i][currentX2 + j] = currentColor2;
                    }
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
            for(int k = i; k > 0; k--) {
                for(int j = 0; j < WIDTH; j++) {
                    gameArea2[k][j] = gameArea2[k-1][j];
                }
            }
            for(int j = 0; j < WIDTH; j++) {
                gameArea2[0][j] = 0;
            }
            i++;
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
        
        if(kbhit()) {
            // 检查按键冷却时间
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
                    case 'a':
                    case 'A':  // 玩家1左移
                        if(!isPaused && !gameOver && canMove(currentX-1, currentY)) {
                            needRedraw = 1;
                            currentX--;
                        }
                        break;
                    case 'd':
                    case 'D':  // 玩家1右移
                        if(!isPaused && !gameOver && canMove(currentX+1, currentY)) {
                            needRedraw = 1;
                            currentX++;
                        }
                        break;
                    case 'w':
                    case 'W':  // 玩家1旋转
                        if(!isPaused && !gameOver) {
                            needRedraw = 1;
                            rotateShape();
                        }
                        break;
                    case 's':
                    case 'S':  // 玩家1下移
                        if(!isPaused && !gameOver && canMove(currentX, currentY + 1)) {
                            needRedraw = 1;
                            currentY++;
                        }
                        break;
                    case 'z':
                    case 'Z':  // 暂停
                        needRedraw = 1;
                        isPaused = !isPaused;
                        break;
                    case 'q':
                    case 'Q':  // 退出
                        gameOver = 1;
                        gameOver2 = 1;
                        break;
                }
            }
        }
        
        if(!isPaused) {
            if(clock() - lastMove > getSpeedByScore()) {
                needRedraw = 1;
                if(!gameOver) moveDown();   // 玩家1方块下落
                if(!gameOver2) moveDown2();  // 玩家2方块下落
                lastMove = clock();
            }
        }
        
        // 只有在需要重绘时才绘制
        if(needRedraw || prevX != currentX || prevY != currentY || prevX2 != currentX2 || prevY2 != currentY2) {
            // 如果位置变化，先擦除旧位置
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
            
            // 绘制当前方块 - 玩家1
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
            
            // 绘制当前方块 - 玩家2
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
        
        // 如果游戏暂停，显示暂停信息
        drawTwoPlayerPauseMessage();
        
        Sleep(75);
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
            system("cls");
            gotoxy(15, 10);
            if (gameOver && gameOver2) {
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
            gotoxy(15, 12);
            printf("玩家1得分：%d", score);
            gotoxy(15, 13);
            printf("玩家2得分：%d", score2);
            gotoxy(15, 14);
            printf("最高分纪录：%d", highScore);
            gotoxy(15, 16);
            printf("按任意键重新开始，Q键退至菜单，esc键退出游戏");
        } else {
            // 单人模式
            initGame();
            gameLoop();
            system("cls");
            gotoxy(15, 10);
            printf("游戏结束！最终得分：%d", score);
            gotoxy(15, 11);
            printf("最高分纪录：%d", highScore);
            gotoxy(15, 13);
            printf("按任意键重新开始，Q键退至菜单，esc键退出游戏");
        }
        
        int key = getch();
        if (key == 'q' || key == 'Q') {
            showStartScreen(); // 返回菜单
        } else if (key == 27) { // ESC键
            break; // 退出游戏
        }
    }
    return 0;
} 
