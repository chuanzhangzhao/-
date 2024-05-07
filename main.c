#include<stdio.h>
#include<time.h>
#include<windows.h>
#include<stdlib.h>

#define U 1
#define D 2
#define L 3
#define R 4       //蛇的状态，U：上 ；D：下；L:左 R：右


#define MAX_USERS 10
#define USERNAME_LENGTH 20
#define PASSWORD_LENGTH 20

// 游戏日志数据结构
typedef struct GameLog
{
    int userId;
    char username[USERNAME_LENGTH];
    time_t startTime;
    double duration;
    int score;
} GameLog;

typedef struct SNAKE //蛇身的一个节点
{
    int x;
    int y;
    struct SNAKE* next;
} snake;



typedef struct User
{
    int id;
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
} User;



//全局变量//
char storeUsername[20];
char storePassword[20];
int score = 0, add = 10;//总得分与每次吃食物得分。
int status, sleeptime = 200;//每次运行的时间间隔
snake* head, * food;//蛇头指针，食物指针
snake* q;//遍历蛇的时候用到的指针
int endgamestatus = 0; //游戏结束的情况，1：撞到墙；2：咬到自己；3：主动退出游戏。
GameLog logs[100]; // 游戏日志表
int numLogs = 0; // 用户计数

//声明全部函数//
void Pos();
void creatMap();
void initsnake();
int biteself();
void createfood();
void cantcrosswall();
void snakemove();
void pause();
void gamecircle();
void welcometogame();
void endgame();
void gamestart();

void registerUser();
void saveUsers();

User users[MAX_USERS];
int numUsers = 0;

void registerUser()
{
    if (numUsers >= MAX_USERS)
    {
        printf("无法继续注册新用户，达到最大用户数限制\n");
        return;
    }

    users[numUsers].id = users[numUsers-1].id+1;
    printf("请输入用户名: ");
    scanf("%s", users[numUsers].username);
    strcpy(storeUsername, users[numUsers].username);
    printf("请输入密码: ");
    scanf("%s", users[numUsers].password);
    numUsers++;
    saveUsers();

    printf("注册成功！\n");

}
void loadUsers()
{
    FILE *file = fopen("users.txt", "r");
    if (file != NULL)
    {
        while (fscanf(file, "%d %s %s", &users[numUsers].id, users[numUsers].username, users[numUsers].password) != EOF)
        {
            numUsers++;
        }
        fclose(file);
    }
}

void saveUsers()
{
    FILE *file = fopen("users.txt", "w");
    for (int i = 0; i < numUsers; i++)
    {
        fprintf(file, "%d %s %s\n", users[i].id, users[i].username, users[i].password);
    }
    fclose(file);
}

void recordGameLog(const char* username, double duration, int score)
{
    GameLog newLog;
    strcpy(newLog.username, username);
    newLog.duration = duration;
    newLog.score = score;

    logs[numLogs++] = newLog;

    FILE *file = fopen("game_logs.txt", "w");
    for (int i = 0; i < numUsers; i++)
    {
        fprintf(file, "%s %2.f %d\n",  newLog.username, newLog.duration, newLog.score);

    }
    fclose(file);
}


int verifyUser()
{
    char inputUsername[USERNAME_LENGTH];
    char inputPassword[PASSWORD_LENGTH];

    printf("请输入用户名: ");
    scanf("%s", inputUsername);
    printf("请输入密码: ");
    scanf("%s", inputPassword);

    for (int i = 0; i < numUsers; i++)
    {
        if (strcmp(users[i].username, inputUsername) == 0 && strcmp(users[i].password, inputPassword) == 0)
        {
            return 1; // 验证通过
        }
    }

    return 0; // 验证失败
}


void Pos(int x, int y)//设置光标位置
{
    COORD pos;
    HANDLE hOutput;
    pos.X = x;
    pos.Y = y;
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hOutput, pos);
}

void creatMap()//创建地图
{
    int i;
    for (i = 0; i < 58; i += 2)//打印上下边框
    {
        Pos(i, 0);
        printf("■");
        Pos(i, 26);
        printf("■");
    }
    for (i = 1; i < 26; i++)//打印左右边框
    {
        Pos(0, i);
        printf("■");
        Pos(56, i);
        printf("■");
    }
}

