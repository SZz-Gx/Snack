#include <iostream>
#include <deque>    //只操作头和尾，蛇身没有动的必要性
#include "raylib.h" //绘图
#include <string>
#include <fstream>

//常量设置
const int CELL_SIZE = 10;   //每个格子的像素是10*10
const int COLS = 64;    //640 / 10
const int ROWS = 48;    //480 / 10

//定义碰撞箱结构体
struct Cell
{
    int x;
    int y;
    //重载 == 运算符，用于判断碰撞箱
    bool operator==(const Cell& other) const
    {
        return x == other.x && y == other.y;
    }
};

//sprite（精灵类），只提供接口，数据自己初始化
class Sprite
{
public:
    Sprite(int x,int y,Color color):m_x(x),m_y(y),m_color(color){};    //有参默认构造
    //绘制精灵，定义为虚函数，继承的时候必须重写
    virtual void draw() = 0;
protected:
    //坐标
    int m_x;
    int m_y;
    Color m_color;   //使用raylib的内置结构体
};

//snake
class Snake : public Sprite
{
public:
    //利用初始化列表，把 x , y 传给父类 sprite
    Snake(int x,int y,Color color):Sprite(x,y,color)
    {
        /*根据初始种子坐标，初始化三节蛇身
        * 假设初始向上走，所以身体的 y 坐标依次 +1，y轴正方向与数学常用建系相反
        * body.push_back({x,y}) 蛇头,循环创建10个块
        */
        for (int i = 0;i<10;++i)
        {
            body.push_back({x,y+i});
        }
    };

    //绘制snake
    void draw() override
    {
        // for (size_t i = 0;i<body.size();++i)
        // {
        //     //蛇头画深绿色，身体画浅绿色用以区分方向
        //     Color c = (i==0) ? DARKBLUE : m_color;
        //     //网格坐标*10，变成px坐标；画9*9的方格，留1px做网格线
        //     DrawRectangle(body[i].x*10,body[i].y*10,9,9,c);
        // }

        for (const auto& cell : body)
        {
            Color c = (&cell == &body.front()) ? DARKBLUE : m_color;  //蛇头深蓝，蛇身浅蓝
            //换算：网格*CELL_SIZE = p_i(x,y)
            // 宽度画 9，留 1 像素的缝隙，视觉上就不会糊成一片
            DrawRectangle(cell.x * CELL_SIZE, cell.y * CELL_SIZE,
                          CELL_SIZE - 1, CELL_SIZE - 1, c);
        }
    }

    //核心移动逻辑
    /*
    void move(int dx,int dy)
    {
        //1.获取当前蛇头
        Cell head = body.front();

        //2.根据方向计算新的蛇头
        Cell newHead = {head.x + dx,head.y + dy};

        //3.头部入队
        body.push_front(newHead);

        //4.尾部出队
        body.pop_back();

        //同步父类坐标
        m_x = newHead.x;
        m_y = newHead.y;
    }
    */
    //让move返回一个状态标志，用于区分方向/死亡/是否吃得food
    bool move(int dx,int dy,bool isEating)
    {
        Cell head = body.front();
        Cell newHead = {head.x + dx,head.y + dy };

        //碰撞检测
        bool hitWall = (newHead.x<0 || newHead.x>=COLS ||
                        newHead.y<0 || newHead.y>=ROWS);

        if (hitWall)
        {
            //返回false表示死亡
            return false;
        }

        //撞自己检测
        for (const auto& cell : body)
        {
            if (cell == newHead)
            {
                return false;
            }
        }

        //移动（入队\出队）
        body.push_front(newHead);

        //如果吃到food，不删尾巴，蛇变成
        if (!isEating)
        {
            body.pop_back();
        }

        //同步父类坐标
        m_y = newHead.y;
        m_x = newHead.x;

        return true;    //活着
    }

    // 获取蛇头位置（用来判断食物是否被吃）
    Cell getHead() const { return body.front(); }

    // 获取蛇身（用来让 main 判断新食物是否刷在蛇身上，或者直接把这个逻辑封装到类里更好）
    const std::deque<Cell>& getBody() const { return body; }

private:
    std::deque<Cell>body;   //蛇的所有节点
};

