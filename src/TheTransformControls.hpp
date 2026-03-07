#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/GJTransformControl.hpp>

using namespace geode::prelude;

struct TheTransformControls : Modify<TheTransformControls, GJTransformControl> {
	struct Fields {
		bool initialized;
		bool draggingPoint;

		CCSprite* snappedTo;

		std::vector<CCSprite*> warpCorners;
		std::vector<CCSprite*> disabledWarpers;
	};

	bool isEnabled();
	void enableAll();
	void updateWarpCorners();
	void updateDisabledWarpers();
	bool performSnap(bool test);

	virtual bool init() override;
	virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
	virtual void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
	virtual void ccTouchEnded(CCTouch* touch, CCEvent* event) override;

};
