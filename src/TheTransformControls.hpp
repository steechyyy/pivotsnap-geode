#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/GJTransformControl.hpp>

using namespace geode::prelude;

struct TheTransformControls : Modify<TheTransformControls, GJTransformControl> {
	struct Fields {
		bool initialized;
		bool draggingPoint;

		CCSprite* snappedTo;

		std::vector<CCSprite*> warpers;
		std::vector<CCSprite*> disabledWarpers;
	};

	// returns whether the warp ctrls are enabled or not
	bool isEnabled();

	// restores everything to original state (e.g. when warpctrls are disabled)
	void enableAll();

	// refreshes the warpers vector
	void updateWarpers();

	// updates the warpers that should be disabled. im disabling those because they would cause a crash if otherwise enabled
	void updateDisabledWarpers();

	// perform the actual snap. "test" returns if it would snap or not
	bool performSnap(bool test);

	// hooks
	virtual bool init() override;
	virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
	virtual void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
	virtual void ccTouchEnded(CCTouch* touch, CCEvent* event) override;

};
