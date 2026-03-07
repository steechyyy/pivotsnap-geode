#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

class TheTransformControls;

struct TheEditorUI : Modify<TheEditorUI, EditorUI> {
public:
	struct Fields {
		CCMenuItemSpriteExtra* snapBtn = nullptr;
		TheTransformControls* pivotsnap = nullptr;

		bool firstInitialized = false;
		bool transformActive = false;
	};

	void enabler();

	$override
	bool init(LevelEditorLayer* lel);
	
	void onSnapBtn(CCObject* sender);

	$override
	void activateTransformControl(CCObject* sender);

	$override
	void deactivateTransformControl();

};