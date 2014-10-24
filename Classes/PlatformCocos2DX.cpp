//
//  PlatformCocos2DX.cpp
//  RussianCube
//
//  Created by 陈逸宇 on 6/17/14.
//
//

#include "PlatformCocos2DX.h"
#include "SimpleAudioEngine.h"
#include <iostream>

using namespace cocos2d;

PlatformCocos2DX::PlatformCocos2DX()
{
    //游戏实现的逻辑类，里面保存了一局游戏的状态
    m_game = new Game();
    m_elapsedTime = 0L;
}

PlatformCocos2DX::~PlatformCocos2DX()
{
    delete m_game;
    m_game = nullptr;
}

Scene * PlatformCocos2DX::scene()
{
    Scene *scene = Scene::create();
    //因为使用了CREATE_FUNC，所以可以调用create创建对象
    PlatformCocos2DX *layer = PlatformCocos2DX::create();
    
    scene->addChild(layer);
    return scene;
}

bool PlatformCocos2DX::init()
{
    //先初始化基类
    if (!LayerColor::initWithColor(Color4B(255, 255, 255, 255))) {
        return false;
    }
    
    //初始化随机数产生器
    srand(time(nullptr));
    
    //激活触摸,3.0里面不再需要
    //setTouchEnabled(true);
    
    Director::getInstance()->setProjection(Director::Projection::_2D);
    //是否显示状态
    Director::getInstance()->setDisplayStats(true);
    
    //获得窗口大小
    Size size = Director::getInstance()->getWinSize();
    
    //TODO
    m_yOffset = int(size.height - (size.height - BACKGROUND_HEIGHT) / 2);
    
    //添加背景图片
    Sprite *pBackgound = Sprite::create("back1.png");
    pBackgound->setScale(2);
    
    addChild(pBackgound, 0);
    
    //创建渲染纹理
    m_canvas = RenderTexture::create(size.width / 2, size.height / 2);
    
    if (nullptr == m_canvas) {
        return false;
    }
    m_canvas->setPosition(size.width / 2, size.height / 2);
    addChild(m_canvas);
    
    //创建drawing方块种类的的sprite，每种方块都有一个颜色
    for (int i = 0; i < Game::TETROMINO_TYPES + 1; ++i) {
        m_tiles[i] = Sprite::create("blocks1.png",
                            Rect((TILE_SIZE + 2) * i, 0,
                            TILE_SIZE + 2, TILE_SIZE + 2));
        //retain的意思是保存这个指针一段时间，自己调用release释放。如果
        //调用autorelease()，就要接受内在的内存管理
        m_tiles[i]->retain();
    }
    
    //创建shadow的sprite
    for (int i = 0; i < Game::TETROMINO_TYPES + 1; ++i) {
        m_shadows[i] = Sprite::create("blocks1.png",
                            Rect((TILE_SIZE + 2) * i, TILE_SIZE + 2,
                            TILE_SIZE - 2, TILE_SIZE - 2));
        m_shadows[i]->retain();
    }
    
    //创建drawing数字的sprite
    for (int i = 0; i < Game::TETROMINO_TYPES + 1; ++i) {
        for (int j = 0; j < 10; ++j) {
            m_numbers[i][j] = Sprite::create("numbers.png",
                            Rect((NUMBER_WIDTH + 1) * j, (NUMBER_HEIGHT + 1) * i,
                            NUMBER_WIDTH + 1, NUMBER_HEIGHT + 1));
            m_numbers[i][j]->retain();
        }
    }
    
    //创建退出菜单
    MenuItemImage *pCloseItem = MenuItemImage::create(
                        "close_normal.png",
                        "close_selected.png",
                        this,
                        menu_selector(PlatformCocos2DX::menuCloseCallback));
    
    pCloseItem->setPosition(size.width - 20, 20);
    
    //使用pCloseItem创建menu
    Menu *pMenu = Menu::create(pCloseItem, NULL);
    pMenu->setPosition(0, 0);
    addChild(pMenu, 1);
    
    //设置计划任务，以及调用的函数
    this->schedule(schedule_selector(PlatformCocos2DX::update));
    
    m_game->init(this);
    return true;
}

Sprite *PlatformCocos2DX::getTile(int x, int y, int id, bool shadow)
{
    Sprite *tile;
    if (!shadow) {
        tile = Sprite::create("blocks1.png",
                            Rect((TILE_SIZE + 2) * id, 0,
                            TILE_SIZE + 2, TILE_SIZE + 2));
        tile->setPosition(x + TILE_SIZE / 2, m_yOffset - y - TILE_SIZE / 2);
    }
    else
    {
        tile = Sprite::create("blocks1.png",
                              Rect((TILE_SIZE + 2) * id, TILE_SIZE + 2,
                                   TILE_SIZE - 2, TILE_SIZE - 2));
        tile->setPosition(x + TILE_SIZE / 2 - 1, m_yOffset - y - TILE_SIZE / 2);
    }
    return tile;
}

