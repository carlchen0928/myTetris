//
//  game.cpp
//  RussianCube
//
//  Created by 陈逸宇 on 6/15/14.
//
//

#include "game.h"

void Game::setMatrixCells(int *matrix, int width, int height, int value)
{
    for (int i = 0; i < width; ++i)
    {
        for (int j = 0; j < height; ++j)
        {
            *(matrix + i + (j * width)) = value;
        }
    }
}

void Game::setTetromino(int indexTetromino, Game::StcTetromino *tetromino)
{
    setMatrixCells(&tetromino->cells[0][0], TETROMINO_SIZE, TETROMINO_SIZE, EMPTY_CELL);
    tetromino->size = TETROMINO_SIZE - 1;    //why 3？
    
    // Initial configuration from http://tetris.wikia.com/wiki/SRS
    switch (indexTetromino) {
        case TETROMINO_I:
            tetromino->cells[0][1] = COLOR_CYAN;
            tetromino->cells[1][1] = COLOR_CYAN;
            tetromino->cells[2][1] = COLOR_CYAN;
            tetromino->cells[3][1] = COLOR_CYAN;
            tetromino->size = TETROMINO_SIZE;
            break;
            
        case TETROMINO_O:
            tetromino->cells[0][0] = COLOR_YELLOW;
            tetromino->cells[0][1] = COLOR_YELLOW;
            tetromino->cells[1][0] = COLOR_YELLOW;
            tetromino->cells[1][1] = COLOR_YELLOW;
            tetromino->size = TETROMINO_SIZE - 2;
            break;
            
        case TETROMINO_T:
            tetromino->cells[0][0] = COLOR_BLUE;
            tetromino->cells[1][0] = COLOR_BLUE;
            tetromino->cells[1][1] = COLOR_BLUE;
            tetromino->cells[2][1] = COLOR_BLUE;
            
            break;
            
        case TETROMINO_S:
            tetromino->cells[0][1] = COLOR_PURPLE;
            tetromino->cells[1][0] = COLOR_PURPLE;
            tetromino->cells[1][1] = COLOR_PURPLE;
            tetromino->cells[2][0] = COLOR_PURPLE;
            break;
            
        case TETROMINO_Z:
            tetromino->cells[0][0] = COLOR_GREEN;
            tetromino->cells[1][0] = COLOR_GREEN;
            tetromino->cells[1][1] = COLOR_GREEN;
            tetromino->cells[2][1] = COLOR_GREEN;
            break;
            
        case TETROMINO_J:
            tetromino->cells[0][0] = COLOR_RED;
            tetromino->cells[0][1] = COLOR_RED;
            tetromino->cells[1][1] = COLOR_RED;
            tetromino->cells[2][1] = COLOR_RED;
            break;
            
        case TETROMINO_L:
            tetromino->cells[0][1] = COLOR_ORANGE;
            tetromino->cells[1][1] = COLOR_ORANGE;
            tetromino->cells[2][1] = COLOR_ORANGE;
            tetromino->cells[2][0] = COLOR_ORANGE;
            break;
            
        default:
            break;
    }
}

void Game::start()
{
    mErrorCode = ERROR_NONE;
    mSystemTime = mPlatform->getSystemTime();
    mLastFallTime = mSystemTime;
    mIsOver = false;
    mIsPaused = false;
    mShowPreview = true;
    mEvents = EVENT_NONE;
    mFallingDelay = INIT_DELAY_FALL;
    mShowShadow = false;
    
    mStats.score = 0;
    mStats.lines = 0;
    mStats.totalPieces = 0;
    mStats.level = 0;
    for (int i = 0; i < TETROMINO_TYPES; ++i) {
        mStats.pieces[i] = 0;
    }
    
    //初始化整个棋盘
    setMatrixCells(&mMap[0][0], BOARD_TILEMAP_WIDTH, BOARD_TILEMAP_HEIGHT, EMPTY_CELL);
    
    //开始时随机设置一个nextblock作为第一个形状
    setTetromino(mPlatform->random() % TETROMINO_TYPES, &mFallingBlock);
    
    //初始化为mFallingBlock的up-left
    mFallingBlock.x = BOARD_TILEMAP_WIDTH / 2 - mFallingBlock.size / 2;
    mFallingBlock.y = 0;
    
    setTetromino(mPlatform->random() % TETROMINO_TYPES, &mNextBlock);
    onTetrominoMoved();
    
    mDelayDown = -1;
    mDelayLeft = -1;
    mDelayRight = -1;
    mDelayRotation = -1;
}

void Game::init(Platform *targetPlatform)
{
    mPlatform = targetPlatform;
    //用this指针初始化platform，也就是用game对象初始化
    mErrorCode = mPlatform->init(this);
    
    if(mErrorCode == ERROR_NONE)
    {
        start();
    }
}

