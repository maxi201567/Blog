#include "GameScene.h"
#include "SimpleAudioEngine.h"
#include "AudioEngine.h"
#include "HelloWorldScene.h"

USING_NS_CC;
using namespace experimental;//AudioEngine


Scene* GameScene::createScene()
{
	return GameScene::create();
}

bool GameScene::init()
{
	if (!Scene::init())
	{
		return false;
	}
	initMenu();
	for (int i = 0; i < minecount; i++)
	{
		int x = random() % 16;
		int y = random() % 16;
		if (map[y][x] == 1)
			i--;
		else
		{
			minearr[i][0] = x;
			minearr[i][1] = y;
			map[y][x] = 1;
		}
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	auto gamepad = Menu::create();
	gamepad->setAnchorPoint(Vec2(0, 0));
	gamepad->setPosition(16, 72);
	MenuItemImage* key[16][16];
	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 16; j++)
		{
			key[i][j] = MenuItemImage::create("keyUp.png", "keyDown.png", "keyDown.png", CC_CALLBACK_1(GameScene::keysCallback, this));
			key[i][j]->setAnchorPoint(Vec2(0, 1));
			key[i][j]->setScale(0.8f);
			key[i][j]->setPosition(Vec2(j * 36 + (i % 2) * 18, i * 11));
			auto keytext = Label::createWithTTF(" ", "fonts/Marker Felt.ttf", 16);
			keytext->setAnchorPoint(Vec2(0.5f, 0.5f));
			keytext->setPosition(Vec2(key[i][j]->getContentSize().width / 2, key[i][j]->getContentSize().height / 2));
			keytext->setName("text");
			key[i][j]->addChild(keytext);
			key[i][j]->setTag(i * 16 + j);
			gamepad->addChild(key[i][j], 30000 + i * 16 + j);
		}
	this->addChild(gamepad, 1);
	return true;
}

