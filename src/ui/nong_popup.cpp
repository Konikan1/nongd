#include "nong_popup.hpp"

bool NongPopup::setup(int songID, CustomSongWidget* parent) {
    this->m_songID = songID;
    this->m_parentWidget = parent;

    auto title = "NONGs for " + std::to_string(songID);
    this->setTitle(title);

    this->setSongs();
    this->createList();
    this->createAddButton();
    this->createRemoveAllButton();
    this->createSettingsButton();
    this->createFetchSongHubMenu();
    return true;
}

void NongPopup::createAddButton() {
    this->m_addButtonMenu = CCMenu::create();
    this->m_addButtonMenu->setID("add-button-menu");
    auto addButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png"),
        this,
        menu_selector(NongPopup::openAddPopup)
    );
    addButton->setID("add-button");
    
    this->m_addButtonMenu->addChild(addButton);
    auto centered = CCDirector::get()->getWinSize() / 2;
    this->m_addButtonMenu->setPosition(centered.width + this->getPopupSize().width / 2 - 10.f, centered.height - this->getPopupSize().height / 2 + 10.f);
    this->m_mainLayer->addChild(m_addButtonMenu);
}

void NongPopup::createRemoveAllButton() {
    auto menu = CCMenu::create();
    menu->setID("remove-all-menu");
    auto sprite = ButtonSprite::create("Remove all");
    sprite->setScale(0.8f);
    auto button = CCMenuItemSpriteExtra::create(
        sprite,
        this,
        menu_selector(NongPopup::deleteAllNongs) 
    );
    button->setID("remove-all-button");

    menu->addChild(button);
    auto windowSize = CCDirector::sharedDirector()->getWinSize();
    auto center = windowSize / 2;
    menu->setPosition(center.width, (center.height - this->getListSize().height / 2) - 24.5f);
    menu->setZOrder(10);
    this->m_mainLayer->addChild(menu);
}

void NongPopup::createSettingsButton() {
    auto menu = CCMenu::create();
    menu->setID("settings-menu");
    auto sprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
    sprite->setScale(0.8f);
    auto settingsButton = CCMenuItemSpriteExtra::create(
        sprite,
        this,
        menu_selector(NongPopup::onSettings)
    );
    settingsButton->setID("settings-button");

    menu->addChild(settingsButton);
    auto center = CCDirector::sharedDirector()->getWinSize() / 2;
    menu->setPosition(center.width + this->getPopupSize().width / 2 - 30.f, center.height + this->getPopupSize().height / 2 - 30.f);
    this->m_mainLayer->addChild(menu);
}

void NongPopup::onSettings(CCObject*) {
    openSettingsPopup(Mod::get());
}

void NongPopup::createFetchSongHubMenu() {
    auto menu = CCMenu::create();
    menu->setID("fetch-song-hub-menu");
    auto sprite = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
    auto fetchButton = CCMenuItemSpriteExtra::create(
        sprite,
        this,
        menu_selector(NongPopup::fetchSongHub)
    );
    fetchButton->setID("fetch-song-hub-button");

    menu->addChild(fetchButton);
    auto centered = CCDirector::get()->getWinSize() / 2;
    menu->setPosition(centered.width - this->getPopupSize().width / 2 + 10.f, centered.height - this->getPopupSize().height / 2 + 10.f);
    this->m_fetchSongHubMenu = menu;
    this->m_mainLayer->addChild(menu);
}

