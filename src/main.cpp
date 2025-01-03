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
		log::debug("updatevalidsprites called");
		if (m_fields->warpSprites->count() > 0) {
			m_fields->warpSprites->removeAllObjects(); // Clear existing objects safely
		}
		log::debug("removed objects");

		const std::string textureNomenclature = "warpBtn_02_001.png";

		auto* cachedFrames = CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
		std::unordered_map<std::string, CCSpriteFrame*> frameMap;
		log::debug("did some things");

		// Populate the frame map with frames matching the nomenclature
		for (const auto& pair : CCDictionaryExt<std::string, CCSpriteFrame*>(cachedFrames)) {
			const std::string& key = pair.first;
			CCSpriteFrame* frame = pair.second;

			if (key == textureNomenclature) {
				frameMap[key] = frame;
			}
		}
		log::debug("populated frame map");

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
		log::debug("iterated over main node's children");

		if (m_fields->warpSprites->count() <= 0) {
			log::warn("updateValidSprites(): No warp sprite textures found. This shouldn't happen, report if this does.");
		}

		// 2024-12-05 4:40:30PM "yay"
	}


	void enableWarpers() {
		log::debug("enableWarpers(): called.");
		if (m_fields->disabledWarps->count() > 0) {
			log::debug("enableWarpers(): more than 0 disabled warpers.");
			//Disable the disabled. I hope that makes sense :D
			CCObject* warper;
			log::debug("enableWarpers(): gettin ready for the ccarray foreach");
			CCARRAY_FOREACH(m_fields->disabledWarps, warper) {
				log::debug("enableWarpers(): checking if {} is a valid sprite", warper);
				if (auto warperSprite = dynamic_cast<CCSprite*>(warper)) {
					log::debug("enableWarpers(): seems like {} is a valid sprite", warper);
					warperSprite->setColor({ 255, 255, 255 });
					log::debug("set color of the aformentioned warper");
				}
				else {
					log::warn("enableWarpers(): Unexpected object inside disabledWarps. This shouldn't happen, please report!!");
				}
			}
			log::debug("enableWarpers(): removing the disabled warps from the array");
			m_fields->disabledWarps->removeAllObjects();
		}
	}


	void updateDisabledWarps() {
		log::debug("updatedDisabledWarps(): got called, enabling warpers.");
		enableWarpers();
		log::debug("updatedDisabledWarps(): enabled the warpers");
		if (m_fields->snappedTo == nullptr) {
			log::debug("updatedDisabledWarps(): not snapped to anything. aborting.");
			return;
		}

		log::debug("updatedDisabledWarps(): defining disabledclr");
		ccColor3B disabledclr = { 140, 90, 90 };
		log::debug("updatedDisabledWarps(): defining the warpsprite we're snapped to from the mfields");
		CCSprite* warpSprite = m_fields->snappedTo;

		log::debug("updatedDisabledWarps(): getting the positions");
		float yPos = warpSprite->getPositionY();
		float xPos = warpSprite->getPositionX();

		log::debug("updatedDisabledWarps(): defining the amount of whatever i dont wanna type this out");
		int xCount = 0;
		int yCount = 0;

		log::debug("updatedDisabledWarps(): gettin ready for another ccarray foreach!");
		CCObject* warpSpriteObj;
		CCArray* axisAlignedSprites = CCArray::create(); // WILL contain all sprites that align on either the x or y axis
		log::debug("updatedDisabledWarps(): Were getting into the danger zone. Created a CCArray");
		axisAlignedSprites->retain();
		log::debug("updatedDisabledWarps(): Retained the new ccarray. This might be the crash. Please be. Fgs.");
		CCARRAY_FOREACH(m_fields->warpSprites, warpSpriteObj) {
			log::debug("updatedDisabledWarps(): initiated the ccarray. object being checked: {}", warpSpriteObj);
			if (CCSprite* CCwarpSprite = dynamic_cast<CCSprite*>(warpSpriteObj)) {
				log::debug("updatedDisabledWarps(): the aformentioned object has passed the dynamiccast.");
				
				if (!m_fields->disabledWarps->containsObject(CCwarpSprite) && CCwarpSprite != warpSprite) {
					log::debug("updatedDisabledWarps(): the aformentioned object has passed another test!");
					if (abs(CCwarpSprite->getPositionX() - xPos) < EPSILON) {
						log::debug("updatedDisabledWarps(): the aformentioned object is aligned with something on x idfk");
						axisAlignedSprites->addObject(CCwarpSprite);
						log::debug("updatedDisabledWarps(): added that object to some array");
						xCount++;
						log::debug("updatedDisabledWarps(): incremented x");

					}

					if (abs(CCwarpSprite->getPositionY() - yPos) < EPSILON) {
						log::debug("updatedDisabledWarps(): the aformentioned object is aligned with something on x idfk");
						axisAlignedSprites->addObject(CCwarpSprite);
						log::debug("updatedDisabledWarps(): added that object to some array");
						yCount++;
						log::debug("updatedDisabledWarps(): incremented y");
					}

				};
			}

		}

		if (axisAlignedSprites->count() != 4) { //If it's not a corner
			log::debug("updatedDisabledWarps(): its not a corner!");
			if (yCount > xCount) { //if there were more horizontal positions found than vertical ones, it must be a horizontal row
				log::debug("updatedDisabledWarps(): horiz");

				m_fields->disabledWarps->removeAllObjects();
				log::debug("updatedDisabledWarps(): horiz, killed em objects, and am gettin ready for yet another ccarray foreachh");
				CCObject* rowObject;
				CCARRAY_FOREACH(m_fields->warpSprites, rowObject) {
					log::debug("updatedDisabledWarps(): horiz, jiasdgjisd");
					if (CCSprite* rowObj = dynamic_cast<CCSprite*>(rowObject)) {
						log::debug("updatedDisabledWarps(): horiz, i dont understand my own code anymore, {}", rowObj);
						if (abs(rowObj->getPositionY() - yPos) < EPSILON && rowObj != warpSprite) {
							log::debug("updatedDisabledWarps(): horiz, yea something matches up, adding it to some array");
							m_fields->disabledWarps->addObject(rowObj);
							log::debug("updatedDisabledWarps(): horiz, i think i did that im gonna go insane");
						}
					}

				}

			}
			else if (xCount > yCount) { // if there were more vertical positions found, must be a vertical row then, right?
				log::debug("updatedDisabledWarps(): vertical");

				m_fields->disabledWarps->removeAllObjects();
				log::debug("updatedDisabledWarps(): vertic, killed dem objects, and am gettin ready for yet another ccarray foreachh");
				CCObject* columnObject;
				CCARRAY_FOREACH(m_fields->warpSprites, columnObject) {
					log::debug("updatedDisabledWarps(): vertic, obj is in the ccarray, {}", columnObject);

					if (CCSprite* columnObj = dynamic_cast<CCSprite*>(columnObject)) {
						log::debug("updatedDisabledWarps(): vertic, columbnobj passed some dynacmiccast");
						if (abs(columnObj->getPositionX() - xPos) < EPSILON && columnObj != warpSprite) {
							log::debug("updatedDisabledWarps(): vertic, PASSED ABS....");
							m_fields->disabledWarps->addObject(columnObj);
							log::debug("updatedDisabledWarps(): vertic, added object to clumnom...");
						}
					}

				}

			}

		}
		else {
			log::debug("hi vitox");
			CCObject* axisObj;
			CCARRAY_FOREACH(axisAlignedSprites, axisObj) {
				log::debug("thanks for helping me with fixing this");
				if (CCSprite* axisObject = dynamic_cast<CCSprite*>(axisObj)) {
					log::debug("i appreciate you taking your time with this");
					m_fields->disabledWarps->addObject(axisObject);
					log::debug("especially cuz i keep on sending you random files");
				}
			}

		}

		// applying colors because visual stuff gives me dopamine
		CCObject* warp;
		log::debug("Look at those disabled warps: {}", m_fields->disabledWarps);
		CCARRAY_FOREACH(m_fields->disabledWarps, warp) {
			log::debug("dude im ccarraying something aggain, look, {}", warp);
			if (CCSprite* warpObject = dynamic_cast<CCSprite*>(warp)) {
				log::debug("and that object seems to actually be a ccsprite");
				warpObject->setColor(disabledclr);
				log::debug("im sorry for your console being this clogged.");
			}
			
		}
		// It's as shrimple as that!!!

		log::debug("releaseing memory. im finally done with typing. u agthe tit");
		axisAlignedSprites->release(); // memory not leaking anymor
	};

	std::pair<bool, CCSprite*> snap(bool test) {
		log::debug("snap called!");

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
			log::debug("updating valid sprites");
			updateValidSprites();
			log::debug("snap(): updated the valid sprites.");

			if (!m_mainNodeParent->isVisible()) {
				return std::make_pair(false, nullptr);
				log::debug("mainnodeparent is invisible");
			}


			CCSprite* pivotNode = GJTransformControl::spriteByTag(1);
			log::debug("gjt spritebytag1");
			CCArray* targets = m_fields->warpSprites;
			CCRect pivotBox = pivotNode->boundingBox();

			int foundObjs = 0;

			log::debug("gettin ready for the ccarray foreach");
			CCObject* obj;
			CCARRAY_FOREACH(targets, obj) {
				log::debug("im looking if {} is touching the pivot node", obj);

				if (CCSprite* warpSprite = dynamic_cast<CCSprite*>(obj)) {
					log::debug("correctly dynamically casted.");
					if (warpSprite && warpSprite != pivotNode && pivotBox.intersectsRect(warpSprite->boundingBox())) {
						log::debug("it passed various tests just now");
						CCPoint result = warpSprite->getParent()->convertToWorldSpace(warpSprite->getPosition());
						log::debug("i got the result position");

						pivotNode->setPosition(pivotNode->getParent()->convertToNodeSpace(result));
						log::debug("i set position of pivotnode");
						m_fields->snappedTo = warpSprite;
						log::debug("set internal value for where the thing is snapped to. magic snapple");
						foundObjs++;
						log::debug("incremented found objects");
						break;
						log::debug("cancelling, cuz i found snap object");
					}
				}

			}
			log::debug("something inbetween! doing some if statement now.");
			if (foundObjs == 0) {
				log::debug("there are no found objects");
				m_fields->snappedTo = nullptr;
				log::debug("reset the snappedTo internal value. calling enableWarpers()");
				enableWarpers();
				log::debug("enableWarpers should be done.");
				// log::debug("No objects to snap to found");
			}

			log::debug("refreshing gj controls");
			GJTransformControl::refreshControl();
			log::debug("refreshed gj controls, calling updateDisabledWarps");
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