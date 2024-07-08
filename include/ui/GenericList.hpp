#pragma once
#include <Geode/Geode.hpp>
#include <deque>

namespace tulip::editor {
    class GenericList : public cocos2d::CCNode {
    public:
        geode::ScrollLayer* m_scrollLayer;
        std::deque<geode::Ref<cocos2d::CCNode>> m_items;
        float m_downLimit = 0.f;
        float m_upLimit = 0.f;
        int m_downIndex = -1;
        int m_upIndex = 0;

        using GeneratorType = geode::utils::MiniFunction<cocos2d::CCNode*(int)>;
        GeneratorType m_generator;

        static GenericList* create(const cocos2d::CCSize& size, GeneratorType generator);
        bool init(const cocos2d::CCSize& size, GeneratorType generator);

        void update(float dt) override;

        void removeTop();
        void removeBottom();
        void clear();

        void setContentSize(const cocos2d::CCSize& size) override;
        void setLayerContentSize(const cocos2d::CCSize& size);

        void readjustLimits();
    };
}