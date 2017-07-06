#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "GameScene.h"
USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Scene::init())
	{
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	/////////////////////////////
	// 2. add a menu item with "X" image, which is clicked to quit the program
	//    you may modify it.

	// add a "close" icon to exit the progress. it's an autorelease object
	auto menu = Menu::create();

	auto closeItem = MenuItemImage::create(
		"normal.png",
		"click.png",
		CC_CALLBACK_1(HelloWorld::menuCallback, this));

	auto text1 = Label::createWithTTF("Exit", "fonts/Marker Felt.ttf", 32);
	text1->setAnchorPoint(Vec2(0.5, 0.5));
	text1->setPosition(Vec2(closeItem->getContentSize().width / 2, closeItem->getContentSize().height / 2));
	closeItem->addChild(text1);

	auto startItem = MenuItemImage::create(
		"normal.png",
		"click.png",
		CC_CALLBACK_1(HelloWorld::menuCallback, this));

	auto text0 = Label::createWithTTF("Start", "fonts/Marker Felt.ttf", 32);
	text0->setAnchorPoint(Vec2(0.5, 0.5));
	text0->setPosition(Vec2(startItem->getContentSize().width / 2, startItem->getContentSize().height / 2));
	startItem->addChild(text0);

	menu->addChild(startItem, 10000);
	menu->addChild(closeItem, 10001);
	menu->alignItemsVertically();
	menu->setAnchorPoint(Vec2(0.5, 1));
	menu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height/2));
	this->addChild(menu, 1);
	/////////////////////////////
	// 3. add your codes below...

	// add a label shows "Hello World"
	// create and initialize a label
	auto label = Label::createWithTTF("Hex Mines", "fonts/Marker Felt.ttf", 64);
	// position the label on the center of the screen
	label->setPosition(Vec2(origin.x + visibleSize.width / 2,
		origin.y + visibleSize.height - label->getContentSize().height));
	//label->enableUnderline();
	// add the label as a child to this layer
	this->addChild(label, 1);

	auto sprite = Sprite::create("Band.png");
	// position the sprite on the center of the screen
	sprite->setAnchorPoint(Vec2(1, 0));
	sprite->setPosition(Vec2(visibleSize.width - 10, 10));
	// add the sprite as a child to this layer
	this->addChild(sprite, 0);

	return true;
}


void HelloWorld::menuCallback(Ref* pSender)
{
	int id = dynamic_cast<MenuItem*>(pSender)->getLocalZOrder() - 10000;
	switch (id)
	{
	case 0:
	{
		auto newscene = GameScene::createScene();
		auto trans = TransitionFade::create(1.0f, newscene);
		Director::getInstance()->replaceScene(trans);
		break;
	}
	case 1:
		Director::getInstance()->end();
		break;
	default:
		break;
	}
    //Close the cocos2d-x game scene and quit the application
    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}
