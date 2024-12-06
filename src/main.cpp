/**
 * Include the Geode headers.
 */
#include <Geode/Geode.hpp>
#ifdef GEODE_IS_WINDOWS
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif

 /**
  * Brings cocos2d and all Geode namespaces to the current scope.
  */
using namespace geode::prelude;
#ifdef GEODE_IS_WINDOWS
using namespace keybinds;
#endif

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
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

#ifdef GEODE_IS_WINDOWS
$execute {
	BindManager::get()->registerBindable({
		"pivot_snap"_spr,
		"Snap",
		"the key that'll be used to snap the pivot\n(only works with \"keybind\" snap mode)",
		{ Keybind::create(KEY_X, Modifier::None) },
		"Pivot Snap/Keybinds"
	});
}
#endif

class $modify(TheTransformCtrls, GJTransformControl) {

	struct Fields {

		CCSprite* snappedTo = nullptr;
		CCArray* disabledWarps;
		CCArray* warpSprites;

		Fields()
			: disabledWarps(CCArray::create()), warpSprites(CCArray::create()) {
			disabledWarps->retain();
			warpSprites->retain();
		}

		~Fields() {
			disabledWarps->release();
			warpSprites->release();
		}

	};

	void updateValidSprites() {
		if (m_fields->warpSprites->count() > 0) {
			m_fields->warpSprites->removeAllObjects();
		}

		// Getting the frame name "warpBtn_02_001.png" cuz that's the buttons we need. Thanks DevTools
		const std::string textureNomenclature = "warpBtn_02_001.png";

		auto* cachedFrames = CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
		std::unordered_map<std::string, CCSpriteFrame*> frameMap;

		for (const auto& pair : CCDictionaryExt<std::string, CCSpriteFrame*>(cachedFrames)) {
			const std::string& key = pair.first;
			CCSpriteFrame* frame = pair.second;

			if (key == textureNomenclature) {
				frameMap[key] = frame;
			}
		}


		CCObject* obj;
		CCARRAY_FOREACH(m_mainNodeParent->getChildren(), obj) {

			if (auto spriteNode = typeinfo_cast<CCSprite*>(obj)) {
				auto rect = spriteNode->getTextureRect();

				auto it = frameMap.find(textureNomenclature);
				if (it != frameMap.end() && it->second->getRect() == rect) {
					m_fields->warpSprites->addObject(obj);
				}
			}

		}

		// 2024-12-05 4:40:30PM "yay"

	}

	void enableWarpers() {

		//Disable the disabled. I hope that makes sense :D
		CCObject* warpers;
		if (m_fields->disabledWarps->count() > 0) {
			CCARRAY_FOREACH(m_fields->warpSprites, warpers) {
				CCSprite* warperSprite = typeinfo_cast<CCSprite*>(warpers);
				warperSprite->setColor({ 255, 255, 255 });
			}

			m_fields->disabledWarps->removeAllObjects();
		}

	}

	void updateDisabledWarps() { 

		enableWarpers();

		if (m_fields->snappedTo == nullptr) {
			return;
		}

		ccColor3B disabledclr = { 140, 90, 90 };

		CCSprite* warpSprite = m_fields->snappedTo;

		float yPos = warpSprite->getPositionY();
		float xPos = warpSprite->getPositionX();

		int xCount = 0;
		int yCount = 0;

		CCArray* axisAlignedSprites = CCArray::create(); // Contains all sprites that align on either the x or y axis
		axisAlignedSprites->retain();

		CCObject* warpSpriteObj;
		CCARRAY_FOREACH(m_fields->warpSprites, warpSpriteObj) {
			CCSprite* CCwarpSprite = typeinfo_cast<CCSprite*>(warpSpriteObj);

			if (CCwarpSprite && !m_fields->disabledWarps->containsObject(CCwarpSprite) && CCwarpSprite != warpSprite) {

				if (CCwarpSprite->getPositionX() == xPos) {

					axisAlignedSprites->addObject(CCwarpSprite);
					xCount++;
				}

				if (CCwarpSprite->getPositionY() == yPos) {

					axisAlignedSprites->addObject(CCwarpSprite);
					yCount++;

				}

			};
		}

		if (axisAlignedSprites->count() != 4) { //If it's not a corner

			if (yCount > xCount) { //if there were more horizontal positions found than vertical ones, it must be a horizontal row
				

				m_fields->disabledWarps->removeAllObjects();
				CCObject* rowObject;
				CCARRAY_FOREACH(m_fields->warpSprites, rowObject) {
					CCSprite* rowObj = typeinfo_cast<CCSprite*>(rowObject);

					if (rowObj->getPositionY() == yPos && rowObj != warpSprite) {
						m_fields->disabledWarps->addObject(rowObj);
					}
				}

			}
			else if (xCount > yCount) { // if there were more vertical positions found, must be a vertical row then, right?
				

				m_fields->disabledWarps->removeAllObjects();
				CCObject* columnObject;
				CCARRAY_FOREACH(m_fields->warpSprites, columnObject) {
					CCSprite* columnObj = typeinfo_cast<CCSprite*>(columnObject);

					if (columnObj->getPositionX() == xPos && columnObj != warpSprite) {
						m_fields->disabledWarps->addObject(columnObj);
					}
				}

			}

		}
		else {

			CCObject* axisObj;
			CCARRAY_FOREACH(axisAlignedSprites, axisObj) {
				CCSprite* axisObject = typeinfo_cast<CCSprite*>(axisObj);
				m_fields->disabledWarps->addObject(axisObject);
			}

		}

		// applying colors because visual stuff gives me dopamine
		CCObject* warp;
		CCARRAY_FOREACH(m_fields->disabledWarps, warp) {
			CCSprite* warpObject = typeinfo_cast<CCSprite*>(warp);
			warpObject->setColor(disabledclr);
		}
		// It's as shrimple as that!!!


		axisAlignedSprites->release(); // memory not leaking anymor
	};

