#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include "UI/NongLayer.hpp"

#include <string>

USE_GEODE_NAMESPACE();

class $modify(MyLevelInfoLayer, LevelInfoLayer){
	bool init(GJGameLevel* level){
		if (!LevelInfoLayer::init(level)) {
			return false;
		}

		auto rightSideMenu = this->getChildByID("right-side-menu");
		auto sprite = CircleButtonSprite::createWithSprite("test_button.png"_spr);
		auto nongButton = CCMenuItemSpriteExtra::create(
			sprite,
			this,
			menu_selector(NongLayer::onNongLayer)
		);
		nongButton->setTag(level->m_songID);

		rightSideMenu->addChild(nongButton);
		rightSideMenu->updateLayout();
		return true;
	}
};
