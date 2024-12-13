#include <managers/CellManager.hpp>
#include <lavender/Lavender.hpp>

using namespace tulip::editor;
using namespace geode::prelude;

class CellManager::Impl {
public:
    void applyMyLevel(LevelCell* cell, LevelEntry const& entry);
    void applySharedLevel(LevelCell* cell, LevelEntry const& entry);
    void applyDiscoverLevel(LevelCell* cell, LevelEntry const& entry);

    void removeLabels(LevelCell* cell);
};

void CellManager::Impl::removeLabels(LevelCell* cell) {
    if (auto node = cell->getChildByIDRecursive("length-icon")) node->setVisible(false);
    if (auto node = cell->getChildByIDRecursive("length-label")) node->setVisible(false);
    if (auto node = cell->getChildByIDRecursive("song-icon")) node->setVisible(false);
    if (auto node = cell->getChildByIDRecursive("song-label")) node->setVisible(false);
    if (auto node = cell->getChildByIDRecursive("info-icon")) node->setVisible(false);
    if (auto node = cell->getChildByIDRecursive("info-label")) node->setVisible(false);
}

void CellManager::Impl::applyMyLevel(LevelCell* cell, LevelEntry const& entry) {
    this->removeLabels(cell);
    // if (auto node = typeinfo_cast<CCLabelBMFont*>(cell->getChildByIDRecursive("level-name"))) { // temp
    //     node->setString(("Shared: " + std::string(node->getString())).c_str());
    // }

    auto gen = new ui::Container {
        .size = cell->getContentSize(),
        .padding = ui::EdgeInsets::Symmetric{.vertical = 12, .horizontal = 10, },
        .child = new ui::Align {
            .alignment = ui::Alignment::BottomCenter,
            .child = new ui::Row {
                .children = {
                    new ui::TextArea {
                        .text = fmt::format("{} users joined", entry.userCount),
                        .font = "bigFont.fnt",
                        .color = {0, 255, 0, 255},
                        .scale = 0.4f,
                    },
                    new ui::Container {
                        .width = 10,
                    },
                    new ui::TextArea {
                        .text = entry.key,
                        .font = "bigFont.fnt",
                        .color = {0, 255, 200, 255},
                        .scale = 0.4f,
                    },
                    new ui::Container {
                        .width = 10,
                    },
                },
            },
        },
    };

    auto node = gen->get();
    node->setAnchorPoint(ccp(0, 0));
    node->setPosition(ccp(0, 0));
    node->setZOrder(10);
    cell->m_mainLayer->addChild(node);

    // auto row = CCMenu::create();
    // row->setLayout(RowLayout::create()
    //     ->setAutoScale(false)
    //     ->setAxisAlignment(AxisAlignment::Start)
    //     ->setGap(10)
    // );
    // row->setContentSize(ccp(cell->getContentWidth() - 20, 15));
    // row->setAnchorPoint(ccp(0.5, 0));
    // row->setPosition(ccp(cell->getContentWidth() / 2, 10));

    // auto countLabel = CCLabelBMFont::create(fmt::format("{} users", entry.userCount).c_str(), "bigFont.fnt");
    // countLabel->limitLabelWidth(50, 0.4f, 0.1f);
    // countLabel->setColor(ccc3(0, 255, 0));
    // row->addChild(countLabel);

    // auto keyLabel = CCLabelBMFont::create(entry.key.c_str(), "bigFont.fnt");
    // keyLabel->limitLabelWidth(50, 0.4f, 0.1f);
    // keyLabel->setColor(ccc3(0, 255, 200));
    // row->addChild(keyLabel);

    // row->updateLayout();

    // cell->m_mainLayer->addChild(row);


}

void CellManager::Impl::applySharedLevel(LevelCell* cell, LevelEntry const& entry) {
    this->removeLabels(cell);

}

void CellManager::Impl::applyDiscoverLevel(LevelCell* cell, LevelEntry const& entry) {
    this->removeLabels(cell);
}

CellManager* CellManager::get() {
    static CellManager instance;
    return &instance;
}

CellManager::CellManager() : impl(std::make_unique<Impl>()) {}
CellManager::~CellManager() = default;

void CellManager::applyMyLevel(LevelCell* cell, LevelEntry const& entry) {
    impl->applyMyLevel(cell, entry);
}

void CellManager::applySharedLevel(LevelCell* cell, LevelEntry const& entry) {
    impl->applySharedLevel(cell, entry);
}   

void CellManager::applyDiscoverLevel(LevelCell* cell, LevelEntry const& entry) {
    impl->applyDiscoverLevel(cell, entry);
}