void PlatformCocos2DX::menuCloseCallback(cocos2d::Object *pSender)
{
    Director::getInstance()->end();
    exit(0);
}


void PlatformCocos2DX::TouchesBegan(cocos2d::Set *touches, cocos2d::Event *event)
{
    for (auto it = touches->begin(); it != touches->end(); ++it) {
        Touch *touch = (Touch *)(*it);
        //getLocationInView，获得UI上（也就是屏幕上）触摸第一点的位置
        float tx = touch->getLocationInView().x;
        float ty = touch->getLocationInView().y;
        
        if (tx < TX_1) {
            if (ty < TY_1) {
                m_game->onEventStart(Game::EVENT_SHOW_NEXT);
            }
            else if (ty < TY_2) {
                m_game->onEventStart(Game::EVENT_MOVE_LEFT);
            }
            else {
                m_game->onEventStart(Game::EVENT_RESTART);
            }
        }
        else if (tx < TX_2) {
            if (ty < TY_DOWN) {
                m_game->onEventStart(Game::EVENT_ROTATE_CW);
            }
            else if (ty > TY_DROP) {
                m_game->onEventStart(Game::EVENT_DROP);
            }
            else {
                m_game->onEventStart(Game::EVENT_MOVE_DOWN);
            }
        }
        else {
            if (ty < TY_1) {
                m_game->onEventStart(Game::EVENT_SHOW_SHADOW);
            }
            else if (ty < TY_2) {
                m_game->onEventStart(Game::EVENT_MOVE_RIGHT);
            }
            else {
                m_game->onEventStart(Game::EVENT_PAUSE);
            }
        }
        CCLOG("-- touchStart: %d %d", int(tx), int(ty));
    }
}

void PlatformCocos2DX::TouchesEnded(cocos2d::Set *touches, cocos2d::Event *event)
{
    for (auto it = touches->begin(); it != touches->end(); ++it)
	{
		Touch* touch = (Touch *)(*it);
		float tx = touch->getLocationInView().x;
		float ty = touch->getLocationInView().y;
		CCLOG("-- touchEnd: %d %d", int(tx), int(ty));
        
		// Just cancel any continuos action by now
		m_game->onEventEnd(Game::EVENT_MOVE_LEFT);
		m_game->onEventEnd(Game::EVENT_MOVE_RIGHT);
		m_game->onEventEnd(Game::EVENT_MOVE_DOWN);
		m_game->onEventEnd(Game::EVENT_ROTATE_CW);
	}
}

//每次帧更新都要调用
void PlatformCocos2DX::update(float f)
{
    m_elapsedTime += long(1000 * f);
    m_game->update();
}

void PlatformCocos2DX::processEvents()
{
    
}

int PlatformCocos2DX::init(Game *game)
{
    return Game::EVENT_NONE;
}

void PlatformCocos2DX::end()
{
    
}

void PlatformCocos2DX::drawTile(int x, int y, int tile, bool shadow)
{
    if (!shadow) {
        m_tiles[tile]->setPosition(x + TILE_SIZE / 2, m_yOffset - y - TILE_SIZE / 2);
        m_tiles[tile]->visit();
    }
    else {
        m_shadows[tile]->setPosition(x + TILE_SIZE / 2 - 1, m_yOffset - y - TILE_SIZE / 2);
        m_shadows[tile]->visit();
    }
}

// Draw a number on the given position.
// Number sprites start at the left below the sprites for tiles.
void PlatformCocos2DX::drawNumber(int x, int y, long number, int length, int color)
{
	int pos = 0;
	do
	{
		int px = x + NUMBER_WIDTH * (length - pos);
		m_numbers[color][number % 10]->setPosition(px + NUMBER_WIDTH / 2 + 1,
                                                       m_yOffset - y - NUMBER_HEIGHT / 2);
		m_numbers[color][number % 10]->visit();
        
		number /= 10;
        
	} while (++pos < length);
}

long PlatformCocos2DX::getSystemTime()
{
	return m_elapsedTime;
}

int PlatformCocos2DX::random()
{
	return rand();
}


