/**
 * Include the Geode headers
 */
#include <Geode/Geode.hpp>
#include <Geode/modify/GJTransformControl.hpp>
#include <Geode/modify/EditorUI.hpp>

/*
#ifndef GEODE_IS_ANDROID
#include <geode.custom-keybinds/include/Keybinds.hpp>
#endif
*/

using namespace geode::prelude;

/*
#ifndef GEODE_IS_ANDROID
using namespace keybinds;

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
*/


const float EPSILON = 0.001f;

class $modify(TheTransformCtrls, GJTransformControl) {

	struct Fields {

		bool initialized = false;
		CCSprite* snappedTo = nullptr;
		CCArray* disabledWarps;
		CCArray* warpSprites;

		Fields() : disabledWarps(CCArray::create()), warpSprites(CCArray::create()) {
			disabledWarps->retain();
			warpSprites->retain();
		}

		~Fields() {
			disabledWarps->release();
			warpSprites->release();
		}

	};

	bool objectsCheck() {
		log::debug("{}", m_objects);
		if (m_objects && m_objects != nullptr) {
			// log::debug("mandatory string, {}", m_unk1);
			return true;
		}

		// log::debug("i think its nullptr, {}", m_unk1);
		return false;
	}

	

	void updateValidSprites() {

		m_warpSprites->removeAllObjects();

		const std::string textureNomenclature = "warpBtn_02_001.png";

		// I took this from devtools, thanks! :3

		auto* cachedFrames = CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
		std::unordered_map<std::string, CCSpriteFrame*> frameMap;

		
		for (const auto& pair : CCDictionaryExt<std::string, CCSpriteFrame*>(cachedFrames)) {
			const std::string& key = pair.first;
			CCSpriteFrame* frame = pair.second;
			
			if (key == textureNomenclature) {
				frameMap[key] = frame;
			}
		}

		
		for (auto spriteNode : CCArrayExt<CCSprite*>(m_mainNodeParent->getChildren())) {
			auto rect = spriteNode->getTextureRect();

			auto it = frameMap.find(textureNomenclature);
			if (it != frameMap.end() && it->second->getRect() == rect) {
				m_fields->warpSprites->addObject(spriteNode);
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
			
			for (auto warperSprite : CCArrayExt<CCSprite*>(m_fields->disabledWarps)) {
				warperSprite->setColor({ 255, 255, 255 });
			}

			m_fields->disabledWarps->removeAllObjects();
		}
	}


	void updateDisabledWarps() {
		log::debug("Hello i was called");
		enableWarpers();

		if (m_fields->snappedTo == nullptr) {
			return;
		}

		ccColor3B disabledclr = { 140, 90, 90 };
		CCSprite* snappedTo = m_fields->snappedTo;

		float yPos = snappedTo->getPositionY();
		float xPos = snappedTo->getPositionX();

		int xCount = 0;
		int yCount = 0;

		std::vector<CCSprite*> axisAlignedSprites; // WILL contain all sprites that align on either the x or y axis

		for (auto CCwarpSprite : CCArrayExt<CCSprite*>(m_fields->warpSprites)) {

			if (!m_fields->disabledWarps->containsObject(CCwarpSprite) && CCwarpSprite != snappedTo) {
				if (abs(CCwarpSprite->getPositionX() - xPos) < EPSILON) {

					axisAlignedSprites.push_back(CCwarpSprite);
					xCount++;
					continue;

				}

				if (abs(CCwarpSprite->getPositionY() - yPos) < EPSILON) {

					axisAlignedSprites.push_back(CCwarpSprite);
					yCount++;
					continue;

				}

			};
		}

		log::debug("{}", axisAlignedSprites.size());
		if (axisAlignedSprites.size() != 4) { //If it's not a corner
			m_fields->disabledWarps->removeAllObjects();


			if (yCount > xCount) { //if there were more horizontal positions found than vertical ones, it must be a horizontal row

				for (auto rowObj : CCArrayExt<CCSprite*>(m_fields->warpSprites)) {
					if (abs(rowObj->getPositionY() - yPos) < EPSILON && rowObj != snappedTo) {
						m_fields->disabledWarps->addObject(rowObj);
					}
				}

			}
			else if (xCount > yCount) { // if there were more vertical positions found, must be a vertical row then, right?


				for (auto columnObj : CCArrayExt<CCSprite*>(m_fields->warpSprites)) {
					if (abs(columnObj->getPositionX() - xPos) < EPSILON && columnObj != snappedTo) {
						m_fields->disabledWarps->addObject(columnObj);
					}
				}

			}

		} else { // If it's a corner

			m_fields->disabledWarps->removeAllObjects();
			for (auto axisObject : axisAlignedSprites) {
				m_fields->disabledWarps->addObject(axisObject);
			}

		}

		for (auto v : CCArrayExt<CCSprite*>(m_fields->disabledWarps)) {
			v->setColor(disabledclr);
		}
	};

	std::pair<bool, CCSprite*> snap(bool test) {

		if (!objectsCheck()) {
			log::warn("the warp tool seems to not be properly initialized. If you have the warp tool open and you see this, please report !!!");
			return std::make_pair(false, nullptr);
		}

		if (test) { // Testing mode: Returns if the pivot WOULD snap or not. Im repeating lots of code but i dont wanna make a separate function for this
			if (!m_fields->initialized || !m_mainNodeParent->isVisible()) {
				return std::make_pair(false, nullptr);
			}

			updateValidSprites();

			CCSprite* pivotNode = GJTransformControl::spriteByTag(1);
			CCRect pivotBox = pivotNode->boundingBox();

			int foundObjs = 0;

			
			CCSprite* result;
			for (auto v : CCArrayExt<CCSprite*>(m_fields->warpSprites)) {
				if (v && v != pivotNode && pivotBox.intersectsRect(v->boundingBox())) {
					result = v;
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
			CCRect pivotBox = pivotNode->boundingBox();

			int foundObjs = 0;


			for (auto v : CCArrayExt<CCSprite*>(m_fields->warpSprites)) {

				if (v && v != pivotNode && pivotBox.intersectsRect(v->boundingBox())) {

					CCPoint result = v->getParent()->convertToWorldSpace(v->getPosition());

					pivotNode->setPosition(pivotNode->getParent()->convertToNodeSpace(result));
					m_fields->snappedTo = v;
					foundObjs++;
					break;

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


	virtual bool init() {
		// log::debug("initialized!");
		if (!GJTransformControl::init()) {
			return false;
		}

		enableWarpers();

#ifndef GEODE_IS_ANDROID
		auto method = Mod::get()->getSettingValue<std::string>("snap-mode");


		this->addEventListener(
			KeybindSettingPressedEventV3(Mod::get(), "snap-keybind"),
			[this, method](Keybind const& keybind, bool down, bool repeat, double timestamp) {
				if (down && !repeat && method == "keybind" && objectsCheck()) {
					snap(false);
				}
			}
		);

		/* old ck code
		this->template addEventListener<InvokeBindFilter>([=, this](InvokeBindEvent* event) {
			if (event->isDown() && method == "keybind" && munkCheck()) {
				snap(false);
				// log::debug("Attempted to snap.");
			}

			return ListenerResult::Propagate;
		}, "pivot_snap"_spr);
		*/
#endif

		m_fields->initialized = true;
		return true;
	}
	

	virtual void ccTouchEnded(CCTouch * p0, CCEvent * p1) {
		auto snapRes = snap(true);

		if (!snapRes.first || snapRes.first && m_fields->warpSprites->containsObject(snapRes.second) && snapRes.second != m_fields->snappedTo) {
			enableWarpers();
		}

		GJTransformControl::ccTouchEnded(p0, p1);
	}

	virtual bool ccTouchBegan(CCTouch* p0, CCEvent* p1) {
		

		if (m_fields->disabledWarps->count() > 0) {
			CCPoint location = p0->getStartLocation();
			
			for (auto warper : CCArrayExt<CCSprite*>(m_fields->disabledWarps)) {
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


	bool init(LevelEditorLayer * editorLayer) {

		if (!EditorUI::init(editorLayer)) {
			return false;
		}


		NodeIDs::provideFor(this);

		if (Mod::get()->getSettingValue<std::string>("snap-mode") == "button") {
			auto btn = CCMenuItemSpriteExtra::create(
				CCSprite::create("GJ_snapBtn_001.png"_spr),
				this,
				menu_selector(TheEditorUI::onBtn)
			);
			
			CCSize size = m_unlinkBtn->getContentSize();

			float X = m_unlinkBtn->getPositionX() + (size.width / 2) + 10.f;

			
			if (CCNode* linkMenu = this->getChildByID("link-menu")) {
				if (AxisLayout* layout = typeinfo_cast<AxisLayout*>(linkMenu->getLayout())) {
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

	/*
	virtual bool ccTouchBegan(CCTouch* p0, CCEvent* p1) {
		log::debug("blud");
		auto caster = static_cast<TheTransformCtrls*>(m_transformControl);
		caster->munk1();
		EditorUI::ccTouchBegan(p0, p1);
	}
	*/

	void onToggle() {
		static_cast<TheTransformCtrls*>(m_transformControl)->enableWarpers();
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

	void onPlaytest(CCObject * p0) {
		if (m_fields->snapBtn) {
			m_fields->snapBtn->setVisible(false);
		}

		EditorUI::onPlaytest(p0);
	}

	void playtestStopped() {
		if (m_fields->snapBtn) {
			m_fields->snapBtn->setVisible(true);
		}

		EditorUI::playtestStopped();
	}


	void onBtn(CCObject*) {
		auto caster = static_cast<TheTransformCtrls*>(m_transformControl);

		if (caster->m_fields->initialized) {
			caster->snap(false);
		}

	}

};