NongPopup* NongPopup::create(int songID, CustomSongWidget* parent) {
    auto ret = new NongPopup();
    auto size = ret->getPopupSize();
    if (ret && ret->init(size.width, size.height, songID, parent)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

CCSize NongPopup::getPopupSize() const {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto width = std::min(winSize.width - 50.f, 500.f);
    return { width, 280.f };
}

void NongPopup::setSongs() {
    m_songs = NongManager::get()->getNongs(this->m_songID);
}

CCArray* NongPopup::createNongCells() {
    auto songs = CCArray::create();
    // auto activeSong = this->getActiveSong();

    // songs->addObject(NongCell::create(activeSong, this, this->getCellSize(), true, activeSong.path == this->m_songs.defaultPath));

    // for (auto song : m_songs.songs) {
    //     if (m_songs.active == song.path) {
    //         continue;
    //     }
    //     songs->addObject(NongCell::create(song, this, this->getCellSize(), false, song.path == this->m_songs.defaultPath));
    // }

    return songs;
}

void NongPopup::deleteAllNongs(CCObject*) {
    createQuickPopup("Delete all nongs", 
        "Are you sure you want to <cr>delete all nongs</c> for this song?", 
        "No", 
        "Yes",
        [this](auto, bool btn2) {
            if (!btn2) {
                return;
            }

            m_songs = NongManager::get()->deleteAll(this->m_songID);
            this->updateParentWidget(this->getActiveSong());
            this->m_listLayer->removeAllChildrenWithCleanup(true);
            this->m_mainLayer->removeChild(m_listLayer);
            this->m_listLayer = nullptr;
            this->setSongs();
            this->createList();
            FLAlertLayer::create("Success", "All nongs were deleted successfully!", "Ok")->show();
        }
    );
}

SongInfo NongPopup::getActiveSong() {
    for (auto song : m_songs.songs) {
        if (song.path == m_songs.active) {
            return song;
        }
    }
    
    m_songs.active = m_songs.defaultPath;
    NongManager::get()->saveNongs(m_songs, this->m_songID);
    for (auto song : m_songs.songs) {
        if (song.path == m_songs.active) {
            return song;
        }
    }

    throw std::runtime_error("If you somehow reached this, good job.");
}

void NongPopup::saveSongsToJson() {
    NongManager::get()->saveNongs(this->m_songs, this->m_songID);
}

CCSize NongPopup::getCellSize() const {
    return {
        this->getListSize().width,
        60.f
    };
}

CCSize NongPopup::getListSize() const {
    return { 400.f, 190.f };
}

void NongPopup::createList() {
    auto cells = this->createNongCells();
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    this->m_listLayer = CCLayer::create();
    this->m_listLayer->setID("nong-list-layer");
    this->m_list = ListView::create(
        cells,
        this->getCellSize().height,
        this->getListSize().width,
        this->getListSize().height
    );
    this->m_list->setID("nong-list");

    this->m_list->setPositionY(-10.f);

    auto sideLeft = CCSprite::createWithSpriteFrameName("GJ_commentSide_001.png");
    sideLeft->setAnchorPoint(ccp(0, 0));
    sideLeft->setScaleY(5.45f);
    sideLeft->setScaleX(1.2f);
    sideLeft->setPositionX(-3.f);
    sideLeft->setZOrder(9);

    auto sideTop = CCSprite::createWithSpriteFrameName("GJ_commentTop_001.png");
    sideTop->setAnchorPoint(ccp(0, 0));
    sideTop->setScaleX(1.15f);
    sideTop->setPosition(ccp(-3.f, 163.f));
    sideTop->setZOrder(9);

    auto sideBottom = CCSprite::createWithSpriteFrameName("GJ_commentTop_001.png");
    sideBottom->setFlipY(true);
    sideBottom->setAnchorPoint(ccp(0, 0));
    sideBottom->setPosition(ccp(-3.f, -15.f));
    sideBottom->setScaleX(1.15f);
    sideBottom->setZOrder(9);

    auto sideRight = CCSprite::createWithSpriteFrameName("GJ_commentSide_001.png");
    sideRight->setFlipX(true);
    sideRight->setScaleY(5.45f);
    sideRight->setScaleX(1.2f);
    sideRight->setAnchorPoint(ccp(0, 0));
    sideRight->setPositionX(396.f);
    sideRight->setZOrder(9);

    this->m_listLayer->addChild(sideLeft);
    this->m_listLayer->addChild(sideTop);
    this->m_listLayer->addChild(sideBottom);
    this->m_listLayer->addChild(sideRight);
    this->m_listLayer->addChild(this->m_list);
    this->m_listLayer->setPosition(winSize / 2 - m_list->getScaledContentSize() / 2);
    this->m_mainLayer->addChild(m_listLayer);
}

void NongPopup::setActiveSong(SongInfo const& song) {
    if (!ghc::filesystem::exists(song.path) && song.path != this->m_songs.defaultPath && song.songUrl != "local") {
        this->setKeyboardEnabled(false);
        this->setKeypadEnabled(false);
        this->setMouseEnabled(false);
        auto loading = LoadingCircle::create();
        loading->setParentLayer(this);
        loading->setFade(true);
        loading->show();
        NongManager::get()->downloadSong(song, [this, song, loading](double progress) {
            if (progress == 100.f) {
                loading->fadeAndRemove();
                this->updateParentSizeAndIDLabel(song);
                this->setKeyboardEnabled(true);
                this->setKeypadEnabled(true);
                this->setMouseEnabled(true);
            }
        },
        [this, loading](SongInfo const& song, std::string const& error) {
            this->setKeyboardEnabled(true);
            this->setKeypadEnabled(true);
            this->setMouseEnabled(true);
            loading->fadeAndRemove();
            FLAlertLayer::create("Failed", "Failed to download song", "Ok")->show();

            for (auto song : this->m_songs.songs) {
                if (song.path == this->m_songs.defaultPath) {
                    this->setActiveSong(song);
                }
            }
        });
    }

    this->m_songs.active = song.path;

    this->saveSongsToJson();
    
    this->updateParentWidget(song);

    this->m_listLayer->removeAllChildrenWithCleanup(true);
    this->m_mainLayer->removeChild(m_listLayer);
    this->m_listLayer = nullptr;
    this->createList();
}

void NongPopup::openAddPopup(CCObject* target) {
    // auto popup = NongAddPopup::create(this);
    // popup->show();
}

void NongPopup::addSong(SongInfo const& song) {
    for (auto savedSong : this->m_songs.songs) {
        if (song.path.string() == savedSong.path.string()) {
            FLAlertLayer::create("Error", "This NONG already exists! (<cy>" + savedSong.songName + "</c>)", "Ok")->show();
            return;
        }
    }
    NongManager::get()->addNong(song, this->m_songID);
    this->updateParentWidget(song);
    FLAlertLayer::create("Success", "The song was added!", "Ok")->show();
    this->m_listLayer->removeAllChildrenWithCleanup(true);
    this->m_mainLayer->removeChild(m_listLayer);
    this->m_listLayer = nullptr;
    this->setSongs();
    this->createList();
}

void NongPopup::updateParentWidget(SongInfo const& song) {
    this->m_parentWidget->m_songInfo->m_artistName = song.authorName;
    this->m_parentWidget->m_songInfo->m_songName = song.songName;
    if (song.songUrl != "local") {
        this->m_parentWidget->m_songInfo->m_songURL = song.songUrl;
    }
    this->m_parentWidget->updateSongObject(this->m_parentWidget->m_songInfo);
    if (this->m_songs.defaultPath == song.path) {
        this->updateParentSizeAndIDLabel(song, this->m_songID);
    } else {
        this->updateParentSizeAndIDLabel(song);
    }
}

void NongPopup::deleteSong(SongInfo const& song) {
    NongManager::get()->deleteNong(song, this->m_songID);
    this->updateParentWidget(NongManager::get()->getActiveNong(this->m_songID));
    FLAlertLayer::create("Success", "The song was deleted!", "Ok")->show();
    this->m_listLayer->removeAllChildrenWithCleanup(true);
    this->m_mainLayer->removeChild(m_listLayer);
    this->m_listLayer = nullptr;
    this->setSongs();
    this->createList();
}

void NongPopup::updateParentSizeAndIDLabel(SongInfo const& song, int songID) {
    auto label = typeinfo_cast<CCLabelBMFont*>(this->m_parentWidget->getChildByID("nongd-id-and-size-label"));
    if (!label) {
        return;
    }
    auto sizeText = NongManager::get()->getFormattedSize(song);
    std::string labelText;
    if (songID != 0) {
        labelText = "SongID: " + std::to_string(songID) + "  Size: " + sizeText;
    } else {
        labelText = "SongID: NONG  Size: " + sizeText;
    }
    if (label) {
        label->setString(labelText.c_str());
    }
}

void NongPopup::fetchSongHub(CCObject*) {
    createQuickPopup(
        "Fetch SFH", 
        "Do you want to fetch <cl>Song File Hub</c> content for <cy>" + std::to_string(this->m_songID) + "</c>?", 
        "No", "Yes",
        [this](auto, bool btn2) {
            if (btn2) {
                NongManager::get()->fetchSFH(this->m_songID, [this](nongd::FetchStatus result) {
                    this->onSFHFetched(result);
                });
            }
        }
    );
}

void NongPopup::onSFHFetched(nongd::FetchStatus result) {
    switch (result) {
        case nongd::FetchStatus::SUCCESS:
            FLAlertLayer::create("Success", "The Song File Hub data was fetched successfully!", "Ok")->show();
            this->m_listLayer->removeAllChildrenWithCleanup(true);
            this->m_mainLayer->removeChild(m_listLayer);
            this->m_listLayer = nullptr;
            this->setSongs();
            this->createList();
            break;
        case nongd::FetchStatus::NOTHING_FOUND:
            FLAlertLayer::create("Failed", "Found no data for this song!", "Ok")->show();
            break;
        case nongd::FetchStatus::FAILED:
            FLAlertLayer::create("Failed", "Failed to fetch data from Song File Hub!", "Ok")->show();
            break;
    }
}

int NongPopup::getSongID() {
    return this->m_songID;
}