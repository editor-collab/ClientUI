#include <hooks/ui/PauseLayer.hpp>
#include <managers/LevelManager.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

void PauseLayerUIHook::customSetup() {
    PauseLayer::customSetup();

    if (!LevelManager::get()->hasJoinedLevelKey()) {
        return;
    }

    if (Mod::get()->getSavedValue<bool>("shown-playtest-pause-popup") == false) {
        auto popup = geode::createQuickPopup(
            "Editor Collab", 
            "<co>Exiting from playtest</c> is <cr>disabled</c>, please use the <cg>Editor Pause Menu</c> if you need to <cl>exit</c>. "
            "You can also <ca>go back to the editor</c> to continue editing.",
            "OK", nullptr, 350.f, [this](FLAlertLayer* layer, bool btn2) {}, false
        );
        popup->m_scene = this;
        popup->show();
        Mod::get()->setSavedValue("shown-playtest-pause-popup", true);
    }

    return;
}