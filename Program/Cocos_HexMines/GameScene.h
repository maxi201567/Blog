#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "cocos2d.h"

#pragma once
class GameScene : public cocos2d::Scene
{
private:
#define minecount 32
	int map[16][16] = { 0 };
	int minearr[minecount][2];
	int maptask[16][16] = { 0 };
	bool bType = 0;
	bool bPause = 0;
	int opencount = 0;
public:
	static cocos2d::Scene* createScene();

	virtual bool init();
	void initMenu();
	// a selector callback
	void menuCallback(cocos2d::Ref* pSender);
	void keysCallback(cocos2d::Ref* pSender);
	int recursionErgodic(int x, int y, Node* parent);

	// implement the "static create()" method manually
	CREATE_FUNC(GameScene);
};

#endif
