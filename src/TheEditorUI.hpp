#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

class TheTransformControls;

class $modify(TheEditorUI, EditorUI) {
public:
	struct Fields {
		CCMenuItemSpriteExtra* snapBtn = nullptr;
		TheTransformControls* pivotsnap = nullptr;

		bool firstInitialized = false;
		bool transformActive = false;
	};

	$override
	bool init(LevelEditorLayer* lel);

	void onBtn(CCObject* sender);

	$override
	void activateTransformControl(CCObject* sender);

	$override
	void deactivateTransformControl();

private:
	void enabler();
};