/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>
#include <geode.custom-keybinds/include/Keybinds.hpp>

 /**
  * Brings cocos2d and all Geode namespaces to the current scope.
  */
using namespace geode::prelude;
using namespace keybinds;

/**
 * `$modify` lets you extend and modify GD's classes.
 * To hook a function in Geode, simply $modify the class
 * and write a new function definition with the signature of
 * the function you want to hook.
 *
 * Here we use the overloaded `$modify` macro to set our own class name,
 * so that we can use it for button callbacks.
 *
 * Notice the header being included, you *must* include the header for
 * the class you are modifying, or you will get a compile error.
 *
 * Another way you could do this is like this:
 *
 * struct MyMenuLayer : Modify<MyMenuLayer, MenuLayer> {};
 */

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/GJTransformControl.hpp>

$execute {
	BindManager::get()->registerBindable({
		"pivot_snap"_spr,
		"Move",
		"",
		{ Keybind::create(KEY_X, Modifier::None) },
		"Pivot Snap/Keybinds"
	});
}

class $modify(TheTransformCtrls, GJTransformControl) {

	virtual bool init() {
		GJTransformControl::init();

		this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {
			if (event->isDown()) {

				CCSprite* pivotNode = GJTransformControl::spriteByTag(1);
				CCNode* layer = pivotNode->getParent();	

				if (!layer->isVisible()) {
					return ListenerResult::Propagate;
				}

				CCPoint newPos = layer->convertToNodeSpace(getMousePos());

				pivotNode->setPosition(newPos);
				GJTransformControl::refreshControl();

			}

			return ListenerResult::Propagate;
		}, "pivot_snap"_spr);

	}
	

};


/*
class $modify(MyMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init())
			return false;

		auto winSize = CCDirector::get()->getWinSize();

		auto label = cocos2d::CCLabelBMFont::create("Hello world", "bigFont.fnt");
		label->setPosition(winSize / 2);
		this->addChild(label);

		log::debug("hi");



		return true;
	}


};
*/

class $modify(TheEditor, LevelEditorLayer) {
	bool init(GJGameLevel * p0, bool p1) {
		if (!LevelEditorLayer::init(p0, p1)) {
			return false;
		}

		auto mainNode = this->getChildByID("main-node");
		auto batchLayer = mainNode->getChildByID("batch-layer");

		return true;
	}
};

class $modify(TheEditorUI, EditorUI) {

	static void onModify(auto & self) {
		(void)self.setHookPriority("EditorUI::transformObjectCall", INT_MIN);
	}

	void sliderChanged(cocos2d::CCObject * p0) {
		EditorUI::sliderChanged(p0);

		log::debug("Slider");
	}

	void activateTransformControl(cocos2d::CCObject * p0) {
		EditorUI::activateTransformControl(p0);

		log::debug("toggled transform Ctrl.");
	}

	void transformObjectCall(EditCommand command) {
		EditorUI::transformObjectCall(command);

		log::debug("transformObjectCall");
	}

};