void GameScene::menuCallback(cocos2d::Ref* pSender)
{
	int id = dynamic_cast<MenuItem*>(pSender)->getLocalZOrder() - 20000;
	switch (id)
	{
	case 4://ExitButton
	{
		auto newscene = HelloWorld::createScene();
		auto trans = TransitionFade::create(1.0f, newscene);
		Director::getInstance()->replaceScene(trans);
		break;
	}
	case 0://PauseButton
	{
		dynamic_cast<MenuItem*>(pSender)->getParent()->getChildByName("ExitButton")->setVisible(true);
		dynamic_cast<MenuItem*>(pSender)->getParent()->getChildByName("ResumeButton")->setVisible(true);
		dynamic_cast<MenuItem*>(pSender)->getParent()->getChildByName("Tips")->setVisible(true);
		dynamic_cast<MenuItem*>(pSender)->setVisible(false);
		bPause = 1;
		break;
	}
	case 3://ResumeButton
	{
		dynamic_cast<MenuItem*>(pSender)->getParent()->getChildByName("PauseButton")->setVisible(true);
		dynamic_cast<MenuItem*>(pSender)->getParent()->getChildByName("ExitButton")->setVisible(false);
		dynamic_cast<MenuItem*>(pSender)->getParent()->getChildByName("Tips")->setVisible(false);
		dynamic_cast<MenuItem*>(pSender)->setVisible(false);
		bPause = 0;
		break;
	}
	case 1:
		bType = !bType;
		if (bType)
			dynamic_cast<Label*>(dynamic_cast<MenuItem*>(pSender)->getChildByName("text"))->setString("Tag");
		else
			dynamic_cast<Label*>(dynamic_cast<MenuItem*>(pSender)->getChildByName("text"))->setString("Dig");

		break;
	default:
		break;
	}
}
void GameScene::keysCallback(cocos2d::Ref* pSender)
{
	int id = dynamic_cast<MenuItem*>(pSender)->getLocalZOrder() - 30000;
	if (bPause)
		return;
	if (bType == 0)
	{
		int y = id / 16;
		int x = id % 16;
		for (int i = 0; i < minecount; i++)
			if (x == minearr[i][0] && y == minearr[i][1])//踩雷了
			{
				for (int j = 0; j < minecount; j++)
					dynamic_cast<Label*>(dynamic_cast<MenuItem*>(pSender)->getParent()->getChildByTag(minearr[j][0] + minearr[j][1] * 16)->getChildByName("text"))->setString("X");
				dynamic_cast<Menu*>(this->getChildByTag(0))->getChildByName("ExitButton")->setVisible(true);
				dynamic_cast<MenuItemLabel*>(dynamic_cast<Menu*>(this->getChildByTag(0))->getChildByName("Tips"))->setString("Mine Explode!");
				dynamic_cast<Menu*>(this->getChildByTag(0))->getChildByName("Tips")->setVisible(true);
				dynamic_cast<Menu*>(this->getChildByTag(0))->getChildByName("PauseButton")->setVisible(false);
				bPause = 1;
				return;
			}
		dynamic_cast<MenuItem*>(pSender)->setEnabled(false);
		recursionErgodic(x, y, dynamic_cast<MenuItem*>(pSender)->getParent());
	}
	else if (bType == 1)
	{
		dynamic_cast<Label*>(dynamic_cast<MenuItem*>(pSender)->getChildByName("text"))->setString("T");
	}
}
void GameScene::initMenu()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto menu = Menu::create();
	menu->setAnchorPoint(Vec2(0, 0));
	menu->setPosition(Vec2(0, 0));

	auto pauseItem = MenuItemImage::create(
		"normal.png",
		"click.png",
		CC_CALLBACK_1(GameScene::menuCallback, this));
	pauseItem->setAnchorPoint(Vec2(1, 1));
	pauseItem->setPosition(Vec2(visibleSize.width, visibleSize.height));
	pauseItem->setScale(0.5f);
	auto text0 = Label::createWithTTF("Pause", "fonts/Marker Felt.ttf", 32);
	text0->setAnchorPoint(Vec2(0.5, 0.5));
	text0->setPosition(Vec2(pauseItem->getContentSize().width / 2, pauseItem->getContentSize().height / 2));
	pauseItem->addChild(text0);

	auto closeItem = MenuItemImage::create(
		"normal.png",
		"click.png",
		CC_CALLBACK_1(GameScene::menuCallback, this));
	closeItem->setAnchorPoint(Vec2(0.5, 1));
	closeItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	closeItem->setScale(1.0f, 0.7f);
	auto text1 = Label::createWithTTF("Exit To Menu", "fonts/Marker Felt.ttf", 24);
	text1->setAnchorPoint(Vec2(0.5, 0.5));
	text1->setPosition(Vec2(closeItem->getContentSize().width / 2, closeItem->getContentSize().height / 2));
	closeItem->addChild(text1);

	auto resumeItem = MenuItemImage::create(
		"normal.png",
		"click.png",
		CC_CALLBACK_1(GameScene::menuCallback, this));
	resumeItem->setAnchorPoint(Vec2(0.5, 0));
	resumeItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2));
	resumeItem->setScale(1.0f, 0.7f);
	auto text2 = Label::createWithTTF("Resume To Game", "fonts/Marker Felt.ttf", 24);
	text2->setAnchorPoint(Vec2(0.5, 0.5));
	text2->setPosition(Vec2(resumeItem->getContentSize().width / 2, resumeItem->getContentSize().height / 2));
	resumeItem->addChild(text2);

	auto text3 = MenuItemLabel::create(Label::createWithTTF("Game is Paused!", "fonts/Marker Felt.ttf", 48));
	text3->setAnchorPoint(Vec2(0.5, 1));
	text3->setPosition(Vec2(visibleSize.width / 2, visibleSize.height - 32));

	text3->setName("Tips");
	text3->setVisible(false);
	closeItem->setName("ExitButton");
	closeItem->setVisible(false);
	resumeItem->setName("ResumeButton");
	resumeItem->setVisible(false);
	pauseItem->setName("PauseButton");

	auto typeItem = MenuItemImage::create(
		"normal.png",
		"click.png",
		CC_CALLBACK_1(GameScene::menuCallback, this));
	typeItem->setAnchorPoint(Vec2(0.5, 0));
	typeItem->setPosition(Vec2(visibleSize.width / 2, 0));
	typeItem->setScale(0.8f, 0.7f);
	auto text4 = Label::createWithTTF("Dig", "fonts/Marker Felt.ttf", 24);
	text4->setAnchorPoint(Vec2(0.5, 0.5));
	text4->setPosition(Vec2(typeItem->getContentSize().width / 2, typeItem->getContentSize().height / 2));
	typeItem->addChild(text4);
	typeItem->setName("TypeButton");
	text4->setName("text");

	menu->addChild(text3, 20005);
	menu->addChild(closeItem, 20004);
	menu->addChild(resumeItem, 20003);
	menu->addChild(pauseItem, 20000);
	menu->addChild(typeItem, 20001);
	menu->setTag(0);
	this->addChild(menu, 3);

}
int GameScene::recursionErgodic(int x, int y, Node* parent)//递归便利
{
	if (maptask[y][x] == 1)
		return 0;//标记，已翻开或者未翻开
	if (x < 0 || x > 15 || y < 0 || y > 15)//越界
		return 0;
	if (map[y][x] == 1)//发现雷
		return 1;
	else if (map[y][x] == 0)
	{
		opencount++;
		maptask[y][x] = 1;
		int count = 0;
		if (y % 2 == 1)
		{
			if (y + 1 < 16)
				count += map[y + 1][x];
			if (y - 1 >= 0)
				count += map[y - 1][x];
			if (y + 1 < 16 && x + 1 < 16)
				count += map[y + 1][x + 1];
			if (y - 1 >= 0 && x + 1 < 16)
				count += map[y - 1][x + 1];
			if (y + 2 < 16)
				count += map[y + 2][x];
			if (y - 2 >= 0)
				count += map[y - 2][x];
			//附近是否有雷
			if (count == 0)
			{
				count = recursionErgodic(x, y + 1, parent) +
					recursionErgodic(x, y - 1, parent) +
					recursionErgodic(x + 1, y + 1, parent) +
					recursionErgodic(x + 1, y - 1, parent) +
					recursionErgodic(x, y + 2, parent) +
					recursionErgodic(x, y - 2, parent);
			}
		}
		else
		{
			if (y + 1 < 16)
				count += map[y + 1][x];
			if (y - 1 > 0)
				count += map[y - 1][x];
			if (y + 1 < 16 && x - 1 >= 0)
				count += map[y + 1][x - 1];
			if (y - 1 >= 0 && x - 1 >= 0)
				count += map[y - 1][x - 1];
			if (y + 2 < 16)
				count += map[y + 2][x];
			if (y - 2 >= 0)
				count += map[y - 2][x];
			if (count == 0)
			{
				count = recursionErgodic(x - 1, y + 1, parent) +
					recursionErgodic(x - 1, y - 1, parent) +
					recursionErgodic(x, y + 1, parent) +
					recursionErgodic(x, y - 1, parent) +
					recursionErgodic(x, y + 2, parent) +
					recursionErgodic(x, y - 2, parent);
			}
		}
		if (count > 6) count = 0;
		auto item = dynamic_cast<MenuItem*>(parent->getChildByTag(y * 16 + x));
		item->setEnabled(false);
		char t[16];
		if (count > 0)
		{
			sprintf(t, "%d", count);
			dynamic_cast<Label*>(item->getChildByName("text"))->setString(t);
		}
		else
			dynamic_cast<Label*>(item->getChildByName("text"))->setString(" ");
		if (opencount == 256 - minecount)
		{
		//	for (int j = 0; j < minecount; j++)
		//		dynamic_cast<Label*>(dynamic_cast<MenuItem*>(pSender)->getParent()->getChildByTag(minearr[j][0] + minearr[j][1] * 16)->getChildByName("text"))->setString("X");
			dynamic_cast<Menu*>(this->getChildByTag(0))->getChildByName("ExitButton")->setVisible(true);
			dynamic_cast<MenuItemLabel*>(dynamic_cast<Menu*>(this->getChildByTag(0))->getChildByName("Tips"))->setString("You Win!");
			dynamic_cast<Menu*>(this->getChildByTag(0))->getChildByName("Tips")->setVisible(true);
			dynamic_cast<Menu*>(this->getChildByTag(0))->getChildByName("PauseButton")->setVisible(false);
			bPause = 1;
		}
		return 0;
	}
	return 0;
}
