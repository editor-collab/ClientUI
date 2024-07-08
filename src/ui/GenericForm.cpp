#include <ui/GenericForm.hpp>

using namespace geode::prelude;
using namespace tulip::editor;

GenericForm::GenericForm() :
    m_columnMenu(),
    m_errorLabel(),
    m_confirmButton() {
	m_noElasticity = true;
}

GenericForm::~GenericForm() {}

bool GenericForm::init(std::string const& title) {
	return Popup::initAnchored(300.f, 100.f, title, "GJ_square05.png");
}

void GenericForm::updateErrorLabel(std::string const& error) {
	m_errorLabel->setString(error.c_str());
}

void GenericForm::onEnter(CCObject* sender) {
	this->updateErrorLabel("");
    this->confirm();
}

bool GenericForm::setup(std::string const& title) {
    m_columnMenu = CCMenu::create();
    m_columnMenu->setContentSize(CCSizeMake(300.f, 300.f));
    m_columnMenu->setAnchorPoint(ccp(0.5f, 1.f));
    m_columnMenu->setLayout(ColumnLayout::create()
        ->setAxisAlignment(AxisAlignment::Start)
        ->setCrossAxisLineAlignment(AxisAlignment::Center)
		->setAxisReverse(true)
        ->setGrowCrossAxis(false)
        ->setAutoScale(false)
        ->setGap(10.f)
        ->setAutoGrowAxis(100.f)
    );
    m_mainLayer->addChildAtPosition(m_columnMenu, Anchor::Top, ccp(0.f, 0.f));

    auto titleLabel = CCLabelBMFont::create(title.c_str(), "goldFont.fnt");
    titleLabel->setScale(.9f);
    titleLabel->setAnchorPoint(ccp(0.5f, 1.f));
    m_columnMenu->addChild(titleLabel);

	m_errorLabel = CCLabelBMFont::create("", "bigFont.fnt");
	m_errorLabel->setColor(ccc3(255, 127, 127));
    m_errorLabel->limitLabelWidth(260.f, .4f, .1f);
	m_columnMenu->addChild(m_errorLabel);

	this->formSetup();

	auto confirmSprite =
	    ButtonSprite::create("Confirm", "bigFont.fnt", "GJ_button_04.png", .8f);
	confirmSprite->setScale(.7f);

	m_confirmButton = CCMenuItemSpriteExtra::create(
	    confirmSprite, this, menu_selector(GenericForm::onEnter)
	);
	m_columnMenu->addChild(m_confirmButton);

    m_columnMenu->updateLayout();
    auto rect = calculateChildCoverage(m_columnMenu);

    log::debug("height {}", rect.size.height);

    m_mainLayer->setContentHeight(rect.size.height + 20.f);

    m_mainLayer->updateLayout();

	return true;
}

void GenericForm::close() {
	this->onClose(nullptr);
}

geode::TextInput* GenericForm::addTextNode(std::string const& id, std::string const& placeholder) {
    auto node = geode::TextInput::create(180.f, placeholder);
    node->setCommonFilter(CommonFilter::Any);
    node->setID(id);
    m_columnMenu->addChild(node);
    return node;
}

std::string GenericForm::getText(std::string const& id) {
    return static_cast<TextInput*>(m_columnMenu->getChildByID(id))->getString();
}