void Game::end()
{
    mPlatform->end();
}


//旋转变换根据http://tetris.wikia.com/wiki/SRS
void Game::rotateTetromino(bool clockwise)
{
    //保存旋转后的数组形状，用来检测碰撞
    int rotated[TETROMINO_SIZE][TETROMINO_SIZE];
    
    //如果是O形，不用旋转
    if(mFallingBlock.type == TETROMINO_O)
    {
        return;
    }
    
    setMatrixCells(&rotated[0][0], TETROMINO_SIZE, TETROMINO_SIZE, EMPTY_CELL);
    
    //将旋转后的cell存入rotated数组
    for (int i = 0; i < mFallingBlock.size; ++i) {
        for (int j = 0; j < mFallingBlock.size; ++j) {
            if(clockwise)
            {
                rotated[mFallingBlock.size - j - 1][i] = mFallingBlock.cells[i][j];
            }
            else
            {
                rotated[j][mFallingBlock.size - i - 1] = mFallingBlock.cells[i][j];
            }
        }
    }
    
    /*
     这里为了简单，我不允许kick wall，也就是当方块贴住墙壁的时候，不允许旋转
    */
    for (int i = 0; i < mFallingBlock.size; ++i) {
        for (int j = 0; j < mFallingBlock.size; ++j) {
            if (rotated[i][j] != EMPTY_CELL) {
                //检测和墙壁的碰撞
                if (mFallingBlock.x + i < 0     //碰到左墙壁
                    //必须在墙内
                    || mFallingBlock.x + i >= BOARD_TILEMAP_WIDTH    //碰到右墙壁
                    || mFallingBlock.y + j >= BOARD_TILEMAP_HEIGHT) {//碰到地面
                    return;
                }
                
                //检测和已有方块的碰撞
                if(mMap[mFallingBlock.x + i][mFallingBlock.y + j] != EMPTY_CELL) {
                    return;
                }
            }
        }
    }
    
    //如果没有碰撞，那么就用rotated替代mFallingBlock
    for (int i = 0; i < mFallingBlock.size; ++i) {
        for (int j = 0; j < mFallingBlock.size; ++j) {
            mFallingBlock.cells[i][j] = rotated[i][j];
        }
    }
    onTetrominoMoved();
}

//检测移动中的碰撞
//如果有碰撞，返回true，否则返回false
//dx和dy是移动的offset
bool Game::checkCollision(int dx, int dy)
{
    int newx = mFallingBlock.x + dx;
    int newy = mFallingBlock.y + dy;
    
    for (int i = 0; i < mFallingBlock.size; ++i) {
        for (int j = 0; j < mFallingBlock.size; ++j) {
            if (mFallingBlock.cells[i][j] != EMPTY_CELL) {
                //检测墙壁的碰撞
                if(newx + i < 0
                   || newx + i >= BOARD_TILEMAP_WIDTH
                   || newy + j >= BOARD_TILEMAP_HEIGHT)
                    return true;
                
                //检测和其他cell的碰撞
                if (mMap[newx + i][newy + j] != EMPTY_CELL) {
                    return true;
                }
            }
        }
    }
    return false;
}

//消除的时候算分
void Game::onFilledRows(int filledRows)
{
    //本次游戏一共消除的行数
    mStats.lines += filledRows;
    
    //根据level和消除的行数计算分数
    switch (filledRows) {
        case 1:
            mStats.score += (SCORE_1_FILLED_ROW * (mStats.level + 1));
            break;
            
        case 2:
            mStats.score += (SCORE_2_FILLED_ROW * (mStats.level + 1));
            break;
        
        case 3:
            mStats.score += (SCORE_3_FILLED_ROW * (mStats.level + 1));
            break;
            
        case 4:
            mStats.score += (SCORE_4_FILLED_ROW * (mStats.level + 1));
            break;
            
        default:
            mErrorCode = ERROR_ASSERT;
            break;
    }
    
    //检查是否要升级
    if (mStats.lines >= FILLED_ROWS_FOR_LEVEL_UP * (mStats.level + 1)) {
        mStats.level++;
        //升级之后加快下落速度
        mFallingDelay = (int)(DELAY_FACTOR_FOR_LEVEL_UP * mFallingDelay / DELAY_DIVISOR_FOR_LEVEL_UP);
    }
    
    mPlatform->onLineCompleted();
}

