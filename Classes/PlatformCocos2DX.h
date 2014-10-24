//
//  PlatformCocos2DX.h
//  RussianCube
//
//  Created by 陈逸宇 on 6/17/14.
//
//

#ifndef RussianCube_PlatformCocos2DX_h
#define RussianCube_PlatformCocos2DX_h

#include "cocos2d.h"
#include "game.h"

#endif

class PlatformCocos2DX : public cocos2d::LayerColor, public Platform
{
    //屏幕大小
    static const int BACKGROUND_WIDTH = 640;
    static const int BACKGROUND_HEIGHT = 1136;
    
    //一个方格的大小
    static const int TILE_SIZE = 40;
    
    //棋盘左上坐标
    static const int BOARD_X = 20;
    static const int BOARD_Y = 100;
    
    //下一个形状展示位置
    static const int PREVIEW_X = 0;
    static const int PREVIEW_Y = 10;
    
    
	// Score position and length on screen
	static const int SCORE_X      = 320;
	static const int SCORE_Y      = 10;
	static const int SCORE_LENGTH = 30;
    
	// Lines position and length on screen
	static const int LINES_X      = 500;
	static const int LINES_Y      = 10;
	static const int LINES_LENGTH = 30;
    
	// Level position and length on screen
	static const int LEVEL_X      = 550;
	static const int LEVEL_Y      = 10;
	static const int LEVEL_LENGTH = 20;
    
	// Tetromino subtotals position
	static const int TETROMINO_X   = 425;
	static const int TETROMINO_L_Y = 53;
	static const int TETROMINO_I_Y = 77;
	static const int TETROMINO_T_Y = 101;
	static const int TETROMINO_S_Y = 125;
	static const int TETROMINO_Z_Y = 149;
	static const int TETROMINO_O_Y = 173;
	static const int TETROMINO_J_Y = 197;
    
	// Size of subtotals
	static const int TETROMINO_LENGTH = 5;
    
	// Tetromino total position
	static const int PIECES_X      = 418;
	static const int PIECES_Y      = 221;
	static const int PIECES_LENGTH = 6;
    
	// Size of number
	static const int NUMBER_WIDTH  = 7;
	static const int NUMBER_HEIGHT = 9;
    
	// Touch zones limits
	static const int TX_1 = 160;
	static const int TX_2 = 320;
    
	static const int TY_DROP = 250;
	static const int TY_DOWN = 70;
    
	static const int TY_1 = 50;
	static const int TY_2 = 270;
    
public:
    
	PlatformCocos2DX();
	~PlatformCocos2DX();
    
	virtual int init(Game* game);
	virtual void end();
	virtual void processEvents();
	virtual void renderGame();
	virtual long getSystemTime();
	virtual int random();
    virtual void onLineCompleted();
    virtual void onPieceDrop();

	// Cocos2d-x init function.
    virtual bool init();
    
	// Return the scene pointer.
    static cocos2d::Scene* scene();
    
    // Callback for the "close" menu.
    virtual void menuCloseCallback(cocos2d::Object* pSender);
    
	// Touch events.
	void TouchesEnded(cocos2d::Set * touches, cocos2d::Event * event);
	void TouchesBegan(cocos2d::Set * touches, cocos2d::Event * event);
    
	// Platform cocos2d-x update.
	void update(float f);
    
    //Implement the "static node()" method manually.
    CREATE_FUNC(PlatformCocos2DX);
    
private:
    
	void drawTile(int x, int y, int tile, bool shadow);
	void drawNumber(int x, int y, long number, int length, int color);
    
	long       m_elapsedTime; // Used to store the time passed.
	int        m_yOffset;     // Used to correct rendering positions.
    
	Game* m_game;  // The game instance.
    
	// Array of sprites for drawing, there is one more color than tetromino types.
	cocos2d::Sprite* m_tiles[Game::TETROMINO_TYPES + 1];
	cocos2d::Sprite* m_shadows[Game::TETROMINO_TYPES + 1];
	cocos2d::Sprite* m_numbers[Game::TETROMINO_TYPES + 1][10];
    
	// We draw the game state over this rendering target object.
	cocos2d::RenderTexture *m_canvas;

    
	std::vector<cocos2d::Sprite*> m_board;     // Board sprites.
	std::vector<cocos2d::Sprite*> m_tetromino; // Falling tetromino sprites.
	std::vector<cocos2d::Sprite*> m_shadow;    // Shadow tetromino sprites.
	std::vector<cocos2d::Sprite*> m_next;      // Next tetromino sprites.
    
	cocos2d::Sprite* getTile(int x, int y, int id, bool shadow);
};