	std::pair<bool, CCSprite*> snap(bool test) {

		if (test) { // Testing mode: Returns if the pivot WOULD snap or not. Im repeating lots of code but i dont wanna make a separate function for this
			updateValidSprites();

			if (!m_mainNodeParent->isVisible()) {
				return std::make_pair(false, nullptr);
			}

			CCSprite* pivotNode = GJTransformControl::spriteByTag(1);
			CCArray* targets = m_fields->warpSprites;
			CCRect pivotBox = pivotNode->boundingBox();

			int foundObjs = 0;

			CCObject* obj;
			CCSprite* result;
			CCARRAY_FOREACH(targets, obj) {
				CCSprite* warpSprite = typeinfo_cast<CCSprite*>(obj);

				if (warpSprite && warpSprite != pivotNode && pivotBox.intersectsRect(warpSprite->boundingBox())) {
					result = warpSprite;
					foundObjs++;

					break;
				}
			}


			if (foundObjs == 0) {
				log::debug("Test! No snap.");
				return std::make_pair(false, nullptr);
			}
			else {
				log::debug("Test! Would snap.");
				return std::make_pair(true, result);
			}

		}
		else {
			updateValidSprites();

			if (!m_mainNodeParent->isVisible()) {
				return std::make_pair(false, nullptr);
			}

			CCSprite* pivotNode = GJTransformControl::spriteByTag(1);
			CCArray* targets = m_fields->warpSprites;
			CCRect pivotBox = pivotNode->boundingBox();

			int foundObjs = 0;

			CCObject* obj;
			CCARRAY_FOREACH(targets, obj) {
				CCSprite* warpSprite = typeinfo_cast<CCSprite*>(obj);

				if (warpSprite && warpSprite != pivotNode && pivotBox.intersectsRect(warpSprite->boundingBox())) {
					CCPoint result = warpSprite->getParent()->convertToWorldSpace(warpSprite->getPosition());

					pivotNode->setPosition(pivotNode->getParent()->convertToNodeSpace(result));
					m_fields->snappedTo = warpSprite;

					foundObjs++;

					break;
				}
			}

			if (foundObjs == 0) {
				m_fields->snappedTo = nullptr;
				enableWarpers();
				log::debug("No objects to snap to found");
			}


			GJTransformControl::refreshControl();
			updateDisabledWarps();
			return std::make_pair(false, nullptr);
		}
		
	};


	//Now im hooking le functionas.
	virtual bool init() {
		enableWarpers();

		GJTransformControl::init();
		auto method = Mod::get()->getSettingValue<std::string>("snap-mode");

		#ifdef GEODE_IS_WINDOWS
		this->template addEventListener<InvokeBindFilter>([=, this](InvokeBindEvent* event) {
			if (event->isDown() && method == "keybind") {
				snap(false);
				log::debug("Attempted to snap.");
			}

			return ListenerResult::Propagate;
			}, "pivot_snap"_spr);
		#endif

		return true;
	}


	virtual void ccTouchEnded(CCTouch* p0, CCEvent* p1) {
		auto snapRes = snap(true);

		if (!snapRes.first || snapRes.first && m_fields->warpSprites->containsObject(snapRes.second) && snapRes.second != m_fields->snappedTo) {
			enableWarpers();
		}

		GJTransformControl::ccTouchEnded(p0, p1);
	}

	virtual bool ccTouchBegan(CCTouch* p0, CCEvent* p1) {
		CCPoint location = p0->getStartLocation();
		

		if (m_fields->disabledWarps->count() > 0) {
			

			CCObject* disabledWarper;
			CCARRAY_FOREACH(m_fields->disabledWarps, disabledWarper) {
				
				CCSprite* warper = typeinfo_cast<CCSprite*>(disabledWarper);
				if (!warper) continue;

				float deadzoneWidth = warper->getContentWidth() * 1.1;
				float deadzoneHeight = warper->getContentHeight() * 1.1;
				
				CCPoint worldPos = warper->getParent()->convertToWorldSpace(warper->getPosition());
				CCRect bounderBox = CCRect(
					worldPos.x - deadzoneWidth / 2,
					worldPos.y - deadzoneHeight / 2,
					deadzoneWidth,
					deadzoneHeight
				);

				log::debug("Width: {}, Height: {}", warper->getContentWidth(), warper->getContentHeight());

				if (bounderBox.containsPoint(location)) {
					log::debug("Touched forbidden point");
					return true;
				}

			}
		}


		return GJTransformControl::ccTouchBegan(p0, p1);
	}

};

class $modify(TheEditorUI, EditorUI) {

	void onToggle() {
		static_cast<TheTransformCtrls*>(m_transformControl)->enableWarpers();
	}
	
	void activateTransformControl(CCObject* p0) {
		log::debug("Print");
		onToggle();
		EditorUI::activateTransformControl(p0);
	}

};

#endif
