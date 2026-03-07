#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

#include "TheEditorUI.hpp"
#include "TheTransformControls.hpp"

using namespace geode::prelude;

#define m_fields this->m_fields


struct Fields {
	CCMenuItemSpriteExtra* snapBtn = nullptr;
	TheTransformControls* pivotsnap = nullptr;

	// the reason this value exists is BECAUSE editorui::init calls deactivateTransformControls before pivotsnap is even initialized in the fields,
	// as such, enabler() prints a warning into the log which i only want happening if something truly goes wrong...
	bool firstInitialized = false;

	bool transformActive = false;
};

void TheEditorUI::enabler() {
	if (m_fields->pivotsnap != nullptr) {
		m_fields->pivotsnap->enableAll();
		if (auto method = Mod::get()->getSettingValue<std::string>("snap-method") == "button") {
			m_fields->snapBtn->setVisible(m_fields->pivotsnap->isEnabled());
			m_fields->transformActive = m_fields->pivotsnap->isEnabled();
		}
	}
	else {
		log::warn("enabler(): HELP!! Something went wrong when getting the transformcontrols class! Report this!!");
	}
}


bool TheEditorUI::init(LevelEditorLayer * lel) {
	if (!EditorUI::init(lel)) {
		return false;
	}

	m_fields->pivotsnap = static_cast<TheTransformControls*>(m_transformControl);
	NodeIDs::provideFor(this);

	// setup button
	auto method = Mod::get()->getSettingValue<std::string>("snap-method");
	m_fields->firstInitialized = true;
	if (method == "keybind") { return true; }


	CCSize size = m_unlinkBtn->getContentSize();
	float X = m_unlinkBtn->getPositionX() + (size.width / 2) + 10.f;

	//playback-menu
	if (auto menu = this->querySelector("editor-buttons-menu")) {
		auto sprite = CCSprite::create("GJ_snapBtn_001.png"_spr);

		auto btn = CCMenuItemSpriteExtra::create(
			sprite,
			this,
			menu_selector(TheEditorUI::onSnapBtn)
		);
		btn->setPosition(ccp(79, 30));
		btn->setContentSize(CCSize(20, 20));
		btn->setID("snap"_spr);
		btn->setVisible(false);

		sprite->setPosition(btn->getContentSize() / 2);

		menu->addChild(btn);
		menu->updateLayout();


		//log::debug("ok");

		m_fields->snapBtn = btn;
	}


	return true;
}

void TheEditorUI::onSnapBtn(CCObject * sender) {
	log::debug("{}, {}", m_fields->pivotsnap, m_fields->firstInitialized);
	if (m_fields->pivotsnap != nullptr && m_fields->firstInitialized) {
		m_fields->pivotsnap->performSnap(false);
	}
	else {
		log::warn("HELP!! Something went wrong when getting the transformcontrols class! Report this!!");
	}
}

/* out of line LINED OUT BRUH 👅👅👅👅
void deselectObject() {
	m_fields->pivotsnap->enableAll();
	EditorUI::deselectObject();
}
*/

/*

These functions were all used before i figured out that deactivateTransformControl() was a thing bruh.


void deselectObject(GameObject* obj) {
	EditorUI::deselectObject(obj);
	enabler();
}

void deselectAll() {
	EditorUI::deselectAll();
	enabler();
}

void onPlaytest(CCObject* p0) {
	EditorUI::onPlaytest(p0);
	enabler();
}


virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
	if (m_fields->pivotsnap != nullptr) {
		m_fields->snapBtn->setVisible(m_fields->pivotsnap->isEnabled());
	}
	else {
		log::warn("HELP!! Something went wrong when getting the transformcontrols class! Report this!!");
	}

	return EditorUI::ccTouchBegan(touch, event);
}
*/

void TheEditorUI::activateTransformControl(CCObject * sender) {
	//log::debug("activate");
	EditorUI::activateTransformControl(sender);

	if (m_fields->pivotsnap != nullptr) {
		#undef m_fields
		m_fields->pivotsnap->m_fields->initialized = true;
		#define m_fields this->m_fields
		// if any index mod reads this, please lmk how to avoid this #define stuff because this feels very unclean ...
	}

	if (m_fields->firstInitialized) {
		TheEditorUI::enabler();
	}
}

void TheEditorUI::deactivateTransformControl() {
	//log::debug("deactivate");
	EditorUI::deactivateTransformControl();
	if (m_fields->firstInitialized) {
		TheEditorUI::enabler();
	}

}