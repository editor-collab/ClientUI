#include <ui/GenericList.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

GenericList* GenericList::create(const cocos2d::CCSize& size, GeneratorType generator) {
    auto ret = new GenericList();
    if (ret && ret->init(size, generator)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool GenericList::init(const cocos2d::CCSize& size, GeneratorType generator) {
    if (!CCNode::init()) {
        return false;
    }

    this->scheduleUpdate();

    m_generator = generator;

    this->setContentSize(size);
    m_scrollLayer = ScrollLayer::create(size);
    this->readjustLimits();

    this->addChild(m_scrollLayer);
    return true;
}

void GenericList::update(float dt) {
    CCNode::update(dt);
    if (!m_scrollLayer) return;
    auto scrollLayerHeight = m_scrollLayer->getContentSize().height;
    auto contentLayerHeight = m_scrollLayer->m_contentLayer->getContentSize().height;
    auto contentLayerY = m_scrollLayer->m_contentLayer->getPositionY();

    auto targetDownLimit = -contentLayerY;
    auto targetUpLimit = scrollLayerHeight - contentLayerY;

    // //////// log::debug("targetDownLimit: {}, targetUpLimit: {}", targetDownLimit, targetUpLimit);
    // //////// log::debug("downLimit: {}, upLimit: {}", m_downLimit, m_upLimit);

    // needs to add more items to the bottom
    while (m_downLimit > targetDownLimit && m_downLimit > 0) {
        auto newItem = m_generator(m_downIndex + 1);
        if (newItem == nullptr) break;
        m_downIndex += 1;
        m_downLimit -= newItem->getScaledContentSize().height;
        auto newY = m_downLimit;

        newItem->setPosition(ccp(0.f, newY));
        newItem->setAnchorPoint(ccp(0.f, 0.f));
        m_items.push_back(newItem);
        m_scrollLayer->m_contentLayer->addChild(newItem);
    }

    // needs to add more items to the top
    while (m_upLimit < targetUpLimit && m_upLimit < contentLayerHeight) {
        auto newItem = m_generator(m_upIndex - 1);
        if (newItem == nullptr) break;
        m_upIndex -= 1;
        auto newY = m_upLimit;
        m_upLimit += newItem->getScaledContentSize().height;

        newItem->setPosition(ccp(0.f, newY));
        newItem->setAnchorPoint(ccp(0.f, 0.f));
        m_items.push_front(newItem);
        m_scrollLayer->m_contentLayer->addChild(newItem);
    }

    // needs to remove items from the bottom
    while (m_items.size() > 0) {
        auto item = m_items.back();
        if (item->getPositionY() + item->getScaledContentSize().height < targetDownLimit) {
            this->removeBottom();
        } else {
            break;
        }
    }

    // needs to remove items from the top
    while (m_items.size() > 0) {
        auto item = m_items.front();
        if (item->getPositionY() > targetUpLimit) {
            this->removeTop();
        } else {
            break;
        }
    }
}

void GenericList::removeTop() {
    auto item = m_items.front();
    m_upLimit -= item->getScaledContentSize().height;
    m_upIndex += 1;
    m_items.pop_front();
    m_scrollLayer->m_contentLayer->removeChild(item);
}
void GenericList::removeBottom() {
    auto item = m_items.back();
    m_downLimit += item->getScaledContentSize().height;
    m_downIndex -= 1;
    m_items.pop_back();
    m_scrollLayer->m_contentLayer->removeChild(item);
}
void GenericList::clear() {
    while (m_items.size() > 0) {
        this->removeBottom();
    }
}

void GenericList::setContentSize(const cocos2d::CCSize& size) {
    CCNode::setContentSize(size);
    if (m_scrollLayer) {
        m_scrollLayer->setContentSize(size);
        this->readjustLimits();
    }
}

void GenericList::setLayerContentSize(const cocos2d::CCSize& size) {
    if (m_scrollLayer) {
        m_scrollLayer->m_contentLayer->setContentSize(size);
        m_scrollLayer->moveToTop();
        this->readjustLimits();
    }
}

void GenericList::readjustLimits() {
    if (m_scrollLayer) {
        auto scrollLayerHeight = m_scrollLayer->getContentSize().height;
        auto contentLayerHeight = m_scrollLayer->m_contentLayer->getContentSize().height;
        m_downLimit = contentLayerHeight;
        m_upLimit = contentLayerHeight;
        m_downIndex = -1;
        m_upIndex = 0;

        m_scrollLayer->m_contentLayer->removeAllChildren();
        m_items.clear();
    }
}