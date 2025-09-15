
#include <Geode/Geode.hpp>

#include <Geode/modify/PauseLayer.hpp>
using namespace geode::prelude;

namespace tulip::editor {
    struct PauseLayerUIHook : Modify<PauseLayerUIHook, PauseLayer> {
        $override
        void customSetup() override;
    };
}