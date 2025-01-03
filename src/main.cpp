/**
 * Include the Geode headers
 */
#include <Geode/Geode.hpp>
#include <Geode/modify/GJTransformControl.hpp>
#include <Geode/modify/EditorUI.hpp>

#ifdef GEODE_IS_WINDOWS
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif

using namespace geode::prelude;
#ifdef GEODE_IS_WINDOWS
using namespace keybinds;
#endif


#ifdef GEODE_IS_WINDOWS
$execute{
	BindManager::get()->registerBindable({
		"pivot_snap"_spr,
		"Snap",
		"the key that'll be used to snap the pivot\n(only works with \"keybind\" snap mode)",
		{ Keybind::create(KEY_X, Modifier::None) },
		"Pivot Snap/Keybinds"
	});
}
#endif

const float EPSILON = 0.001f;

class $modify(TheTransformCtrls, GJTransformControl) {

	struct Fields {

		bool initialized = false;
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
			m_fields->warpSprites->removeAllObjects(); // Clear existing objects safely
		}

		const std::string textureNomenclature = "warpBtn_02_001.png";

		auto* cachedFrames = CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
		std::unordered_map<std::string, CCSpriteFrame*> frameMap;

		// Populate the frame map with frames matching the nomenclature
		for (const auto& pair : CCDictionaryExt<std::string, CCSpriteFrame*>(cachedFrames)) {
			const std::string& key = pair.first;
			CCSpriteFrame* frame = pair.second;

			if (key == textureNomenclature) {
				frameMap[key] = frame;
			}
		}

		// Iterate over the main node's children and filter based on texture
		CCObject* obj;
		CCARRAY_FOREACH(m_mainNodeParent->getChildren(), obj) {
			if (auto spriteNode = dynamic_cast<CCSprite*>(obj)) {
				auto rect = spriteNode->getTextureRect();

				auto it = frameMap.find(textureNomenclature);
				if (it != frameMap.end() && it->second->getRect() == rect) {
					m_fields->warpSprites->addObject(spriteNode);
				}
			}
		}

		if (m_fields->warpSprites->count() <= 0) {
			log::warn("updateValidSprites(): No warp sprite textures found. This shouldn't happen, report if this does.");
		}

		// 2024-12-05 4:40:30PM "yay"
	}


	void enableWarpers() {
		if (m_fields->disabledWarps->count() > 0) {
			//Disable the disabled. I hope that makes sense :D
			CCObject* warper;
			CCARRAY_FOREACH(m_fields->disabledWarps, warper) {
				if (auto warperSprite = dynamic_cast<CCSprite*>(warper)) {
					warperSprite->setColor({ 255, 255, 255 });
				}
				else {
					log::warn("enableWarpers(): Unexpected object inside disabledWarps. This shouldn't happen, please report!!");
				}
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

		CCObject* warpSpriteObj;
		CCArray* axisAlignedSprites = CCArray::create(); // WILL contain all sprites that align on either the x or y axis
		axisAlignedSprites->retain();
		CCARRAY_FOREACH(m_fields->warpSprites, warpSpriteObj) {
			if (CCSprite* CCwarpSprite = dynamic_cast<CCSprite*>(warpSpriteObj)) {
				
				if (!m_fields->disabledWarps->containsObject(CCwarpSprite) && CCwarpSprite != warpSprite) {
					if (abs(CCwarpSprite->getPositionX() - xPos) < EPSILON) {

						axisAlignedSprites->addObject(CCwarpSprite);
						xCount++;

					}

					if (abs(CCwarpSprite->getPositionY() - yPos) < EPSILON) {

						axisAlignedSprites->addObject(CCwarpSprite);
						yCount++;

					}

				};
			}

		}

		if (axisAlignedSprites->count() != 4) { //If it's not a corner

			if (yCount > xCount) { //if there were more horizontal positions found than vertical ones, it must be a horizontal row

				m_fields->disabledWarps->removeAllObjects();
				CCObject* rowObject;
				CCARRAY_FOREACH(m_fields->warpSprites, rowObject) {

					if (CCSprite* rowObj = dynamic_cast<CCSprite*>(rowObject)) {
						if (abs(rowObj->getPositionY() - yPos) < EPSILON && rowObj != warpSprite) {
							m_fields->disabledWarps->addObject(rowObj);
						}
					}

				}

			}
			else if (xCount > yCount) { // if there were more vertical positions found, must be a vertical row then, right?

				m_fields->disabledWarps->removeAllObjects();
				CCObject* columnObject;
				CCARRAY_FOREACH(m_fields->warpSprites, columnObject) {

					if (CCSprite* columnObj = dynamic_cast<CCSprite*>(columnObject)) {
						if (abs(columnObj->getPositionX() - xPos) < EPSILON && columnObj != warpSprite) {
							m_fields->disabledWarps->addObject(columnObj);
						}
					}

				}

			}

		}
		else {
			CCObject* axisObj;
			CCARRAY_FOREACH(axisAlignedSprites, axisObj) {
				if (CCSprite* axisObject = dynamic_cast<CCSprite*>(axisObj)) {
					m_fields->disabledWarps->addObject(axisObject);
				}
			}

		}

		// applying colors because visual stuff gives me dopamine
		CCObject* warp;
		CCARRAY_FOREACH(m_fields->disabledWarps, warp) {

			if (CCSprite* warpObject = dynamic_cast<CCSprite*>(warp)) {
				warpObject->setColor(disabledclr);
			}
			
		}
		// It's as shrimple as that!!!

		axisAlignedSprites->release(); // memory not leaking anymor
	};

	std::pair<bool, CCSprite*> snap(bool test) {

		if (test) { // Testing mode: Returns if the pivot WOULD snap or not. Im repeating lots of code but i dont wanna make a separate function for this
			if (!m_fields->initialized || !m_mainNodeParent->isVisible()) {
				return std::make_pair(false, nullptr);
			}

			updateValidSprites();

			CCSprite* pivotNode = GJTransformControl::spriteByTag(1);
			CCArray* targets = m_fields->warpSprites;
			CCRect pivotBox = pivotNode->boundingBox();

			int foundObjs = 0;

			CCObject* obj;
			CCSprite* result;
			CCARRAY_FOREACH(targets, obj) {
				CCSprite* warpSprite = dynamic_cast<CCSprite*>(obj);

				if (warpSprite && warpSprite != pivotNode && pivotBox.intersectsRect(warpSprite->boundingBox())) {
					result = warpSprite;
					foundObjs++;

					break;
				}
			}


			if (foundObjs == 0) {
				// log::debug("Test! No snap.");
				return std::make_pair(false, nullptr);
			}
			else {
				// log::debug("Test! Would snap.");
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

				if (CCSprite* warpSprite = dynamic_cast<CCSprite*>(obj)) {

					if (warpSprite && warpSprite != pivotNode && pivotBox.intersectsRect(warpSprite->boundingBox())) {

						CCPoint result = warpSprite->getParent()->convertToWorldSpace(warpSprite->getPosition());

						pivotNode->setPosition(pivotNode->getParent()->convertToNodeSpace(result));
						m_fields->snappedTo = warpSprite;
						foundObjs++;
						break;

					}
				}

			}

			if (foundObjs == 0) {

				m_fields->snappedTo = nullptr;

				enableWarpers();

				// log::debug("No objects to snap to found");
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
				// log::debug("Attempted to snap.");
			}

			return ListenerResult::Propagate;
		}, "pivot_snap"_spr);
#endif


		return true;
	}


	virtual void ccTouchEnded(CCTouch * p0, CCEvent * p1) {
		auto snapRes = snap(true);

		if (!snapRes.first || snapRes.first && m_fields->warpSprites->containsObject(snapRes.second) && snapRes.second != m_fields->snappedTo) {
			enableWarpers();
		}

		GJTransformControl::ccTouchEnded(p0, p1);
	}

	virtual bool ccTouchBegan(CCTouch * p0, CCEvent * p1) {
		CCPoint location = p0->getStartLocation();


		if (m_fields->disabledWarps->count() > 0) {

			CCObject* disabledWarper;
			CCARRAY_FOREACH(m_fields->disabledWarps, disabledWarper) {

				CCSprite* warper = dynamic_cast<CCSprite*>(disabledWarper);
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

				// log::debug("Width: {}, Height: {}", warper->getContentWidth(), warper->getContentHeight());

				if (bounderBox.containsPoint(location)) {
					// log::debug("Touched forbidden point");
					return true;
				}

			}
		}

		m_fields->initialized = true;
		return GJTransformControl::ccTouchBegan(p0, p1);
	}

};

