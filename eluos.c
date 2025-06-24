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
// 定义方块显示符号，初始为“■”，后续可切换为“██”
char BLOCK[8] = "■";

// 定义颜色
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_PURPLE 5
#define COLOR_YELLOW 6
#define COLOR_WHITE 7

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
void checkBlockSymbol();            // 方块符号自适应检测

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

// 显示开始界面
void showStartScreen() {
    system("cls");
    setColor(COLOR_CYAN);
    gotoxy(15, 10);
    printf("俄罗斯方块");
    gotoxy(15, 12);
    printf("按任意键开始");
    getch();
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
        gotoxy(0, i); printf("|"); // 左
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
                setColor(COLOR_CYAN);
                printf(BLOCK);
            } else {
                printf("  ");
            }
        }
    }
}

// 绘制下一个方块
void drawNextShape() {
    setColor(COLOR_WHITE);
    gotoxy(WIDTH+5, 2);
    printf("下一个方块:");
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            gotoxy(WIDTH+5+j, i+4);
            if(nextShape[i][j]) {
                setColor(COLOR_YELLOW);
                printf(BLOCK);
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
    // 生成新的 nextShape
    int shapeIndex = rand() % 7;
    memcpy(nextShape, shapes[shapeIndex], sizeof(nextShape));
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
                        gameArea[currentY + i][currentX + j] = 1;
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
    score += linesCleared * 100;
    if(score > highScore) {
        highScore = score;
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
    // 先生成 nextShape
    int shapeIndex = rand() % 7;
    memcpy(nextShape, shapes[shapeIndex], sizeof(nextShape));
    // 生成第一个 currentShape
    generateNewShape();
}

// 游戏主循环
void gameLoop() {
    clock_t lastMove = clock();
    
    while(!gameOver) {
        if(kbhit()) {
            int key = getch();
            // 处理方向键
            if(key == 224) {  // 方向键的第一个字符
                key = getch();  // 获取方向键的第二个字符
                if(!isPaused) {  // 只有在非暂停状态才处理方向键
                    switch(key) {
                        case 72:  // 上箭头
                            rotateShape();
                            break;
                        case 80:  // 下箭头
                            if(canMove(currentX, currentY + 1)) currentY++;
                            break;
                        case 75:  // 左箭头
                            if(canMove(currentX-1, currentY)) currentX--;
                            break;
                        case 77:  // 右箭头
                            if(canMove(currentX+1, currentY)) currentX++;
                            break;
                    }
                }
            } else {
                // 处理其他按键
                switch(key) {
                    case 'a':
                    case 'A':
                        if(!isPaused && canMove(currentX-1, currentY)) currentX--;
                        break;
                    case 'd':
                    case 'D':
                        if(!isPaused && canMove(currentX+1, currentY)) currentX++;
                        break;
                    case 'w':
                    case 'W':
                        if(!isPaused) rotateShape();
                        break;
                    case ' ':
                        if(!isPaused) {
                            while(canMove(currentX, currentY+1)) currentY++;
                        }
                        break;
                    case 'z':
                    case 'Z':
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
            if(clock() - lastMove > 500) {
                moveDown();
                lastMove = clock();
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
            setColor(COLOR_CYAN);
            for(int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    if(currentShape[i][j]) {
                        gotoxy(currentX+j+1, currentY+i+1);
                        printf(BLOCK);
                    }
                }
            }
        }
        
        // 如果游戏暂停，显示暂停信息
        drawPauseMessage();
        
        Sleep(50);
    }
}

// 方块符号自适应检测
void checkBlockSymbol() {
    system("cls");
    printf("方块显示测试：\n\n");
    printf("  ");
    printf(BLOCK);
    printf("  ← 这是方块，显示正常吗？\n\n");
    printf("如果正常，请按Y或y，否则按任意键切换为兼容符号。\n");
    int key = getch();
    if (key != 'y' && key != 'Y') {
        strcpy(BLOCK, "██");
        printf("\n已切换为兼容符号。按任意键继续...\n");
        getch();
    }
}

int main() {
    // 设置控制台编码
    setUTF8();
    
    // 设置控制台窗口大小
    system("mode con cols=80 lines=25");
    // 隐藏光标
    CONSOLE_CURSOR_INFO cursor_info = {1, 0};
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

    // 检查方块符号兼容性
    checkBlockSymbol();

    showStartScreen();
    while (1) {
        initGame();
        gameLoop();

        system("cls");
        gotoxy(15, 10);
        printf("游戏结束！最终得分：%d", score);
        gotoxy(15, 11);
        printf("最高分纪录：%d", highScore);
        gotoxy(15, 13);
        printf("按任意键重新开始，Q键退出...");
        int key = getch();
        if (key == 'q' || key == 'Q') {
            break;
        }
    }
    return 0;
} 
