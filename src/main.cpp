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

	static void onModify(auto & self) {
		(void) self.setHookPriority("GJTransformControl::init", -3000000);
		(void) self.setHookPriority("GJTransformControl::ccTouchEnded", -3000000);
		(void) self.setHookPriority("GJTransformControl::ccTouchMoved", -3000000);
	}

	struct Fields {

		std::string method;
		CCSprite* snappedTo = nullptr;
		CCArray* disabledWarps;
		CCArray* validWarpSprites;

		Fields()
			: disabledWarps(CCArray::create()), validWarpSprites(CCArray::create()) {
			disabledWarps->retain();
			validWarpSprites->retain();
		}

		~Fields() {
			disabledWarps->release();
			validWarpSprites->release();
		}

	};

	void updateValidSprites() {
		if (m_fields->validWarpSprites->count() > 0) {
			m_fields->validWarpSprites->removeAllObjects();
		}
		
		CCObject* mWarpSprite;
		CCARRAY_FOREACH(m_warpSprites, mWarpSprite) {
			CCSprite* castedWarpSprite = dynamic_cast<CCSprite*>(mWarpSprite);

			if (castedWarpSprite->getContentHeight() == castedWarpSprite->getContentWidth()) {
				m_fields->validWarpSprites->addObject(castedWarpSprite);
			}

		}

	}

	void printDisabledWarps() {
		CCObject* obj;

		CCARRAY_FOREACH(m_fields->disabledWarps, obj) {
			CCSprite* castedObj = dynamic_cast<CCSprite*>(obj);
			log::debug("{} is disabled", castedObj->getTag());
		}

	}

	void enableAll() {

		CCObject* warp;
		if (m_fields->disabledWarps->count() > 0) {

			CCARRAY_FOREACH(m_fields->validWarpSprites, warp) {
				CCSprite* objWarpSprite = dynamic_cast<CCSprite*>(warp);
				objWarpSprite->setColor({ 255, 255, 255 });
			}


			m_fields->disabledWarps->removeAllObjects();
		}

	}

	void updateDisabledWarps(CCSprite* warpSprite) {
		updateValidSprites();

		enableAll();
		
		CCObject* obj2;
		CCObject* obj;

		ccColor3B disabledclr = { 140, 90, 90 };

		float yPos = warpSprite->getPositionY();
		float xPos = warpSprite->getPositionX();

		int xCount = 0;
		int yCount = 0;


		CCARRAY_FOREACH(m_fields->validWarpSprites, obj) {
			CCSprite* objWarpSprite = dynamic_cast<CCSprite*>(obj);


			if (objWarpSprite && !m_fields->disabledWarps->containsObject(objWarpSprite) && objWarpSprite != warpSprite) {

				if (objWarpSprite->getPositionX() == xPos) {

					m_fields->disabledWarps->addObject(objWarpSprite);
					xCount++;
				}

				if (objWarpSprite->getPositionY() == yPos) {

					m_fields->disabledWarps->addObject(objWarpSprite);
					yCount++;

				}

			};

			// log::debug("yCount is {}", yCount);
		}

		

		if (m_fields->disabledWarps->count() != 4) { //If it's not a corner

			if (yCount > xCount) { //if there were more horizontal positions found than vertical ones, it must be a horizontal row

				m_fields->disabledWarps->removeAllObjects();
				CCObject* rowObject;
				CCARRAY_FOREACH(m_fields->validWarpSprites, rowObject) {
					CCSprite* rowObj = dynamic_cast<CCSprite*>(rowObject);

					if (rowObj->getPositionY() == yPos && rowObj != warpSprite) {
						m_fields->disabledWarps->addObject(rowObj);
					}
				}

			}
			else if (xCount > yCount) { // if there were more vertical positions found, must be a horizontal row then, right?

				m_fields->disabledWarps->removeAllObjects();
				CCObject* columnObject;
				CCARRAY_FOREACH(m_fields->validWarpSprites, columnObject) {
					CCSprite* columnObj = dynamic_cast<CCSprite*>(columnObject);

					if (columnObj->getPositionX() == xPos && columnObj != warpSprite) {
						m_fields->disabledWarps->addObject(columnObj);
					}
				}

			}

		}

		printDisabledWarps();

		//apply colorrs!!
		CCObject* warp;
		CCARRAY_FOREACH(m_fields->disabledWarps, warp) {
			CCSprite* warpObj = dynamic_cast<CCSprite*>(warp);
			warpObj->setColor(disabledclr);
		}

	};

	void performSnapTest(bool isTouchMoved) {

		CCSprite* pivotNode = GJTransformControl::spriteByTag(1);
		CCNode* parentLayer = m_mainNodeParent;

		if (!parentLayer->isVisible()) {
			return;
		}


		CCArray* snapTargets = m_fields->validWarpSprites;
		CCRect boundingBox = pivotNode->boundingBox();
		CCObject* obj;
		int foundObjs = 0;

		CCARRAY_FOREACH(snapTargets, obj) {
			CCSprite* warpSprite = dynamic_cast<CCSprite*>(obj);

			if (warpSprite && warpSprite != pivotNode && boundingBox.intersectsRect(warpSprite->boundingBox())) {
				
				CCPoint res = warpSprite->getParent()->convertToWorldSpace(warpSprite->getPosition());
				
				pivotNode->setPosition(pivotNode->getParent()->convertToNodeSpace(res));
				m_fields->snappedTo = warpSprite;

				if (isTouchMoved == false) {
					updateDisabledWarps(warpSprite);
				}
				foundObjs++;

				break;
			}
		}

		if (foundObjs == 0) {
			m_fields->snappedTo = nullptr;
			enableAll();
		}

		GJTransformControl::refreshControl();
	};

	virtual bool init() {

		GJTransformControl::init();
		m_fields->method = Mod::get()->getSettingValue<std::string>("snap-mode");


		this->template addEventListener<InvokeBindFilter>([=](InvokeBindEvent* event) {

			if (event->isDown() && m_fields->method == "keybind") {
				performSnapTest(false);
			}

			return ListenerResult::Propagate;
			}, "pivot_snap"_spr);

	}

	virtual bool ccTouchBegan(CCTouch* p0, CCEvent* p1) {
		
		CCPoint touch = p0->getLocation();
		log::debug("mandatory string, {}", touch);
		CCObject* obj;
		CCObject* warp;

		updateValidSprites();

		CCARRAY_FOREACH(m_fields->validWarpSprites, obj) {
			CCSprite* warpSprite = dynamic_cast<CCSprite*>(obj);

			if (warpSprite) {

				CCPoint pos = warpSprite->getParent()->convertToWorldSpace(warpSprite->getPosition());
				CCSize size = warpSprite->getScaledContentSize();

				CCRect boundingBox = CCRect(
					pos.x - (size.width * 1.5) / 2,
					pos.y - (size.height * 1.5) / 2,
					size.width * 1.5,
					size.height * 1.5
				);

				if (boundingBox.containsPoint(touch)) {
					log::debug("disabledObjects {}", m_fields->disabledWarps);
					
					CCARRAY_FOREACH(m_fields->disabledWarps, warp) {
						log::debug("disabledObject, {}", warp->getTag());
						if (warp->getTag() == warpSprite->getTag()) {
							log::debug("denied point {}", warpSprite->getTag());
							return false;
						}
					}

				}

			}

		}


		GJTransformControl::ccTouchBegan(p0, p1);
	}

	virtual void ccTouchEnded(CCTouch* p0, CCEvent* p1) {
		GJTransformControl::ccTouchEnded(p0, p1);

		if (m_fields->method == "snapOnRelease") {
			performSnapTest(false);
		}
		else
		{
			CCSprite* pivotNode = GJTransformControl::spriteByTag(1);
			CCNode* parentLayer = m_mainNodeParent;

			if (!parentLayer->isVisible()) {
				return;
			}


			CCArray* snapTargets = m_fields->validWarpSprites;
			CCRect boundingBox = pivotNode->boundingBox();

			CCObject* obje;

			CCARRAY_FOREACH(snapTargets, obje) {
				CCSprite* obj = dynamic_cast<CCSprite*>(obje);

				if (!boundingBox.intersectsRect(obj->boundingBox())) {
					enableAll();
				}
			}


		}

	};
	
	/*
	virtual void ccTouchMoved(CCTouch* p0, CCEvent* p1) {

		if (m_fields->method == "realtime") {
			performSnapTest(true);
		}

		if (m_fields->lastSnapped != m_fields->snappedTo) {

			updateDisabledWarps(m_fields->snappedTo);
			m_fields->lastSnapped = m_fields->snappedTo;

		}

		GJTransformControl::ccTouchMoved(p0, p1);

	};
	
	*/

};

// todo :  Fix vector too long error