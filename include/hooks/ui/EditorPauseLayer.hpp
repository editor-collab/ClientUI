#pragma once

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include "../../managers/FetchManager.hpp"

#include <Geode/modify/EditorPauseLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    struct EditorPauseLayerUIHook : Modify<EditorPauseLayerUIHook, EditorPauseLayer> {
        struct Fields {
            bool playLock = false;
        };

        $override
        bool init(LevelEditorLayer* editorLayer);

        $override
        void saveLevel();
        
        void setupGuidelinesMenu();
        void setupInfoMenu();
        void setupResumeMenu();

        void onPlay(cocos2d::CCObject*);
        void onPlayInLDM(cocos2d::CCObject*);
        void onExitWithoutPrompt(cocos2d::CCObject*);
        void onSaveToLocal(cocos2d::CCObject*);
    };
}