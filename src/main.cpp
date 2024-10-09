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
		"Snap",
		"the key that'll be used to snap the pivot\n(only works with \"keybind\" snap mode)",
		{ Keybind::create(KEY_X, Modifier::None) },
		"Pivot Snap/Keybinds"
	});
}

class $modify(TheTransformCtrls, GJTransformControl) {

	struct Fields {
		std::string method;
	};

	void performSnapTest() {

		CCSprite* pivotNode = GJTransformControl::spriteByTag(1);
		CCNode* parentLayer = m_mainNodeParent;

		if (!parentLayer->isVisible()) {
			return;
		}


		CCArray* snapTargets = m_warpSprites;
		CCRect boundingBox = pivotNode->boundingBox();
		CCObject* obj;

		CCARRAY_FOREACH(snapTargets, obj) {
			CCSprite* warpSprite = dynamic_cast<CCSprite*>(obj);

			if (warpSprite && warpSprite != pivotNode && warpSprite->getTag() <= 9 && boundingBox.intersectsRect(warpSprite->boundingBox())) {
				
				CCPoint res = warpSprite->getParent()->convertToWorldSpace(warpSprite->getPosition());
				
				pivotNode->setPosition(pivotNode->getParent()->convertToNodeSpace(res));
				log::debug("Mandatory string, {}", res);
				
			}
		}

		log::debug("Mandatory string 2, {}", pivotNode->convertToWorldSpace({ 0, 0 }));
		GJTransformControl::refreshControl();
	};

	virtual bool init() {

		GJTransformControl::init();
		m_fields->method = Mod::get()->getSettingValue<std::string>("snap-mode");

		this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {

			if (event->isDown() && m_fields->method == "keybind") {
				performSnapTest();
			}

			return ListenerResult::Propagate;
			}, "pivot_snap"_spr);
	}

	virtual void ccTouchEnded(CCTouch* p0, CCEvent* p1) {
		GJTransformControl::ccTouchEnded(p0, p1);

		if (m_fields->method == "snapOnRelease") {
			performSnapTest();
		}

	};

	virtual void ccTouchMoved(CCTouch* p0, CCEvent* p1) {

		if (Mod::get()->getSettingValue<bool>("limit-size")) {

			CCArray* warpSprites = m_warpSprites;

			CCObject* obj;
			const float maxWarp = 15000;


			CCARRAY_FOREACH(warpSprites, obj) {
				CCSprite* warpSprite = dynamic_cast<CCSprite*>(obj);

				if (warpSprite) {
					CCPoint pos = warpSprite->getPosition();

					if (pos.y > maxWarp || pos.x > maxWarp) {
						return;
					}

				};
			}

		}

		GJTransformControl::ccTouchMoved(p0, p1);
		if (m_fields->method == "realtime") {
			performSnapTest();
		}


	};

};

// todo :  Fix vector too long error