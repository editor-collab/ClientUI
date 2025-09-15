#pragma once
#include <Geode/Geode.hpp>

namespace tulip::editor {
	class GenericForm : public geode::Popup<std::string const&> {
	public:
        cocos2d::CCMenu* m_columnMenu;
		cocos2d::CCLabelBMFont* m_errorLabel;
        CCMenuItemSpriteExtra* m_confirmButton;

		GenericForm();
		~GenericForm() override;

		bool init(std::string const& title);
		bool setup(std::string const& title) override;
        virtual void formSetup() = 0;

		void close();

		void updateErrorLabel(std::string const& error);
        geode::TextInput* addTextNode(std::string const& id, std::string const& placeholder);

        std::string getText(std::string const& id);

        void onEnter(cocos2d::CCObject* sender);
		virtual void confirm() = 0;
	};
}
