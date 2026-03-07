#include <Geode/Geode.hpp>
#include <Geode/modify/GJTransformControl.hpp>

#include "Geode/modify/Modify.hpp"

#include "TheEditorUI.hpp"
#include "TheTransformControls.hpp"

using namespace geode::prelude;

#define m_fields this->m_fields

static std::string getSpriteFrameName(CCNode* node) {
	//cred to devtools

	if (auto textureProtocol = typeinfo_cast<CCTextureProtocol*>(node)) {
		if (auto texture = textureProtocol->getTexture()) {
			if (auto spriteNode = typeinfo_cast<CCSprite*>(node)) {

				auto* cachedFrames = CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
				const auto rect = spriteNode->getTextureRect();

				for (auto [key, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(cachedFrames)) {
					if (frame->getTexture() == texture && frame->getRect() == rect) {
						return key;
					}
				}

			}
		}
	}

	return "";
}

template <typename T>
static bool contains(const std::vector<T>& vec, const T& value)
{
	return std::find(vec.begin(), vec.end(), value) != vec.end();
}

constexpr ccColor3B disabledClr = { 140, 90, 90  };
constexpr ccColor3B white = { 255, 255, 255 };
constexpr ccColor3B green = { 102, 255, 102 };
constexpr float EPSILON = 0.005f;



struct Fields {
	bool initialized = false;
	bool draggingPoint = false;
	CCSprite* snappedTo = nullptr;
	std::vector<CCSprite*> warpCorners;
	std::vector<CCSprite*> disabledWarpers;
};

// obvious methinks
bool TheTransformControls::isEnabled() {
	return this->isVisible();
}

/**
* just restores everything to original state or something idfk anymore
*/
void TheTransformControls::enableAll() {
	if (m_fields->snappedTo) {
		m_fields->snappedTo->setColor(white);
	}
	m_fields->snappedTo = nullptr;

	updateDisabledWarpers();
	updateWarpCorners();
}

/**
* updates the warpCorners vector, which contains the
* sprites that actually warp the object
*/
void TheTransformControls::updateWarpCorners() {
	m_fields->warpCorners.clear();

	for (auto v : CCArrayExt<CCSprite*>(m_warpSprites)) {
		if (getSpriteFrameName(v) == "warpBtn_02_001.png") {
			m_fields->warpCorners.push_back(v);
		}
	}

	if (m_fields->warpCorners.empty()) {
		log::warn("updateWarpCorners(): no warp corner textures found, this should NOT happen, please report!");
	}

	m_fields->warpCorners.shrink_to_fit();
}

/**
* loops through m_fields->warpCorners and disables warpers
* that would cause a crash if enabled
*/
void TheTransformControls::updateDisabledWarpers() {
	for (auto v : m_fields->disabledWarpers) {
		v->setColor(white);
	}

	m_fields->disabledWarpers.clear();

	if (m_fields->snappedTo == nullptr) return;

	float x = m_fields->snappedTo->getPositionX();
	float y = m_fields->snappedTo->getPositionY();

	// temporarily store all sprites that align on an axis
	std::vector<CCSprite*> xAlignedSprites;
	std::vector<CCSprite*> yAlignedSprites;

	// loop through every warper and store aligned sprites
	for (auto v : m_fields->warpCorners) {
		
		if (v != m_fields->snappedTo) {
			if (std::abs(v->getPositionX() - x) < EPSILON) {
				xAlignedSprites.push_back(v);
				continue;
			}

			if (std::abs(v->getPositionY() - y) < EPSILON) {
				yAlignedSprites.push_back(v);
				continue;
			}
		}

	}

	if (xAlignedSprites.size() + yAlignedSprites.size() >= 4) {
		// it's a corner
		m_fields->disabledWarpers = xAlignedSprites;

		m_fields->disabledWarpers.insert(
			m_fields->disabledWarpers.end(),
			yAlignedSprites.begin(),
			yAlignedSprites.end()
		);
	} else {
		// it's not a corner!
		bool moreX = xAlignedSprites.size() > yAlignedSprites.size();

		if (moreX) {
			m_fields->disabledWarpers = std::move(xAlignedSprites);
		} else {
			m_fields->disabledWarpers = std::move(yAlignedSprites);
		}

	}

	// color
	for (auto v : m_fields->disabledWarpers) {
		v->setColor(disabledClr);
	}

}

bool TheTransformControls::performSnap(bool test) {
	if (!m_fields->initialized || !m_mainNode->isVisible()) { log::debug("HELP! attempted to snap without stuf being properly initialized, what the hell??"); return false; }

	// Safety i'm playing it very safe here hi
	auto ui = m_delegate ? m_delegate->getUI() : nullptr;
	auto editor = static_cast<TheEditorUI*>(ui);

	if (!editor) {
		log::warn("HELP! EditorUI missing");
		return false;
	}
	#undef m_fields
	if (!editor->m_fields->transformActive) {
		log::debug("HELP! attempted to snap but transform isnt active. what the hell man");
		return false;
	}
	#define m_fields this->m_fields
	// if any index mod reads this, please lmk how to avoid this #define stuff because this feels very unclean ...


	// refresh every warp corner
	updateWarpCorners();
	
	// this is equivalent to GJTransformControls::spriteByTag(), but im using ts instead because the inline definition
	// isnt implemented in ios yet
	CCSprite* pivotNode = static_cast<cocos2d::CCSprite*>(m_warpSprites->objectAtIndex(1));
	CCRect hitbawx = pivotNode->boundingBox();

	bool wouldsnap = false;

	// loop through all warp sprites to see if one collides with the pivotnode
	for (auto v : m_fields->warpCorners) {

		// If v exists, is not the pivot node, and the pivot node touches a warpcorner
		if (v && v != pivotNode && hitbawx.intersectsRect(v->boundingBox())) {
			wouldsnap = true;

			if (!test) {
				CCPoint res = v->getParent()->convertToWorldSpace(v->getPosition());
				pivotNode->setPosition(pivotNode->getParent()->convertToNodeSpace(res));
				m_fields->snappedTo = v;
				m_fields->snappedTo->setColor(green);
			}

			break;

		}
	}

	if (!test && wouldsnap) {
		updateDisabledWarpers();
		GJTransformControl::refreshControl();
	}

	return wouldsnap;

	/* evil code DONT DO THIS THIS IS BAD BAD BAD
	auto self = reinterpret_cast<uintptr_t>(this);

	*(short*)(self + 0x74) = 1;
	

	static uint8_t touch[0x40];
	memset(touch, 0, sizeof(touch));
	*(int*)(touch + 0x38) = *(int*)(self + 0x70);

	GJTransformControl::ccTouchEnded(reinterpret_cast<CCTouch*>(touch), nullptr);
	*/
}

/*
void test() {
	//log::debug("size of m_warpSprites: {}", m_warpSprites->count());
	TheTransformControls::performSnap(false);
	for (auto v : m_fields->warpCorners) {
		//log::debug("{}", v);
	}
}
*/

$override
bool TheTransformControls::init() {
	if (!GJTransformControl::init()) {
		return false;
	}


	this->addEventListener(
		KeybindSettingPressedEventV3(Mod::get(), "snap-keybind"),
		[this](Keybind const& keybind, bool down, bool repeat, double timestamp) {
			if (down && !repeat) {
				performSnap(false);
			}
		}
	);

	return true;
}

$override
bool TheTransformControls::ccTouchBegan(CCTouch* touch, CCEvent* event) {
	// this is equivalent to GJTransformControls::spriteByTag(), but im using ts instead because the inline definition
	// isnt implemented in ios yet..
	CCSprite* pivotNode = static_cast<cocos2d::CCSprite*>(m_warpSprites->objectAtIndex(1));
	CCPoint pos = pivotNode->getParent()->convertToNodeSpace(touch->getLocation());

	if (pivotNode->boundingBox().containsPoint(pos)) {
		// log::debug("Started dragging point..");
		m_fields->draggingPoint = true;
	}

	/*
		sink input when its over a disabled sprite
	*/
	if (m_fields->disabledWarpers.size() > 0) {
		CCPoint worldPos = touch->getLocation();

		for (CCSprite* v : m_fields->disabledWarpers) {
			CCPoint world = v->getParent()->convertToWorldSpace(v->getPosition());
			CCRect sink = CCRect(
				world.x - (v->getContentWidth() * 1.1) / 2,
				world.y - (v->getContentHeight() * 1.1) / 2,
				v->getContentWidth() * 1.1,
				v->getContentHeight() * 1.1
			);

			if (sink.containsPoint(worldPos)) {
				return true; // sink
			}

		}
	}


	/**
	if (performSnap(true)) {
		m_fields->draggingPoint = true;
	} else {
		m_fields->draggingPoint = false;
	}
	*/
	
	return GJTransformControl::ccTouchBegan(touch, event);
}

$override
void TheTransformControls::ccTouchMoved(CCTouch* touch, CCEvent* event) {
	if (m_fields->draggingPoint) {

		if (m_fields->snappedTo) {
			m_fields->snappedTo->setColor(white);
			m_fields->snappedTo = nullptr;
		}

		enableAll();
	}

	GJTransformControl::ccTouchMoved(touch, event);
}

$override
void TheTransformControls::ccTouchEnded(CCTouch* touch, CCEvent* event) {
	//log::debug("Stopped dragging point..");
	m_fields->draggingPoint = false;

	if (performSnap(true) && m_fields->snappedTo == nullptr) {
		enableAll();
	}
	//log::debug("ccTouchEnded");

	GJTransformControl::ccTouchEnded(touch, event);
}