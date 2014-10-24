//
//  platform.h
//  RussianCube
//
//  Created by 陈逸宇 on 6/15/14.
//
//

#ifndef RussianCube_platform_h
#define RussianCube_platform_h

class Game;
class Platform
{
public:
    virtual int init(Game *game) = 0;
    virtual void end() = 0;
    virtual void processEvents() = 0;
    virtual void renderGame() = 0;
    virtual long getSystemTime() = 0;
    virtual int random() = 0;
    virtual void onLineCompleted() = 0;
    virtual void onPieceDrop() = 0;
};

#endif
