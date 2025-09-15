#include <ui/ExploreMenu.hpp>
#include <ui/GenericList.hpp>
#include <ui/MainScene.hpp>
#include <data/DiscoverableLevel.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

ExploreMenu* ExploreMenu::create(cocos2d::CCSize const& size, bool isExplore) {
    auto ret = new (std::nothrow) ExploreMenu();
    if (ret && ret->init(size, isExplore)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool ExploreMenu::init(cocos2d::CCSize const& size, bool isExplore) {
    if (!CCNode::init()) {
        return false;
    }

    if (isExplore) {
        // ClientBridge::get()->sendDiscoverableLevelsRequest();
    }

    m_isExplore = isExplore;
    this->setContentSize(size);
    if (isExplore) {
        this->createExploreBanner({size.width, 40.f});
    }
    else {
        // if (ClientBridge::get()->isHostUser()) {
        //     if (EditorLoop::get()->m_sharedLevel.m_levelKey == LevelKey()) {
        //         this->createCreateBanner({size.width, 40.f});
        //     }
        //     else {
        //         this->createHostBanner({size.width, 40.f});
        //     }
        // }
        // else {
            this->createPromoBanner({size.width, 40.f});
        // }
    }
    this->createList({size.width, size.height - 40.f}, isExplore);

    return true;
}

void ExploreMenu::onJoinLevel(cocos2d::CCObject* sender) {
    // auto idx = static_cast<CCMenuItem*>(sender)->getTag();
    // auto& list = m_isExplore ? EditorLoop::get()->m_discoverableLevels : EditorLoop::get()->m_lastLevels;

    // if (idx < 0 || idx >= list.size()) return;

    // auto& item = list[idx];

    // ClientBridge::get()->sendJoinLevelRequest(
	//     LevelKeyImpl::toString(item.m_levelKey)
	// );
}

void ExploreMenu::onJoinCode(cocos2d::CCObject* sender) {
    // auto code = m_textInput->getString();

	// if (code.size() == 8) {
	// 	ClientBridge::get()->sendJoinLevelRequest(code);
	// }
	// else {
	// 	EditorLoop::get()->spawnAlert("Code is not 8 characters long");
	// }
}

void ExploreMenu::onPromo(cocos2d::CCObject* sender) {
    AppDelegate::get()->openURL("https://buy.stripe.com/aEUbLb38R2Cw91K9AA");
}

void ExploreMenu::onJoinHosted(cocos2d::CCObject* sender) {
    // ClientBridge::get()->sendHostJoinLevelRequest();
}

void ExploreMenu::onDeleteLast(cocos2d::CCObject* sender) {
    // auto idx = static_cast<CCMenuItem*>(sender)->getTag();
    // auto& list = EditorLoop::get()->m_lastLevels;

    // if (idx < 0 || idx >= list.size()) return;

    // list.erase(list.begin() + idx);

    // Loader::get()->queueInMainThread([](){
    //     EditorLoop::get()->m_tulipScene->recreateMenu();
    // });
}

void ExploreMenu::onOpenBrowser(cocos2d::CCObject* sender) {
    auto searchObject = GJSearchObject::create(SearchType::MyLevels);
    searchObject->m_page = std::min<int>(GameManager::get()->getIntGameVariable("0091"), 999);
    CCDirector::get()->pushScene(CCTransitionFade::create(.5f, 
        LevelBrowserLayer::scene(searchObject)
    ));
}

void ExploreMenu::createBanner(cocos2d::CCSize const& size) {
    auto bg = CCLayerColor::create(ccc4(100, 97, 13, 255));
    bg->setContentSize(size);
    bg->setID("banner");
    this->addChildAtPosition(bg, Anchor::TopLeft, ccp(0, -size.height));

    m_menu = CCMenu::create();
    m_menu->setAnchorPoint(ccp(0.5f, 0.5f));
    m_menu->setContentSize(size);
    m_menu->setID("banner-menu");
    m_menu->setLayout(RowLayout::create()
        ->setAxisAlignment(AxisAlignment::Center)
        ->setCrossAxisLineAlignment(AxisAlignment::Center)
        ->setGrowCrossAxis(false)
        ->setAutoScale(false)
        ->setGap(10.f));
    m_menu->setPosition(size / 2);
    bg->addChild(m_menu);
}

void ExploreMenu::createPromoBanner(cocos2d::CCSize const& size) {
    this->createBanner(size);

    auto label = CCLabelBMFont::create("Visit Stripe to get hosting!", "bigFont.fnt");
    label->limitLabelWidth(240.0, 0.6, 0.0);
    label->setAnchorPoint(ccp(0.0f, 0.5f));
    label->setID("promo-label");
    m_menu->addChild(label);

    auto enterSprite = ButtonSprite::create("View", 60, true, "bigFont.fnt", "ButtonBG1.png"_spr, 28.0f, 0.7f);
    auto enterButton = CCMenuItemSpriteExtra::create(
        enterSprite, this, menu_selector(ExploreMenu::onPromo)
    );
    enterButton->setID("promo-button");

    auto inputWidth = size.width - label->getScaledContentSize().width - enterButton->getScaledContentSize().width - 40.f;
    auto pad = CCNode::create();
    pad->setContentSize({inputWidth, 1});
    m_menu->addChild(pad);
    m_menu->addChild(enterButton);

    m_menu->updateLayout();
}

void ExploreMenu::createCreateBanner(cocos2d::CCSize const& size) {
    this->createBanner(size);

    auto label = CCLabelBMFont::create("Open a level and share it!", "bigFont.fnt");
    label->limitLabelWidth(240.0, 0.6, 0.0);
    label->setAnchorPoint(CCPointMake(0.0f, 0.0f));
    label->setID("create-level-label");
    m_menu->addChild(label);

    auto enterSprite = ButtonSprite::create("Open", 60, true, "bigFont.fnt", "ButtonBG1.png"_spr, 28.0f, 0.7f);
    auto enterButton = CCMenuItemSpriteExtra::create(
        enterSprite, this, menu_selector(ExploreMenu::onOpenBrowser)
    );
    enterButton->setID("promo-button");

    auto inputWidth = size.width - label->getScaledContentSize().width - enterButton->getScaledContentSize().width - 40.f;
    auto pad = CCNode::create();
    pad->setContentSize({inputWidth, 1});
    m_menu->addChild(pad);
    m_menu->addChild(enterButton);

    m_menu->updateLayout();
}

void ExploreMenu::createHostBanner(cocos2d::CCSize const& size) {
    this->createBanner(size);

    auto nameLabel = CCLabelBMFont::create("TODO"/*EditorLoop::get()->m_sharedLevel.m_levelName.c_str()*/, "bigFont.fnt");
    nameLabel->limitLabelWidth(240.0, 0.6, 0.0);
    nameLabel->setAnchorPoint(CCPointMake(0.0f, 0.0f));
    nameLabel->setID("hosted-name-label");
    m_menu->addChild(nameLabel);

    auto enterSprite = CCSprite::createWithSpriteFrameName("GJ_hammerIcon_001.png");
    enterSprite->setAnchorPoint(CCPointMake(0.5f, 0.5f));
    enterSprite->setScale(0.75f);
    auto enterSprite2 = CCScale9Sprite::create("ButtonBG1.png"_spr);
    enterSprite2->setContentSize({28.0f, 28.0f});
    enterSprite2->addChildAtPosition(enterSprite, Anchor::Center, ccp(0, 0));
    auto enterButton = CCMenuItemSpriteExtra::create(
        enterSprite2, this, menu_selector(ExploreMenu::onJoinHosted)
    );
    enterButton->setAnchorPoint(CCPointMake(0.5f, 0.5f));
    enterButton->setID("hosted-enter-button");

    auto inputWidth = size.width - nameLabel->getScaledContentSize().width - enterButton->getScaledContentSize().width - 40.f;
    auto pad = CCNode::create();
    pad->setContentSize({inputWidth, 1});
    m_menu->addChild(pad);
    m_menu->addChild(enterButton);

    m_menu->updateLayout();
}

void ExploreMenu::createExploreBanner(cocos2d::CCSize const& size) {
    this->createBanner(size);

    auto label = CCLabelBMFont::create("Enter a code:", "bigFont.fnt");
	label->setScale(0.6f);
    label->setID("enter-label");
    m_menu->addChild(label);

    m_codeLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_codeLabel->setScale(0.6f);
    m_codeLabel->setAnchorPoint(ccp(0.5f, 0.5f));
    m_codeLabel->setID("enter-code-label");

    auto enterSprite = ButtonSprite::create("OK", 40, true, "bigFont.fnt", "ButtonBG1.png"_spr, 28.0f, 0.7f);
    auto enterButton = CCMenuItemSpriteExtra::create(
        enterSprite, this, menu_selector(ExploreMenu::onJoinCode)
    );
    enterButton->setID("enter-button");
    
    auto inputWidth = size.width - label->getScaledContentSize().width - enterButton->getScaledContentSize().width - 40.f;

    m_textInput = TextInput::create(inputWidth, "Room Code");
    m_textInput->getInputNode()->setLabelPlaceholderScale(.6f);
    m_textInput->setID("enter-text-input");
    m_textInput->setCallback([&](std::string const& code) {
        std::string filtered;
        if (code.size() > 8) {
            m_textInput->setString(code.substr(0, 8));
            filtered = code.substr(0, 8);
        }
        else {
            filtered = code;
        }

        m_textInput->getInputNode()->setVisible(filtered.size() == 0);
        if (filtered.size() > 4) {
            m_codeLabel->setString((filtered.substr(0, 4) + "-" + filtered.substr(4, filtered.size())).c_str());
        }
        else {
            m_codeLabel->setString(filtered.c_str());
        }
    });
    m_textInput->addChildAtPosition(m_codeLabel, Anchor::Center, ccp(0, 0));
    m_menu->addChild(m_textInput);

    m_menu->addChild(enterButton);

    m_menu->updateLayout();
}

void ExploreMenu::createList(cocos2d::CCSize const& size, bool isExplore) {
    auto list = std::vector<DiscoverableLevel>{};
    list.push_back(DiscoverableLevel{
        LevelKey(), "Test Level", "Test Host", 0, false, 
    });
    list.push_back(DiscoverableLevel{
        LevelKey(), "Test Level 2", "Test Host 2", 0, true,
    });
    // auto& list = isExplore ? EditorLoop::get()->m_discoverableLevels : EditorLoop::get()->m_lastLevels;
    
    m_list = GenericList::create(size, [=, this](int idx) -> CCNode* {
        // auto& list = isExplore ? EditorLoop::get()->m_discoverableLevels : EditorLoop::get()->m_lastLevels;

		if (idx < 0 || idx >= list.size()) return nullptr;

        auto& item = list[idx];

		auto layer = CCLayerColor::create(idx % 2 ? ccc4(30, 30, 30, 255) : ccc4(40, 40, 40, 255));
        layer->setID("level-list-item");
		layer->setContentSize({size.width, 40.0f});

		auto nameLabel = CCLabelBMFont::create(item.m_levelName.c_str(), "bigFont.fnt");
		nameLabel->limitLabelWidth(240.0, 0.6, 0.0);
		nameLabel->setAnchorPoint(CCPointMake(0.0f, 0.0f));
        nameLabel->setID("name-label");
		layer->addChildAtPosition(nameLabel, Anchor::Left, ccp(10.0, -2.0));

        if (isExplore) {
            auto playerLabel = CCLabelBMFont::create(fmt::format("{} people online", item.m_joinedPlayers).c_str(), "bigFont.fnt");
            playerLabel->setAnchorPoint(CCPointMake(0.0f, 0.0f));
            playerLabel->limitLabelWidth(90.0, 0.3, 0.0);
            playerLabel->setColor({0, 255, 0});
            playerLabel->setID("player-label");
            layer->addChildAtPosition(playerLabel, Anchor::Left, ccp(nameLabel->getScaledContentSize().width + 15.f, 1.0));
        }
        
        auto hostLabel = CCLabelBMFont::create(("by " + item.m_hostName).c_str(), "goldFont.fnt");
		hostLabel->limitLabelWidth(240.0, 0.4, 0.0);
		hostLabel->setAnchorPoint(CCPointMake(0.0f, 1.0f));
        hostLabel->setID("host-label");
		layer->addChildAtPosition(hostLabel, Anchor::Left, ccp(10.0, -3.0));

		auto menu = CCMenu::create();
		menu->setAnchorPoint(CCSizeMake(0.5f, 0.5f));
		menu->setContentSize(CCSizeMake(346.0f, 40.0f));
        menu->setID("level-list-menu");
		layer->addChildAtPosition(menu, Anchor::Center, ccp(0, 0));

        if (!isExplore) {
            auto deleteSprite = CCSprite::createWithSpriteFrameName("GJ_deleteIcon_001.png");
            deleteSprite->setAnchorPoint(CCPointMake(0.5f, 0.5f));
            deleteSprite->setScale(0.75f);
            auto deleteSprite2 = CCScale9Sprite::create("ButtonBG1.png"_spr);
            deleteSprite2->setContentSize({28.0f, 28.0f});
            deleteSprite2->addChildAtPosition(deleteSprite, Anchor::Center, ccp(0, 0));
            auto deleteButton = CCMenuItemSpriteExtra::create(
                deleteSprite2, this, menu_selector(ExploreMenu::onDeleteLast)
            );
            deleteButton->setTag(idx);
            deleteButton->setAnchorPoint(CCPointMake(0.5f, 0.5f));
            deleteButton->setID("delete-button");
            menu->addChildAtPosition(deleteButton, Anchor::Right, ccp(-55.0f, 0.0f));
        }

        auto enterSprite = CCSprite::createWithSpriteFrameName(item.m_isEditor ? "GJ_hammerIcon_001.png" : "ViewIcon.png"_spr);
        enterSprite->setAnchorPoint(CCPointMake(0.5f, 0.5f));
        enterSprite->setScale(item.m_isEditor ? 0.75f : 0.70f);
        auto enterSprite2 = CCScale9Sprite::create("ButtonBG1.png"_spr);
        enterSprite2->setContentSize({28.0f, 28.0f});
        enterSprite2->addChildAtPosition(enterSprite, Anchor::Center, ccp(0, 0));
        auto enterButton = CCMenuItemSpriteExtra::create(
            enterSprite2, this, menu_selector(ExploreMenu::onJoinLevel)
        );
        enterButton->setTag(idx);
        enterButton->setAnchorPoint(CCPointMake(0.5f, 0.5f));
        enterButton->setID("enter-button");
        menu->addChildAtPosition(enterButton, Anchor::Right, ccp(-19.0f, 0.0f));

		return layer;
	});
    m_list->setContentSize(size);
    m_list->setLayerContentSize({size.width, std::max(size.height, 40.0f * list.size())});
    m_list->setAnchorPoint(ccp(0.5f, 0.0f));
    this->addChildAtPosition(m_list, Anchor::Bottom, ccp(0, 0));
}