void initsnake()//初始化蛇身
{
    snake* tail;
    int i;
    tail = (snake*)malloc(sizeof(snake));//从蛇尾开始，头插法，以x,y设定开始的位置//
    tail->x = 24;
    tail->y = 5;
    tail->next = NULL;
    for (i = 1; i <= 4; i++)
    {
        head = (snake*)malloc(sizeof(snake));
        head->next = tail;
        head->x = 24 + 2 * i;
        head->y = 5;
        tail = head;
    }
    while (tail != NULL)//从头到为，输出蛇身
    {
        Pos(tail->x, tail->y);
        printf("■");
        tail = tail->next;
    }
}

int biteself()//判断是否咬到了自己
{
    snake* self;
    self = head->next;
    while (self != NULL)
    {
        if (self->x == head->x && self->y == head->y)
        {
            return 1;
        }
        self = self->next;
    }
    return 0;
}

void createfood()//随机出现食物
{
    int food_x;
    int food_y;
    snake* food_1;
    srand((unsigned)time(NULL));

    do
    {
        food_x = rand() % 54 + 2; // 生成 x 坐标
        food_y = rand() % 24 + 1; // 生成 y 坐标

        q = head;
        while (q != NULL)
        {
            if (q->x == food_x && q->y == food_y) // 判断食物是否与蛇身重合
            {
                break;
            }
            q = q->next;
        }

    }
    while (q != NULL || food_x % 2 != 0 || food_x == 0 || food_x == 56 || food_y == 0 || food_y == 26);

    food_1 = (snake*)malloc(sizeof(snake));
    food_1->x = food_x;
    food_1->y = food_y;

    Pos(food_1->x, food_1->y);
    food = food_1;
    printf("■");
}




void cantcrosswall()//不能穿墙
{
    if (head->x == 0 || head->x == 56 || head->y == 0 || head->y == 26)
    {
        endgamestatus = 1;
        endgame();
    }
}

void snakemove()//蛇前进,上U,下D,左L,右R
{
    snake* nexthead;
    cantcrosswall();

    nexthead = (snake*)malloc(sizeof(snake));
    if (status == U)
    {
        nexthead->x = head->x;
        nexthead->y = head->y - 1;
        if (nexthead->x == food->x && nexthead->y == food->y)//如果下一个有食物//
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            score = score + add;
            createfood();
        }
        else                                               //如果没有食物//
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (status == D)
    {
        nexthead->x = head->x;
        nexthead->y = head->y + 1;
        if (nexthead->x == food->x && nexthead->y == food->y)  //有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            score = score + add;
            createfood();
        }
        else                               //没有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (status == L)
    {
        nexthead->x = head->x - 2;
        nexthead->y = head->y;
        if (nexthead->x == food->x && nexthead->y == food->y)//有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            score = score + add;
            createfood();
        }
        else                                //没有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (status == R)
    {
        nexthead->x = head->x + 2;
        nexthead->y = head->y;
        if (nexthead->x == food->x && nexthead->y == food->y)//有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            score = score + add;
            createfood();
        }
        else                                         //没有食物
        {
            nexthead->next = head;
            head = nexthead;
            q = head;
            while (q->next->next != NULL)
            {
                Pos(q->x, q->y);
                printf("■");
                q = q->next;
            }
            Pos(q->next->x, q->next->y);
            printf("  ");
            free(q->next);
            q->next = NULL;
        }
    }
    if (biteself() == 1)       //判断是否会咬到自己
    {
        endgamestatus = 2;
        endgame();
    }
}

void pause()//暂停
{
    while (1)
    {
        Sleep(300);
        if (GetAsyncKeyState(VK_SPACE))
        {
            break;
        }

    }
}

