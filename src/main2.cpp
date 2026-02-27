#include <geode/Geode.hpp>
#include <Geode/modify/GJTransformControl.hpp>

using namespace geode::prelude;

static std::string getSpriteFrameName(CCNode* node) {
	//cred to devtools

	if (auto textureProtocol = typeinfo_cast<CCTextureProtocol*>(node)) {
		if (auto texture = textureProtocol->getTexture()) {
			if (auto spriteNode = typeinfo_cast<CCSprite*>(node)) {

				auto* cachedFrames = CCSpriteFrameCache::sharedSpriteFrameCache()->m_pSpriteFrames;
				const auto rect = spriteNode->getTextureRect();

				for (auto [key, frame] : CCDictionaryExt<std::string, CCSpriteFrame*>(cachedFrames)) {
					if (frame->getTexture() == texture && frame->getRect() == rect) {
						return key.c_str();
						break;
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
constexpr float EPSILON = 0.001f;

class $modify(TheTransformControls, GJTransformControl) {

	struct Fields {
		CCSprite* snappedTo = nullptr;
		std::vector<CCSprite*> warpCorners;
		std::vector<CCSprite*> disabledWarpers;
	};

	/**
	* updates the warpCorners vector, which contains the
	* sprites that actually warp the object
	*/
	void updateWarpCorners() {
		m_fields->warpCorners.clear();

		for (auto v : CCArrayExt<CCSprite*>(m_warpSprites)) {
			if (getSpriteFrameName(v) == "warpBtn_02_001.png") {
				m_fields->warpCorners.push_back(v);
			}
		}

		if (m_fields->warpCorners.size() <= 0) {
			log::warn("updateWarpCorners(): no warp corner textures found, this should NOT happen, please report!");
		}

		m_fields->warpCorners.shrink_to_fit();
	}

	/**
	* loops through m_fields->warpCorners and disables warpers
	* that would cause a crash if enabled
	*/
	void updateDisabledWarpers() {

		if (!m_fields->snappedTo) return;

		float x = m_fields->snappedTo->getPositionX();
		float y = m_fields->snappedTo->getPositionY();

		// temporarily store all sprites that align on an axis
		std::vector<CCSprite*> xAlignedSprites;
		std::vector<CCSprite*> yAlignedSprites;

		// loop through every warper and store aligned sprites
		for (auto v : m_fields->warpCorners) {
			
			if (!contains(m_fields->disabledWarpers, v) && v != m_fields->snappedTo) {
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
			m_fields->disabledWarpers = std::move(xAlignedSprites);

			m_fields->disabledWarpers.insert(
				m_fields->disabledWarpers.end(),
				std::make_move_iterator(yAlignedSprites.begin()),
				std::make_move_iterator(yAlignedSprites.end())
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
	
	virtual bool init() {
		if (!this->init()) {
			return false;
		}

		auto method = Mod::get()->getSettingValue<std::string>("snap-mode");

		this->addEventListener(
			KeybindSettingPressedEventV3(Mod::get(), "snap-keybind"),
			[this, method](Keybind const& keybind, bool down, bool repeat, double timestamp) {
				if (down && !repeat && method == "keybind") {
					test();
				}
			}
		);

		return true;
	}

	void test() {
		log::debug("size of m_warpSprites: {}", m_warpSprites->count());
		for (auto v : m_fields->warpCorners) {
			log::debug("{}", v);
		}
	}





};