//food
class Food : public Sprite
{
public:
    //利用初始化列表，把 x , y 传给父类 sprite
    Food(int x,int y,Color color):Sprite(x,y,color){};

    //绘制food
    void draw() override
    {
        DrawRectangle(m_x,m_y,CELL_SIZE - 1,CELL_SIZE - 1,m_color);
    }
};

int main()
{
    InitWindow(640, 480, "贪吃蛇");    // 1. 初始化窗口 (宽 800, 高 450, 标题)
    SetTargetFPS(60);    // 2. 设置帧率上限 (每秒30帧）
    bool restart = false;

    do
    {
        restart = false;
        Snake mySnake(32,24,BLUE);   //定蛇头的位置及颜色

        //随机生成food
        Cell foodPos = {GetRandomValue(0,COLS - 1),GetRandomValue(0,ROWS - 1)};

        float moveTimer = 0.0f;
        float moveInterval = 0.3f;
        int dirX = 0;int dirY = -1;     //初始方向向上
        bool gameOver = false;

        //绘板
        while (!WindowShouldClose() && !restart)
        {
            if (!gameOver)
            {
                // 处理按键，防止直接掉头
                if (IsKeyPressed(KEY_UP) && dirY != 1)  { dirX = 0; dirY = -1; }
                if (IsKeyPressed(KEY_DOWN) && dirY != -1) { dirX = 0; dirY = 1; }
                if (IsKeyPressed(KEY_LEFT) && dirX != 1) { dirX = -1; dirY = 0; }
                if (IsKeyPressed(KEY_RIGHT) && dirX != -1){ dirX = 1; dirY = 0; }

                //利用帧时间做计时器
                /*
                 * 每秒的帧数是固定的，所以把每一帧的所用的时间直接累加给移动判断触发器“moveTimer”
                 * 如果累加器的值大于等于触发器，累加器的值清零，实现移动
                 * 移动方向是上一轮的方向或者键入值
                 */
                moveTimer += GetFrameTime();
                if (moveTimer >= moveInterval)
                {
                    moveTimer = 0.0f;

                    //判断这一步是否会吃到food，即新蛇头的位置是否等于food位置
                    Cell head = mySnake.getHead();
                    Cell nextHead = {head.x + dirX,head.y + dirY};
                    bool isEating = (nextHead == foodPos);

                    //执行移动
                    bool isAlive = mySnake.move(dirX, dirY, isEating);

                    if (!isAlive) {
                        gameOver = true;
                    } else if (isEating) {
                        // 如果吃到了，重新随机生成食物（确保不会刷在蛇身上，这里简略写）
                        bool valid = false;
                        while (!valid) {
                            foodPos.x = GetRandomValue(0, COLS - 1);
                            foodPos.y = GetRandomValue(0, ROWS - 1);
                            valid = true;
                            // 遍历蛇身，如果撞了就不合法（你可以把这段封装进 snake 类里）
                            for (const auto& cell : mySnake.getBody()) {
                                if (cell == foodPos) { valid = false; break; }
                            }
                        }
                    }
                }
            }

            //绘制画面
            BeginDrawing();           // 开始绘制
            ClearBackground(RAYWHITE); // 清空背景，不然上一帧的图会叠在上面

            //把对象画出来
            //画food
            DrawRectangle(foodPos.x * CELL_SIZE, foodPos.y * CELL_SIZE,
                          CELL_SIZE - 1, CELL_SIZE - 1, RED);
            mySnake.draw();

            // 如果游戏结束，覆盖一层文字
            if (gameOver) {
                DrawText("GAME OVER!", 220, 200, 40, RED);
                DrawText("Press R to Restart", 200, 260, 20, DARKGRAY);
                if (IsKeyPressed(KEY_R)) {
                    restart = true;
                }
            }
            EndDrawing();             // 结束绘制
        }
    }while (restart && !WindowShouldClose());   //如果按了R且窗口没关，重开
    // 4. 关闭窗口并释放资源
    CloseWindow();
    return 0;
}