class $modify(TheEditorUI, EditorUI) {

	struct Fields {
		CCMenuItemSpriteExtra* snapBtn = nullptr;
	};

	static void onModify(auto & self) {
		(void)self.setHookPriority("EditorUI::init", -3000);
	}


	void onToggle() {
		auto caster = static_cast<TheTransformCtrls*>(m_transformControl);
		caster->enableWarpers();
	}

	/*
	void deselectObject() {
		onToggle();
		EditorUI::deselectObject();
	}
	*/

	void deselectAll() {
		onToggle();
		EditorUI::deselectAll();
	}

	bool init(LevelEditorLayer * editorLayer) {

		if (!EditorUI::init(editorLayer)) {
			return false;
		}

		if (Mod::get()->getSettingValue<std::string>("snap-mode") == "button") {
			auto btn = CCMenuItemSpriteExtra::create(
				CCSprite::create("GJ_snapBtn_001.png"_spr),
				this,
				menu_selector(TheEditorUI::onBtn)
			);

			CCSize size = m_unlinkBtn->getContentSize();

			float X = m_unlinkBtn->getPositionX() + (size.width / 2) + 10.f;

			//This is so it doesn't mess up that one Alphalaneous mod
			if (CCNode* linkMenu = this->getChildByID("link-menu")) {
				if (AxisLayout* layout = dynamic_cast<AxisLayout*>(linkMenu->getLayout())) {
					layout->setGap(0);
				}
				if (CCNode* zoomMenu = this->getChildByID("zoom-menu")) {
					float origScale = linkMenu->getScale();
					linkMenu->setContentHeight(140);
					linkMenu->setScale(0.8f * origScale);
					linkMenu->setPosition((zoomMenu->getPositionX() + 45 * origScale), (zoomMenu->getPositionY() + 2));
					linkMenu->setAnchorPoint(zoomMenu->getAnchorPoint());
					linkMenu->addChild(btn);
					linkMenu->updateLayout();
					m_fields->snapBtn = btn;
				}

			}
		}

		return true;

	}

	void onPlaytest(CCObject * p0) {
		if (m_fields->snapBtn) {
			m_fields->snapBtn->setVisible(false);
		}

		EditorUI::onPlaytest(p0);
	}

	void onStopPlaytest(CCObject * p0) {
		if (m_fields->snapBtn) {
			m_fields->snapBtn->setVisible(true);
		}

		EditorUI::onStopPlaytest(p0);
	}


	void onBtn(CCObject*) {
		auto caster = static_cast<TheTransformCtrls*>(m_transformControl);

		if (caster->m_fields->initialized) {
			caster->snap(false);
		}


	}

};


//TODO::
//		 Android mit Doppeltippen snappen.