//将方块向x，y方向移动
//检测是否与已有的cell冲突或者已经落地，同时检查游戏是否结束
void Game::moveTetromino(int x, int y)
{
    //如果有冲突
    if (checkCollision(x, y)) {
        //如果是往下移动的
        if (y == 1) {
            //如果mFallingBlock刚向下移动了一格，或者还没移动就冲突了，代表游戏结束
            if (mFallingBlock.y <= 1) {
                mIsOver = true;
            }
            else {
                //游戏没有结束，mFallingBlock落地
                //那么把mFallingBlock内的元素copy到mMap中
                for (int i = 0; i < mFallingBlock.size; ++i) {
                    for (int j = 0; j < mFallingBlock.size; ++j) {
                        if (mFallingBlock.cells[i][j] != EMPTY_CELL) {
                            mMap[mFallingBlock.x + i][mFallingBlock.y + j]
                            = mFallingBlock.cells[i][j];
                        }
                    }
                }
            }
            
            //检查落地是否能够得分
            /*
             x x x x x x x x x
             x x x x x x x x x
             x x x x x x x x x
             x x x x x x x x x
             这是mMap的形状
             */
            int numFilledRows = 0;
            for (int j = 1; j < BOARD_TILEMAP_HEIGHT; ++j) {
                bool hasFullRow = true;
                for (int i = 0; i < BOARD_TILEMAP_WIDTH; ++i) {
                    if (mMap[i][j] == EMPTY_CELL) {
                        hasFullRow = false;
                        break;
                    }
                }
                
                //这一排满了
                //将这一排上面的都往下移动一排，相当于消除这一排
                //注意：(0, 0)点在上面图形的右上角，也就是屏幕的左上角
                if (hasFullRow) {
                    for (int t = j; t > 0; --t) {
                        for (int p = 0; p < BOARD_TILEMAP_WIDTH; ++p) {
                            mMap[p][t] = mMap[p][t - 1];
                        }
                    }
                    numFilledRows++;
                }
            }
            
            //计算分数
            if (numFilledRows > 0) {
                onFilledRows(numFilledRows);
            }
            
            mStats.totalPieces++;
            mStats.pieces[mFallingBlock.type]++;
            
            //用下一个形状代替fallingblock
            for (int i = 0; i < TETROMINO_SIZE; ++i) {
                for (int j = 0; j < TETROMINO_SIZE; ++j) {
                    mFallingBlock.cells[i][j] = mNextBlock.cells[i][j];
                }
            }
            mFallingBlock.size = mNextBlock.size;
            mFallingBlock.type = mNextBlock.type;
            
            mFallingBlock.y = 0;
            mFallingBlock.x = BOARD_TILEMAP_WIDTH / 2 - mFallingBlock.size / 2;
            onTetrominoMoved();
            
            //随机选择下一个方块
            setTetromino(mPlatform->random() % TETROMINO_TYPES, &mNextBlock);
        }
    }
    else {
        //没有冲突，直接移动
        mFallingBlock.x += x;
        mFallingBlock.y += y;
    }
    onTetrominoMoved();
}

// 完成一次hard drop，有一定的分数奖励
// A Hard Drop is the event in which a Tetromino drops
// instantly to where the Ghost Piece is. It can't be
// moved or rotated afterwards. It causes the Score to go up.
void Game::dropTetromino()
{
    //计算block的位置？
    mFallingBlock.y += mShadowGap;
    //Force lock?
    moveTetromino(0, 1);
    
    //更新分数，使用了shadow的分数减少
    if (mShowShadow) {
        mStats.score += (long)(SCORE_2_FILLED_ROW * (mStats.level + 1)
                               / SCORE_DROP_WITH_SHADOW_DIVISOR);
    }
    else {
        mStats.score += (long)(SCORE_2_FILLED_ROW * (mStats.level + 1)
                               / SCORE_DROP_DIVISOR);
    }
    
    //计算方块最多能下落到哪里
    int y = 1;
    while (!checkCollision(0, ++y));
    moveTetromino(0, y - 1);
    //Force lock?
    moveTetromino(0, 1);
    
    mStats.score += (long)(SCORE_2_FILLED_ROW * (mStats.level + 1)
                                / SCORE_DROP_DIVISOR);
    mPlatform->onPieceDrop();
}

