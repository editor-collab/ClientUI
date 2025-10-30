#include <hooks/ui/LevelEditorLayer.hpp>
#include <managers/LevelManager.hpp>
#include <ui/EditorOverlay.hpp>
#include <managers/BrowserManager.hpp>


using namespace geode::prelude;
using namespace tulip::editor;

bool LevelEditorLayerUIHook::init(GJGameLevel* level, bool p1) {
    if (!LevelEditorLayer::init(level, p1)) {
        return false;
    }

    LevelEntry* entry = BrowserManager::get()->getLevelEntry(LevelEditorLayer::get()->m_level);
	if (entry == nullptr) return true;

    auto old = m_objectLayer->getChildByType<EditorOverlay>(0);
    if (old == nullptr) {
        auto overlay = EditorOverlay::create(entry->hostAccountId);
        overlay->setID("selection-overlay"_spr);
        m_fields->editorOverlay = overlay;
		m_objectLayer->addChild(overlay, 10000);
    }

    return true;
}