void gamecircle()//控制游戏
{
    Pos(64, 14);
    printf("*******%s正在游戏中********\n",storeUsername);
    Pos(64, 15);
    printf("不能穿墙，不能咬到自己\n");
    Pos(64, 16);
    printf("用↑.↓.←.→分别控制蛇的移动.");
    Pos(64, 17);
    printf("F1 为加速，F2 为减速\n");
    Pos(64, 18);
    printf("F5为显示日志信息\n");
    Pos(64, 18);
    printf("ESC ：退出游戏.space：暂停游戏.");
    Pos(64, 20);
    status = R;
    while (1)
    {
        Pos(64, 10);
        printf("得分：%d  ", score);
        Pos(64, 11);
        printf("每个食物得分：%d分", add);
        if (GetAsyncKeyState(VK_UP) && status != D)
        {
            status = U;
        }
        else if (GetAsyncKeyState(VK_DOWN) && status != U)
        {
            status = D;
        }
        else if (GetAsyncKeyState(VK_LEFT) && status != R)
        {
            status = L;
        }
        else if (GetAsyncKeyState(VK_RIGHT) && status != L)
        {
            status = R;
        }
        else if (GetAsyncKeyState(VK_SPACE))
        {
            pause();
        }
        else if (GetAsyncKeyState(VK_ESCAPE))
        {
            endgamestatus = 3;
            break;
        }
        else if (GetAsyncKeyState(VK_F1))
        {
            if (sleeptime >= 50)
            {
                sleeptime = sleeptime - 30;
                add = add + 2;
                if (sleeptime == 320)
                {
                    add = 2;//防止减到1之后再加回来有错
                }
            }
        }
        else if (GetAsyncKeyState(VK_F2))
        {
            if (sleeptime < 350)
            {
                sleeptime = sleeptime + 30;
                add = add - 2;
                if (sleeptime == 350)
                {
                    add = 1;  //保证最低分为1
                }
            }
        }
        else if (GetAsyncKeyState(VK_F5))
        {
            system("cls");

            // 获取当前时间
            SYSTEMTIME time;
            GetLocalTime(&time);

            int duration = sleeptime; // 游戏持续时间

            // 显示日志信息
            printf("用户名: %s\n", storeUsername);
            printf("开始时间: %d-%02d-%02d %02d:%02d:%02d\n",
                   time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
            printf("持续时间: %d秒\n", duration);
            printf("得分: %d\n", score);
            recordGameLog(storeUsername, duration, score); // 记录游戏日志

            printf("\n按任意键继续游戏...\n");

            //while (!_kbhit()) {} // 等待用户按下任意键
        }

        Sleep(sleeptime);
        snakemove();
    }
}

void welcometogame()
{
    int loggedIn = 0;

    while (!loggedIn)
    {
        char choice;
        printf("欢迎来到贪食蛇游戏！\n");
        printf("请选择操作：\n");
        printf("1. 注册\n");
        printf("2. 登录\n");
        printf("3. 退出游戏\n"); // 添加退出游戏选项
        scanf(" %c", &choice);

        if (choice == '1')
        {
            registerUser();
        }
        else if (choice == '2')
        {
            if (verifyUser())
            {
                loggedIn = 1;
                printf("登录成功！\n");

                char playChoice;
                printf("是否进入游戏？(Y/N): ");
                scanf(" %c", &playChoice);
                if (playChoice == 'Y' || playChoice == 'y')
                {
                    printf("进入游戏...\n");
                    system("mode con cols=100 lines=30");
                    // gamecircle();
                }
                else
                {
                    printf("感谢使用，再见！\n");
                    break; // 退出游戏
                }
            }
            else
            {
                printf("登录失败，请重试。\n");
            }
        }
        else if (choice == '3')
        {
            printf("感谢使用，再见！\n");
            exit(0);
        }
        else
        {
            printf("无效的选择，请重新输入。\n");
        }
    }

    system("cls");
    Pos(40, 12);
    printf("欢迎来到贪食蛇游戏！");
    Pos(40, 25);
    system("pause");
    system("cls");
    Pos(25, 12);
    printf("用↑.↓.←.→分别控制蛇的移动， F1 为加速，2 为减速\n");
    Pos(25, 13);
    printf("加速将能得到更高的分数。\n");
    system("pause");
    system("cls");
}

void endgame()
{
    system("cls");
    Pos(24, 12);
    if (endgamestatus == 1)
    {
        printf("对不起，您撞到墙了。游戏结束!");
    }
    else if (endgamestatus == 2)
    {
        printf("对不起，您咬到自己了。游戏结束!");
    }
    else if (endgamestatus == 3)
    {
        printf("您已经结束了游戏。");
    }
    Pos(24, 13);
    printf("您的得分是 %d\n", score);

    char playChoice;
    printf("是否继续游戏？(Y/N): ");
    scanf(" %c", &playChoice);

    if (playChoice == 'Y' || playChoice == 'y')
    {
        system("cls");
        welcometogame(); // 返回到游戏开始界面
    }
    else
    {
        printf("感谢参与游戏，再见！\n");

        SYSTEMTIME time;
        GetLocalTime(&time);
        int duration = sleeptime; // 游戏持续时间
        recordGameLog(storeUsername, duration, score); // 记录游戏日志


    }
}



void gamestart()
{

    welcometogame();
    creatMap();
    initsnake();
    createfood();

}

int main()
{
    gamestart();
    gamecircle();
    endgame();

    return 0;
}
