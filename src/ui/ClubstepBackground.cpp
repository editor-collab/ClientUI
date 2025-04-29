#include <ui/ClubstepBackground.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

ClubstepBackground* ClubstepBackground::create() {
    auto ret = new (std::nothrow) ClubstepBackground();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool ClubstepBackground::init() {
    if (!CCNode::init()) return false;

    static constexpr auto SIZE = CCSize{480, 360};
    this->setContentSize(SIZE);

    static constexpr std::array<CCPoint, 4> rectangle = {
        CCPoint{0, 0},
        CCPoint{SIZE.width, 0},
        CCPoint{SIZE.width, SIZE.height},
        CCPoint{0, SIZE.height},
    };
    
    auto mask = CCDrawNode::create();
    mask->setContentSize(SIZE);
    mask->drawPolygon(const_cast<CCPoint*>(rectangle.data()), 4, ccc4FFromccc3B({0, 0, 0}), 0, ccc4FFromccc3B({0, 0, 0}));

    auto clip = CCClippingNode::create();
    clip->setStencil(mask);
    clip->setAnchorPoint(ccp(0.5, 0.5));
    clip->setContentSize(SIZE);

    this->addChildAtPosition(clip, Anchor::Center, ccp(0, 0));

    m_bgSprite = CCSprite::create("game_bg_01_001.png");
    m_bgSprite->setPosition(ccp(SIZE.width / 2, SIZE.height / 2));
    m_bgSprite->setScale(1.2f);
    clip->addChild(m_bgSprite);

    constexpr float tintDuration = 1.5f;

    auto bgSequence = CCRepeatForever::create(CCSequence::create(
        CCTintTo::create(tintDuration, 40, 190, 255),
        CCTintTo::create(tintDuration, 40, 255, 133),
        CCTintTo::create(tintDuration, 120, 255, 40),
        CCTintTo::create(tintDuration, 255, 208, 40),
        CCTintTo::create(tintDuration, 255, 47, 40),
        CCTintTo::create(tintDuration, 255, 40, 241),
        CCTintTo::create(tintDuration, 65, 40, 255),
        CCTintTo::create(tintDuration, 40, 126, 255),
        nullptr
    ));

    m_bgSprite->runAction(bgSequence);

    return true;
}

void ClubstepBackground::update(float dt) {
    
}