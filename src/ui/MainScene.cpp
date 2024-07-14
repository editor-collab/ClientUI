#include <ui/MainScene.hpp>
#include <ui/ExploreMenu.hpp>
#include <random>

using namespace tulip::editor;
using namespace geode::prelude;

MainScene::MainScene() = default;
MainScene::~MainScene() = default;

MainScene* MainScene::create() {
    auto ret = new (std::nothrow) MainScene();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

CCScene* MainScene::scene() {
    auto scene = CCScene::create();
    auto layer = MainScene::create();
    scene->addChild(layer);
    return scene;
}

void MainScene::keyBackClicked() {
    auto scene = CreatorLayer::scene();
    CCDirector::sharedDirector()->replaceScene(CCTransitionFade::create(0.5, scene));
}

bool MainScene::init() {
    if (!CCLayerColor::init()) {
        return false;
    }

    // EditorLoop::get()->m_requestedDiscovery = false;
    // ClientBridge::get()->connectToServer();

    auto const winSize = CCDirector::sharedDirector()->getWinSize();

    this->setAnchorPoint(ccp(0, 0));
    this->setPosition(ccp(0, 0));
    this->setContentSize(winSize);

    auto menu = CCMenu::create();
    menu->setID("menu");
    menu->setContentSize(winSize);
    menu->setAnchorPoint(ccp(0.5, 0.5));
    menu->setZOrder(3);
    this->addChildAtPosition(menu, Anchor::Center, ccp(0, 0));

    auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
    auto backButton = CCMenuItemSpriteExtra::create(
        backSprite, this, menu_selector(MainScene::onClose)
    );
    menu->addChildAtPosition(backButton, Anchor::TopLeft, ccp(25, -25));

    auto accountSprite = CCSprite::createWithSpriteFrameName("LoginButton.png"_spr);
	auto accountButton = CCMenuItemSpriteExtra::create(accountSprite, this, menu_selector(MainScene::onAccount));
	accountButton->setID("account-button");
	menu->addChildAtPosition(accountButton, Anchor::TopRight, ccp(-50.f, -70.f));

    auto discordSprite = CCSprite::createWithSpriteFrameName("DiscordButton.png"_spr);
	auto discordButton = CCMenuItemSpriteExtra::create(discordSprite, this, menu_selector(MainScene::onDiscord));
	discordButton->setID("discord-button");
	menu->addChildAtPosition(discordButton, Anchor::TopLeft, ccp(50.f, -70.f));

    auto cornerRed = CCSprite::createWithSpriteFrameName("CornerRed.png"_spr);
    cornerRed->setAnchorPoint(ccp(0, 1));
    cornerRed->setID("corner-red");
    this->addChildAtPosition(cornerRed, Anchor::TopLeft, ccp(0, 0));

    auto cornerGreen = CCSprite::createWithSpriteFrameName("CornerGreen.png"_spr);
    cornerGreen->setAnchorPoint(ccp(1, 1));
    cornerGreen->setID("corner-green");
    this->addChildAtPosition(cornerGreen, Anchor::TopRight, ccp(0, 0));

    auto cornerPurple = CCSprite::createWithSpriteFrameName("CornerPurple.png"_spr);
    cornerPurple->setAnchorPoint(ccp(0, 0));
    cornerPurple->setID("corner-purple");
    this->addChildAtPosition(cornerPurple, Anchor::BottomLeft, ccp(0, 0));

    auto cornerCyan = CCSprite::createWithSpriteFrameName("CornerCyan.png"_spr);
    cornerCyan->setAnchorPoint(ccp(1, 0));
    cornerCyan->setID("corner-cyan");
    this->addChildAtPosition(cornerCyan, Anchor::BottomRight, ccp(0, 0));

    m_background = CCSprite::create(fmt::format("game_bg_{:02}_001.png", GameManager::get()->m_loadedBgID).c_str());
    m_background->setID("background");
    m_background->setColor(ccc3(20, 20, 20));
    m_background->setAnchorPoint(ccp(0, 0));
    ccTexParams params = {GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT};
    m_background->getTexture()->setTexParameters(&params);
    m_background->setTextureRect({0, 0, 1536, 512});
    m_background->setContentSize({1536, 512});
    this->addChild(m_background, -100);

    auto listContainer = CCLayerColor::create(ccc4(20, 20, 20, 255));
    listContainer->setID("list-container");
    listContainer->setContentSize({380, 205});
    listContainer->setAnchorPoint(ccp(0.5, 0.5));
    this->addChildAtPosition(listContainer, Anchor::Center, ccp(0, -10));

    m_listTop1 = CCSprite::createWithSpriteFrameName("ListTop1.png"_spr);
    m_listTop1->setID("list-top-1");
    m_listTop1->setAnchorPoint(ccp(0.5, 0));
    m_listTop1->setZOrder(2);
    listContainer->addChildAtPosition(m_listTop1, Anchor::Top, ccp(0, -11));

    m_listTop2 = CCSprite::createWithSpriteFrameName("ListTop2.png"_spr);
    m_listTop2->setID("list-top-2");
    m_listTop2->setAnchorPoint(ccp(0.5, 0));
    m_listTop2->setZOrder(2);
    m_listTop2->setVisible(false);
    listContainer->addChildAtPosition(m_listTop2, Anchor::Top, ccp(0, -11));

    auto tabMenu = CCMenu::create();
    tabMenu->setID("tab-menu");
    tabMenu->setAnchorPoint(ccp(0.5, 0));
    tabMenu->setContentSize(m_listTop1->getContentSize());
    tabMenu->setZOrder(2);
    listContainer->addChildAtPosition(tabMenu, Anchor::Top, ccp(0, -11));

    auto tab1 = CCNode::create();
    tab1->setContentSize({120, 45});
    auto tab1Button = CCMenuItemSpriteExtra::create(tab1, this, menu_selector(MainScene::onTab));
    tab1Button->setAnchorPoint(ccp(1, 0.5));
    tab1Button->setTag(1);
    tabMenu->addChildAtPosition(tab1Button, Anchor::Center, ccp(-20, 5));

    auto tab2 = CCNode::create();
    tab2->setContentSize({120, 45});
    auto tab2Button = CCMenuItemSpriteExtra::create(tab2, this, menu_selector(MainScene::onTab));
    tab2Button->setAnchorPoint(ccp(0, 0.5));
    tab2Button->setTag(2);
    tabMenu->addChildAtPosition(tab2Button, Anchor::Center, ccp(20, 5));

    auto listBottom = CCSprite::createWithSpriteFrameName("ListBottom.png"_spr);
    listBottom->setID("list-bottom");
    listBottom->setAnchorPoint(ccp(0.5, 1));
    listBottom->setZOrder(2);
    listContainer->addChildAtPosition(listBottom, Anchor::Bottom, ccp(0, 9));

    auto listLeft = CCSprite::createWithSpriteFrameName("ListSide.png"_spr);
    listLeft->setID("list-left");
    listLeft->setAnchorPoint(ccp(0.5, 0.5));
    listLeft->setZOrder(2);
    listLeft->setScaleY(listContainer->getContentHeight() / listLeft->getContentHeight());
    listContainer->addChildAtPosition(listLeft, Anchor::Left, ccp(6, 0));

    auto listRight = CCSprite::createWithSpriteFrameName("ListSide.png"_spr);
    listRight->setID("list-right");
    listRight->setAnchorPoint(ccp(0.5, 0.5));
    listRight->setFlipX(true);
    listRight->setZOrder(2);
    listRight->setScaleY(listContainer->getContentHeight() / listRight->getContentHeight());
    listContainer->addChildAtPosition(listRight, Anchor::Right, ccp(-6, 0));

    m_explore = ExploreMenu::create({356, 205}, false);
    m_explore->setID("explore-menu");
    m_explore->setAnchorPoint(ccp(0.5, 0.5));
    m_explore->setZOrder(1);
    listContainer->addChildAtPosition(m_explore, Anchor::Center, ccp(0, 0));

    this->setKeypadEnabled(true);
    this->scheduleUpdate();

    return true;
}

void MainScene::update(float dt) {
    m_background->setPositionX(m_background->getPositionX() - dt * 20);
    if (m_background->getPositionX() < -512) {
        m_background->setPositionX(0);
    }
}

void MainScene::onTab(CCObject* sender) {
    auto tab = typeinfo_cast<CCMenuItemSprite*>(sender);
    if (tab->getTag() == 1) {
        m_listTop1->setVisible(true);
        m_listTop2->setVisible(false);
    } else {
        m_listTop1->setVisible(false);
        m_listTop2->setVisible(true);
    }
    m_isExplore = tab->getTag() == 2;
    this->recreateMenu();
}

void MainScene::onAccount(cocos2d::CCObject* sender) {
	// SetupAccountLayer::create()->show();
}

void MainScene::onDiscord(cocos2d::CCObject* sender) {
	AppDelegate::get()->openURL("https://discord.gg/GFMnMMkpBq");
}

void MainScene::recreateMenu() {
    auto listContainer = m_explore->getParent();
    m_explore->removeFromParent();
    m_explore = ExploreMenu::create({356, 205}, m_isExplore);
    m_explore->setID("explore-menu");
    m_explore->setAnchorPoint(ccp(0.5, 0.5));
    m_explore->setZOrder(1);
    listContainer->addChildAtPosition(m_explore, Anchor::Center, ccp(0, 0));
}

void MainScene::onClose(CCObject* sender) {
    this->keyBackClicked();
}