//每帧更新都要调用的方法
void Game::update()
{
    //这个方法是空的
    mPlatform->processEvents();
    
    if (mIsOver) {
        if ((mEvents & EVENT_RESTART) != 0) {
            mIsOver = false;
            start();
        }
    }
    else {
        if ((mEvents & EVENT_RESTART) != 0) {
            start();
            return;
        }
        
        long currentTime = mPlatform->getSystemTime();
        
        //上次调用update的时间为mSystemTime
        //timeDelta就是距离上次刷新过去的millseconds
        int timeDelta = (int)(currentTime - mSystemTime);
        if (mDelayDown > 0) {
            mDelayDown -= timeDelta;
            if (mDelayDown <= 0) {
                //到了下降的时间了，重置mDelayDown
                mDelayDown = DAS_MOVE_TIMER;
                //设置下降事件
                mEvents |= EVENT_MOVE_DOWN;
            }
        }
        if (mDelayLeft > 0) {
            mDelayLeft -= timeDelta;
            if (mDelayLeft <= 0) {
                mDelayLeft = DAS_MOVE_TIMER;
                mEvents |= EVENT_MOVE_LEFT;
            }
        }
        else if (mDelayRight > 0) {
            mDelayRight -= timeDelta;
            if (mDelayRight <= 0) {
                mDelayRight = DAS_MOVE_TIMER;
                mEvents |= EVENT_MOVE_RIGHT;
            }
        }
        
        if (mDelayRotation > 0) {
            mDelayRotation -= timeDelta;
            if (mDelayRotation > 0) {
                mDelayRotation = ROTATION_AUTOREPEAT_TIMER;
                mEvents |= EVENT_ROTATE_CW;
            }
        }
        
        //处理暂停
        if ((mEvents & EVENT_PAUSE) != 0) {
            //如果检测到暂停，清空所有当前事件，等待暂停结束
            mIsPaused = !mIsPaused;
            mEvents = EVENT_NONE;
        }
        
        if (mIsPaused) {
            //暂停了，就要更新距离上次方块落地过去的时间
            //这个变量好像没什么用
            mLastFallTime += (currentTime - mSystemTime);
        }
        else {
            if (mEvents != EVENT_NONE) {
                if ((mEvents & EVENT_SHOW_NEXT) != 0) {
                    mShowPreview = !mShowPreview;
                    mStateChanged = true;
                }
                if ((mEvents & EVENT_DROP) != 0) {
                    //hard drop
                    dropTetromino();
                }
                if ((mEvents & EVENT_ROTATE_CW) != 0) {
                    rotateTetromino(true);
                }
                if ((mEvents & EVENT_MOVE_LEFT) != 0) {
                    moveTetromino(-1, 0);
                }
                else if ((mEvents & EVENT_MOVE_RIGHT) != 0) {
                    moveTetromino(1, 0);
                }
                if ((mEvents & EVENT_MOVE_DOWN) != 0) {
                    //这是玩家主动按键触发的下降事件，所以算作玩家加速下降，应该加分
                    mStats.score += (long)(SCORE_2_FILLED_ROW * (mStats.level + 1)
                                            / SCORE_MOVE_DOWN_DIVISOR);
                    moveTetromino(0, 1);
                }
                mEvents = EVENT_NONE;
            }
            
            //如果没有事件，检查是否到了降落一格方块的时候
            if (currentTime - mLastFallTime >= mFallingDelay) {
                //这个下降是系统被动下降，不是玩家主动下降，所以没有加分
                moveTetromino(0, 1);
                mLastFallTime = currentTime;
            }
        }
        //把mSystemTime更新为调用这次update的时间
        mSystemTime = currentTime;
    }
    //绘制游戏
    mPlatform->renderGame();
}

void Game::onTetrominoMoved()
{
    int y = 0;
    //计算ghost piece方块所在位置
    while (!checkCollision(0, ++y));
    mShadowGap = y - 1;
    mStateChanged = true;
}

//处理按键事件
void Game::onEventStart(int command)
{
    switch (command) {
        case EVENT_QUIT:
            mErrorCode = ERROR_PLAYER_QUITS;
            break;
        
        case EVENT_MOVE_DOWN:
            mEvents |= EVENT_MOVE_DOWN;
            mDelayDown = DAS_DELAY_TIMER;
            break;
            
        case EVENT_ROTATE_CW:
            mEvents |= EVENT_ROTATE_CW;
            mDelayRotation = ROTATION_AUTOREPEAT_DELAY;
            break;
            
        case EVENT_MOVE_LEFT:
            mEvents |= EVENT_MOVE_LEFT;
            mDelayLeft = DAS_DELAY_TIMER;
            break;
            
        case EVENT_MOVE_RIGHT:
            mEvents |= EVENT_MOVE_RIGHT;
            mDelayRight = DAS_DELAY_TIMER;
            break;
        
        case EVENT_DROP:        //fall through
        case EVENT_RESTART:
        case EVENT_PAUSE:
        case EVENT_SHOW_NEXT:
        case EVENT_SHOW_SHADOW:
            mEvents |= command;
            break;
    }
}

//按键事件处理完成后调用，恢复状态
void Game::onEventEnd(int command)
{
    switch (command) {
        case EVENT_MOVE_DOWN:
            mDelayDown = -1;
            break;
            
        case EVENT_MOVE_LEFT:
            mDelayLeft = -1;
            break;
            
        case EVENT_MOVE_RIGHT:
            mDelayRight = -1;
            break;
            
        case EVENT_ROTATE_CW:
            mDelayRotation = -1;
            break;
            
        default:
            break;
    }
}





















