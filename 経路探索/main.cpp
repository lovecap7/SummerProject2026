#include "DxLib.h"
#include <algorithm>
#include <vector>
#include <deque>
#include <climits>
#include <cstdlib>

namespace Map
{
    //横幅縦幅
    constexpr int kWidth = 10;
    constexpr int kHeight = 15;
}

//ノード
struct Node
{
    int m_x = 0;
    int m_y = 0;
    //スタートからの距離
    int m_goalLen = 0;
    //ゴールまでの予想距離
    int m_predictionLen = 0;
    //m_goalLen+m_predictionLen
    int m_ansLen = 0;
    //親ノードのポインタ
    Node* m_parent = nullptr;
};

//上下左右への移動量
const int kMoveX[4] = { 0, 0, -1, 1 };
const int kMoveY[4] = { -1, 1, 0, 0 };

//1マス移動するのにかかるコスト
constexpr int kMoveCost = 10;

//1マス移動するのにかかるフレーム数
constexpr int kMoveFrame = 60;

//マンハッタン距離でゴールまでの予想距離を計算
int CalcPrediction(int x, int y, int goalX, int goalY)
{
    return (std::abs(goalX - x) + std::abs(goalY - y)) * kMoveCost;
}

//A*で経路探索を行う
//戻り値はスタート→ゴールの順に並んだ座標列(見つからなければ空)
std::vector<std::pair<int, int>> FindPath(int startX, int startY, int goalX, int goalY,
    int map[Map::kHeight][Map::kWidth])
{
    std::vector<std::pair<int, int>> path;

    if (goalX < 0 || goalX >= Map::kWidth || goalY < 0 || goalY >= Map::kHeight)
    {
        return path;
    }
    if (map[goalY][goalX] == 1)
    {
        return path;
    }

    //ノードの実体はここに溜めておく
    //(dequeはpush_backしても既存要素のアドレスが変わらないので、m_parentが無効にならない)
    std::deque<Node> nodePool;

    //オープンリスト(実体はnodePoolにあるのでポインタで持つ)
    std::vector<Node*> openList;

    //各マスに到達するまでの最短のスタートからの距離
    int bestLen[Map::kHeight][Map::kWidth];
    for (int y = 0; y < Map::kHeight; ++y)
    {
        for (int x = 0; x < Map::kWidth; ++x)
        {
            bestLen[y][x] = INT_MAX;
        }
    }

    Node startNode;
    startNode.m_x = startX;
    startNode.m_y = startY;
    startNode.m_goalLen = 0;
    startNode.m_predictionLen = CalcPrediction(startX, startY, goalX, goalY);
    startNode.m_ansLen = startNode.m_goalLen + startNode.m_predictionLen;
    startNode.m_parent = nullptr;

    nodePool.push_back(startNode);
    openList.push_back(&nodePool.back());
    bestLen[startY][startX] = 0;

    Node* goalNode = nullptr;

    while (!openList.empty())
    {
        auto itr = std::min_element(openList.begin(), openList.end(),
            [](Node* a, Node* b)
            {
                return a->m_ansLen < b->m_ansLen;
            });

        Node* current = *itr;
        openList.erase(itr);

        if (current->m_x == goalX && current->m_y == goalY)
        {
            goalNode = current;
            break;
        }

        for (int i = 0; i < 4; ++i)
        {
            int nx = current->m_x + kMoveX[i];
            int ny = current->m_y + kMoveY[i];

            if (nx < 0 || nx >= Map::kWidth || ny < 0 || ny >= Map::kHeight)
            {
                continue;
            }
            if (map[ny][nx] == 1)
            {
                continue;
            }

            int newGoalLen = current->m_goalLen + kMoveCost;

            if (newGoalLen >= bestLen[ny][nx])
            {
                continue;
            }
            bestLen[ny][nx] = newGoalLen;

            Node next;
            next.m_x = nx;
            next.m_y = ny;
            next.m_goalLen = newGoalLen;
            next.m_predictionLen = CalcPrediction(nx, ny, goalX, goalY);
            next.m_ansLen = next.m_goalLen + next.m_predictionLen;
            next.m_parent = current;

            nodePool.push_back(next);
            openList.push_back(&nodePool.back());
        }
    }

    if (goalNode != nullptr)
    {
        for (Node* node = goalNode; node != nullptr; node = node->m_parent)
        {
            path.push_back({ node->m_x, node->m_y });
        }
        std::reverse(path.begin(), path.end());
    }

    return path;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // ウィンドウモードで起動
    ChangeWindowMode(TRUE);
    // DXライブラリ初期化
    if (DxLib_Init() == -1)
    {
        return -1;
    }
    // 裏画面に描画
    SetDrawScreen(DX_SCREEN_BACK);
    //マップの準備
    //マップの配列 0は通れる,1は壁,2はプレイヤー
    int m_map[Map::kHeight][Map::kWidth] =
    {
       {0,1,0,0,0,0,1,0,0,0},
       {0,1,0,1,0,0,0,0,0,0},
       {0,1,0,1,0,0,1,0,0,0},
       {0,1,0,1,0,0,1,0,0,0},
       {0,1,0,0,0,0,1,0,0,0},
       {0,1,0,0,0,0,0,0,0,0},
       {0,1,0,0,0,0,0,0,0,0},
       {0,1,0,0,0,0,1,0,0,0},
       {0,0,0,0,0,0,1,0,0,0},
       {0,0,0,0,0,0,1,0,1,0},
       {0,0,0,1,0,0,1,0,1,0},
       {0,1,0,1,0,0,1,0,1,0},
       {0,1,0,0,0,0,1,0,0,0},
       {0,1,0,0,0,0,1,0,0,0},
       {0,1,0,0,0,0,1,0,0,0},
    };
    int playerPosX = 0;
    int playerPosY = 0;
    m_map[playerPosY][playerPosX] = 2;

    //左クリックで選択した経路(プレビュー用)
    std::vector<std::pair<int, int>> path;
    //選択中のゴール座標(未選択なら-1)
    int selectedGoalX = -1;
    int selectedGoalY = -1;

    //移動中かどうか
    bool isMoving = false;
    //現在いる区間の開始インデックス(path[moveIndex]→path[moveIndex+1]へ移動中)
    int moveIndex = 0;
    //現在の区間内での経過フレーム数(0～kMoveFrame)
    int moveFrame = 0;

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        // 画面をクリア
        ClearDrawScreen();
        // 画面サイズ取得
        int width, height;
        GetDrawScreenSize(&width, &height);
        //マスのサイズ
        int mapSizeX = (width / Map::kWidth);
        int mapSizeY = (height / Map::kHeight);
        int mapSize = (std::min)(mapSizeX, mapSizeY);
        //マウスクリック位置
        int mx, my;
        GetMousePoint(&mx, &my);

        int mouseInput = GetMouseInput();

        //移動中でなければ、左クリックでゴールを選び直せる
        if (!isMoving && (mouseInput & MOUSE_INPUT_LEFT) != 0)
        {
            int goalX = mx / mapSize;
            int goalY = my / mapSize;
            selectedGoalX = goalX;
            selectedGoalY = goalY;
            //クリックされた場所への経路を探索してプレビュー
            path = FindPath(playerPosX, playerPosY, goalX, goalY, m_map);
        }

        //移動中でなく、有効な経路がある状態で右クリックされたら移動開始
        if (!isMoving && (mouseInput & MOUSE_INPUT_RIGHT) != 0 && path.size() >= 2)
        {
            isMoving = true;
            moveIndex = 0;
            moveFrame = 0;
        }

        //移動更新処理
        if (isMoving)
        {
            ++moveFrame;
            if (moveFrame >= kMoveFrame)
            {
                moveFrame = 0;
                ++moveIndex;

                //論理的な位置を1マス進める
                m_map[playerPosY][playerPosX] = 0;
                playerPosX = path[moveIndex].first;
                playerPosY = path[moveIndex].second;
                m_map[playerPosY][playerPosX] = 2;

                //ゴールに到達したら移動終了
                if (moveIndex >= static_cast<int>(path.size()) - 1)
                {
                    isMoving = false;
                    path.clear();
                    moveIndex = 0;
                    selectedGoalX = -1;
                    selectedGoalY = -1;
                }
            }
        }

        //マップ描画
        for (int i = 0; i < Map::kHeight; ++i)
        {
            for (int j = 0; j < Map::kWidth; ++j)
            {
                int color = 0xffffff;
                if (m_map[i][j] >= 2)
                {
                    color = 0xff0000;
                }
                DrawBox(j * mapSize, i * mapSize,
                    j * mapSize + mapSize, i * mapSize + mapSize,
                    color, m_map[i][j] != 0);
            }
        }

        //経路プレビュー描画(緑色)
        for (const auto& p : path)
        {
            DrawBox(p.first * mapSize, p.second * mapSize,
                p.first * mapSize + mapSize, p.second * mapSize + mapSize,
                0x00ff00, TRUE);
        }

        //選択中のゴール地点を強調(水色)
        if (selectedGoalX >= 0 && selectedGoalY >= 0)
        {
            DrawBox(selectedGoalX * mapSize, selectedGoalY * mapSize,
                selectedGoalX * mapSize + mapSize, selectedGoalY * mapSize + mapSize,
                0xff00ff, TRUE);
        }

        //プレイヤーの描画位置を計算(移動中は補間して滑らかに動かす)
        float drawX = static_cast<float>(playerPosX);
        float drawY = static_cast<float>(playerPosY);
        if (isMoving)
        {
            float t = static_cast<float>(moveFrame) / static_cast<float>(kMoveFrame);
            int nextX = path[moveIndex + 1].first;
            int nextY = path[moveIndex + 1].second;
            drawX = playerPosX + (nextX - playerPosX) * t;
            drawY = playerPosY + (nextY - playerPosY) * t;
        }

        //プレイヤーは経路やマップの上から常に見えるように描画
        int px = static_cast<int>(drawX * mapSize);
        int py = static_cast<int>(drawY * mapSize);
        DrawBox(px, py, px + mapSize, py + mapSize, 0xff0000, TRUE);

        // 描画内容を画面に反映
        ScreenFlip();
    }
    // DXライブラリ終了
    DxLib_End();
    return 0;
}