void PlatformCocos2DX::renderGame()
{
    //如果状态改变了，重新渲染
    if (m_game->hasChanged()) {
        m_canvas->beginWithClear(0, 0, 0, 0);
        
        //绘制previews区域
        if (m_game->showPreview()) {
            for (int i = 0; i < Game::TETROMINO_SIZE; ++i) {
                for (int j = 0; j < Game::TETROMINO_SIZE; ++j) {
                    //如果下一个block的i，j的不为空，就绘制
                    if (m_game->nextBlock().cells[i][j] != Game::EMPTY_CELL) {
                        drawTile(PREVIEW_X + (TILE_SIZE * i),
                                 PREVIEW_Y + (TILE_SIZE * j),
                                 m_game->nextBlock().cells[i][j],
                                 false);
                    }
                }
            }
        }
        
        //在BOARD中绘制shadow方块
        if (m_game->showShadow()) {
            for (int i = 0; i < Game::TETROMINO_SIZE; ++i) {
                for (int j = 0; j < Game::TETROMINO_SIZE; ++j) {
                    if (m_game->fallingBlock().cells[i][j] != Game::EMPTY_CELL) {
                        drawTile(BOARD_X + (TILE_SIZE * (m_game->fallingBlock().x + i)),
                                 BOARD_Y + (TILE_SIZE * (m_game->fallingBlock().y + j + m_game->shadowGap())),
                                 m_game->fallingBlock().cells[i][j],
                                 true);
                    }
                }
            }
        }
        
        //绘制BOARD，这一步要在下一步之前，不然就会覆盖
        for (int i = 0; i < Game::BOARD_TILEMAP_WIDTH; ++i) {
            for (int j = 0; j < Game::BOARD_TILEMAP_HEIGHT; ++j) {
                if (m_game->getCell(i, j) != Game::EMPTY_CELL) {
                    drawTile(BOARD_X + TILE_SIZE * i,
                             BOARD_Y + TILE_SIZE * j,
                             m_game->getCell(i, j),
                             false);
                }
            }
        }
        
        //在BOARD中绘制falling tetrimino
        for (int i = 0; i < Game::TETROMINO_SIZE; ++i) {
            for (int j = 0; j < Game::TETROMINO_SIZE; ++j) {
                if (m_game->fallingBlock().cells[i][j] != Game::EMPTY_CELL) {
                    drawTile(BOARD_X + (TILE_SIZE * (m_game->fallingBlock().x + i)),
                             BOARD_Y + (TILE_SIZE * m_game->fallingBlock().y + j),
                             m_game->fallingBlock().cells[i][j],
                             false);
                }
            }
        }
        
        // Draw game statistic data
		if (!m_game->isPaused())
		{
			drawNumber(LEVEL_X, LEVEL_Y, m_game->stats().level, LEVEL_LENGTH, Game::COLOR_WHITE);
			drawNumber(LINES_X, LINES_Y, m_game->stats().lines, LINES_LENGTH, Game::COLOR_WHITE);
			drawNumber(SCORE_X, SCORE_Y, m_game->stats().score, SCORE_LENGTH, Game::COLOR_WHITE);
            
//			drawNumber(TETROMINO_X, TETROMINO_L_Y, m_game->stats().pieces[Game::TETROMINO_L], TETROMINO_LENGTH, Game::COLOR_ORANGE);
//			drawNumber(TETROMINO_X, TETROMINO_I_Y, m_game->stats().pieces[Game::TETROMINO_I], TETROMINO_LENGTH, Game::COLOR_CYAN);
//			drawNumber(TETROMINO_X, TETROMINO_T_Y, m_game->stats().pieces[Game::TETROMINO_T], TETROMINO_LENGTH, Game::COLOR_PURPLE);
//			drawNumber(TETROMINO_X, TETROMINO_S_Y, m_game->stats().pieces[Game::TETROMINO_S], TETROMINO_LENGTH, Game::COLOR_GREEN);
//			drawNumber(TETROMINO_X, TETROMINO_Z_Y, m_game->stats().pieces[Game::TETROMINO_Z], TETROMINO_LENGTH, Game::COLOR_RED);
//			drawNumber(TETROMINO_X, TETROMINO_O_Y, m_game->stats().pieces[Game::TETROMINO_O], TETROMINO_LENGTH, Game::COLOR_YELLOW);
//			drawNumber(TETROMINO_X, TETROMINO_J_Y, m_game->stats().pieces[Game::TETROMINO_J], TETROMINO_LENGTH, Game::COLOR_BLUE);
            
//			drawNumber(PIECES_X, PIECES_Y, m_game->stats().totalPieces, PIECES_LENGTH, Game::COLOR_WHITE);
		}
        
        // Inform the game that we are done with the changed state
		m_game->onChangeProcessed();
        
        //绘制完毕
		m_canvas->end();
    }
    for (int i = 0; i < m_game->BOARD_TILEMAP_WIDTH; ++i) {
        for (int j = 0; j < m_game->BOARD_TILEMAP_HEIGHT; ++j) {
            if (m_game->mMap[i][j] != m_game->EMPTY_CELL) {
                std::cout<<"o ";
            }
            else
                std::cout<<"x ";
        }
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
    std::cout<<std::endl;
}

void PlatformCocos2DX::onLineCompleted()
{
    
}

void PlatformCocos2DX::onPieceDrop()
{
    
}