#include "ayu_settings.h"

#include "ayu/ui/ayu_logo.h"

#include "lang_auto.h"
#include "core/application.h"

#include "rpl/lifetime.h"
#include "rpl/producer.h"
#include "rpl/variable.h"

#include <fstream>

#include "ayu_worker.h"
#include "window/window_controller.h"

using json = nlohmann::json;

namespace Ayu {

const std::string filename = "tdata/ayu_settings.json";
const std::string themeFilename = "tdata/ayu_theme.json";

// Синглтон
AyuSettings* AyuSettings::_instance = nullptr;

// Переменные для реактивного обновления
rpl::variable<bool> sendReadMessagesReactive;
rpl::variable<bool> sendReadStoriesReactive;
rpl::variable<bool> sendOnlinePacketsReactive;
rpl::variable<bool> sendUploadProgressReactive;
rpl::variable<bool> sendOfflinePacketAfterOnlineReactive;

rpl::variable<bool> ghostModeEnabled;

rpl::variable<QString> deletedMarkReactive;
rpl::variable<QString> editedMarkReactive;

rpl::variable<int> showPeerIdReactive;

rpl::variable<bool> hideFromBlockedReactive;
rpl::event_stream<> historyUpdateReactive;

rpl::lifetime lifetime = rpl::lifetime();

// Проверка на включенный режим-призрак
bool ghostModeEnabled_util(const AyuSettings &settingsUtil) {
	return
		!settingsUtil.sendReadMessages
		&& !settingsUtil.sendReadStories
		&& !settingsUtil.sendOnlinePackets
		&& !settingsUtil.sendUploadProgress
		&& settingsUtil.sendOfflinePacketAfterOnline;
}

// Получение экземпляра синглтона
AyuSettings* AyuSettings::GetInstance() {
	if (!_instance) {
		_instance = new AyuSettings();
		_instance->Initialize();
	}
	return _instance;
}

AyuSettings::AyuSettings() {
	// ~ Ghost essentials
	sendReadMessages = true;
	sendReadStories = true;
	sendOnlinePackets = true;
	sendUploadProgress = true;
	sendOfflinePacketAfterOnline = false;

	markReadAfterAction = true;
	useScheduledMessages = false;
	sendWithoutSound = false;

	// ~ Message edits & deletion history
	saveDeletedMessages = true;
	saveMessagesHistory = true;

	saveForBots = false;

	// ~ Message filters
	hideFromBlocked = false;

	// ~ QoL toggles
	disableAds = true;
	disableStories = false;
	disableCustomBackgrounds = true;
	collapseSimilarChannels = true;
	hideSimilarChannels = false;

	wideMultiplier = 1.0;

	spoofWebviewAsAndroid = false;
	increaseWebviewHeight = false;
	increaseWebviewWidth = false;

	disableNotificationsDelay = false;
	localPremium = false;

	// ~ Customization
	appIcon =
#ifdef Q_OS_MAC
		AyuAssets::DEFAULT_MACOS_ICON
#else
		AyuAssets::DEFAULT_ICON
#endif
	;
	simpleQuotesAndReplies = true;
	replaceBottomInfoWithIcons = true;
	deletedMark = "🧹";
	editedMark = Core::IsAppLaunched() ? tr::lng_edited(tr::now) : QString("edited");
	recentStickersCount = 100;

	// context menu items
	// 0 - hide
	// 1 - show normally
	// 2 - show with SHIFT or CTRL pressed
	showReactionsPanelInContextMenu = 1;
	showViewsPanelInContextMenu = 1;
	showHideMessageInContextMenu = 0;
	showUserMessagesInContextMenu = 2;
	showMessageDetailsInContextMenu = 2;

	showAttachButtonInMessageField = true;
	showCommandsButtonInMessageField = true;
	showEmojiButtonInMessageField = true;
	showMicrophoneButtonInMessageField = true;
	showAutoDeleteButtonInMessageField = true;

	showAttachPopup = true;
	showEmojiPopup = true;

	showLReadToggleInDrawer = false;
	showSReadToggleInDrawer = true;
	showGhostToggleInDrawer = true;
	showStreamerToggleInDrawer = false;

	showGhostToggleInTray = true;
	showStreamerToggleInTray = false;

	monoFont = "";

	hideNotificationCounters = false;
	hideNotificationBadge = false;
	hideAllChatsFolder = false;

	/*
		* channelBottomButton = 0 means "Hide"
		* channelBottomButton = 1 means "Mute"/"Unmute"
		* channelBottomButton = 2 means "Discuss" + fallback to "Mute"/"Unmute"
	*/
	channelBottomButton = 2;

	/*
		* showPeerId = 0 means no ID shown
		* showPeerId = 1 means ID shown as for Telegram API devs
		* showPeerId = 2 means ID shown as for Bot API devs (-100)
	*/
	showPeerId = 2;
	showMessageSeconds = false;
	showMessageShot = true;

	// ~ Confirmations
	stickerConfirmation = false;
	gifConfirmation = false;
	voiceConfirmation = false;
}

AyuSettings::~AyuSettings() {
	// Сохраняем настройки при уничтожении синглтона
	Save();
}

void AyuSettings::Initialize() {
	// Инициализация реактивных связей
	sendReadMessagesReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val != sendReadMessages);
		}) | rpl::start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(*this);
		},
		lifetime);
	
	sendReadStoriesReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val != sendReadStories);
		}) | rpl::start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(*this);
		},
		lifetime);
	
	sendOnlinePacketsReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val != sendOnlinePackets);
		}) | rpl::start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(*this);
		},
		lifetime);
	
	sendUploadProgressReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val != sendUploadProgress);
		}) | rpl::start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(*this);
		},
		lifetime);
	
	sendOfflinePacketAfterOnlineReactive.value() | rpl::filter(
		[=](bool val)
		{
			return (val != sendOfflinePacketAfterOnline);
		}) | rpl::start_with_next(
		[=](bool val)
		{
			ghostModeEnabled =
				ghostModeEnabled_util(*this);
		},
		lifetime);
	
	// Инициализация реактивных переменных текущими значениями
	sendReadMessagesReactive = sendReadMessages;
	sendReadStoriesReactive = sendReadStories;
	sendUploadProgressReactive = sendUploadProgress;
	sendOfflinePacketAfterOnlineReactive = sendOfflinePacketAfterOnline;
	sendOnlinePacketsReactive = sendOnlinePackets;

	deletedMarkReactive = deletedMark;
	editedMarkReactive = editedMark;
	showPeerIdReactive = showPeerId;

	hideFromBlockedReactive = hideFromBlocked;

	ghostModeEnabled = ghostModeEnabled_util(*this);
}

void AyuSettings::Load() {
	// Загрузка основных настроек
	std::ifstream file(filename);
	if (file.good()) {
		try {
			json p;
			file >> p;
			file.close();

			try {
				// Десериализуем себя из JSON
				*this = p.get<AyuSettings>();
			} catch (...) {
				LOG(("AyuGramSettings: failed to parse settings file"));
			}
		} catch (...) {
			LOG(("AyuGramSettings: failed to read settings file (not json-like)"));
		}
	}

	if (cGhost()) {
		sendReadMessages = false;
		sendReadStories = false;
		sendOnlinePackets = false;
		sendUploadProgress = false;
		sendOfflinePacketAfterOnline = true;
	}
	
	// Загружаем настройки темы
	std::ifstream themeFile(themeFilename);
	if (themeFile.good()) {
		try {
			json p;
			themeFile >> p;
			themeFile.close();
			
			// Попытка десериализации настроек темы
			try {
				p.get_to(_themeSettings);
			} catch (...) {
				LOG(("AyuGramSettings: failed to parse theme settings file"));
			}
		} catch (...) {
			LOG(("AyuGramSettings: failed to read theme settings file (not json-like)"));
		}
	}
	
	// Обновляем реактивные переменные
	Initialize();
}

void AyuSettings::Save() {
	// Сохранение основных настроек
	json p = *this;
	std::ofstream file;
	file.open(filename);
	file << p.dump(4);
	file.close();
	
	// Сохранение настроек темы
	json themeJson = _themeSettings;
	std::ofstream themeFile;
	themeFile.open(themeFilename);
	themeFile << themeJson.dump(4);
	themeFile.close();
}

const ThemeSettings& AyuSettings::GetThemeSettings() const {
	return _themeSettings;
}

void AyuSettings::SetThemeSettings(const ThemeSettings& settings) {
	if (_themeSettings != settings) {
		_themeSettings = settings;
		
		// Сохраняем изменения и уведомляем об изменении темы
		Save();
		notifyThemeChanged();
	}
}

void AyuSettings::notifyThemeChanged() {
	_themeChanged.fire({});
}

rpl::producer<> AyuSettings::themeChanged() const {
	return _themeChanged.events();
}

void AyuSettings::set_sendReadMessages(bool val) {
	sendReadMessages = val;
	sendReadMessagesReactive = val;
}

void AyuSettings::set_sendReadStories(bool val) {
	sendReadStories = val;
	sendReadStoriesReactive = val;
}

void AyuSettings::set_sendOnlinePackets(bool val) {
	sendOnlinePackets = val;
	sendOnlinePacketsReactive = val;
}

void AyuSettings::set_sendUploadProgress(bool val) {
	sendUploadProgress = val;
	sendUploadProgressReactive = val;
}

void AyuSettings::set_sendOfflinePacketAfterOnline(bool val) {
	sendOfflinePacketAfterOnline = val;
	sendOfflinePacketAfterOnlineReactive = val;
}

void AyuSettings::set_ghostModeEnabled(bool val) {
	set_sendReadMessages(!val);
	set_sendReadStories(!val);
	set_sendOnlinePackets(!val);
	set_sendUploadProgress(!val);
	set_sendOfflinePacketAfterOnline(val);

	if (const auto window = Core::App().activeWindow()) {
		if (const auto session = window->maybeSession()) {
			AyuWorker::markAsOnline(session); // mark as online to get offline instantly
		}
	}
}

void AyuSettings::set_markReadAfterAction(bool val) {
	markReadAfterAction = val;
}

void AyuSettings::set_useScheduledMessages(bool val) {
	useScheduledMessages = val;
}

void AyuSettings::set_sendWithoutSound(bool val) {
	sendWithoutSound = val;
}

void AyuSettings::set_saveDeletedMessages(bool val) {
	saveDeletedMessages = val;
}

void AyuSettings::set_saveMessagesHistory(bool val) {
	saveMessagesHistory = val;
}

void AyuSettings::set_saveForBots(bool val) {
	saveForBots = val;
}

void AyuSettings::set_hideFromBlocked(bool val) {
	hideFromBlocked = val;
	hideFromBlockedReactive = val;
}

void AyuSettings::set_disableAds(bool val) {
	disableAds = val;
}

void AyuSettings::set_disableStories(bool val) {
	disableStories = val;
}

void AyuSettings::set_disableCustomBackgrounds(bool val) {
	disableCustomBackgrounds = val;
}

void AyuSettings::set_collapseSimilarChannels(bool val) {
	collapseSimilarChannels = val;
}

void AyuSettings::set_hideSimilarChannels(bool val) {
	hideSimilarChannels = val;
}

void AyuSettings::set_wideMultiplier(double val) {
	wideMultiplier = val;
}

void AyuSettings::set_spoofWebviewAsAndroid(bool val) {
	spoofWebviewAsAndroid = val;
}

void AyuSettings::set_increaseWebviewHeight(bool val) {
	increaseWebviewHeight = val;
}

void AyuSettings::set_increaseWebviewWidth(bool val) {
	increaseWebviewWidth = val;
}

void AyuSettings::set_disableNotificationsDelay(bool val) {
	disableNotificationsDelay = val;
}

void AyuSettings::set_localPremium(bool val) {
	localPremium = val;
}

void AyuSettings::set_appIcon(QString val) {
	appIcon = std::move(val);
}

void AyuSettings::set_simpleQuotesAndReplies(bool val) {
	simpleQuotesAndReplies = val;
}

void AyuSettings::set_replaceBottomInfoWithIcons(bool val) {
	replaceBottomInfoWithIcons = val;
}

void AyuSettings::set_deletedMark(QString val) {
	deletedMark = std::move(val);
	deletedMarkReactive = deletedMark;
}

void AyuSettings::set_editedMark(QString val) {
	editedMark = std::move(val);
	editedMarkReactive = editedMark;
}

void AyuSettings::set_recentStickersCount(int val) {
	recentStickersCount = val;
}

void AyuSettings::set_showReactionsPanelInContextMenu(int val) {
	showReactionsPanelInContextMenu = val;
}

void AyuSettings::set_showViewsPanelInContextMenu(int val) {
	showViewsPanelInContextMenu = val;
}

void AyuSettings::set_showHideMessageInContextMenu(int val) {
	showHideMessageInContextMenu = val;
}

void AyuSettings::set_showUserMessagesInContextMenu(int val) {
	showUserMessagesInContextMenu = val;
}

void AyuSettings::set_showMessageDetailsInContextMenu(int val) {
	showMessageDetailsInContextMenu = val;
}

void AyuSettings::set_showAttachButtonInMessageField(bool val) {
	showAttachButtonInMessageField = val;
	triggerHistoryUpdate();
}

void AyuSettings::set_showCommandsButtonInMessageField(bool val) {
	showCommandsButtonInMessageField = val;
	triggerHistoryUpdate();
}

void AyuSettings::set_showEmojiButtonInMessageField(bool val) {
	showEmojiButtonInMessageField = val;
	triggerHistoryUpdate();
}

void AyuSettings::set_showMicrophoneButtonInMessageField(bool val) {
	showMicrophoneButtonInMessageField = val;
	triggerHistoryUpdate();
}

void AyuSettings::set_showAutoDeleteButtonInMessageField(bool val) {
	showAutoDeleteButtonInMessageField = val;
	triggerHistoryUpdate();
}

void AyuSettings::set_showAttachPopup(bool val) {
	showAttachPopup = val;
	triggerHistoryUpdate();
}

void AyuSettings::set_showEmojiPopup(bool val) {
	showEmojiPopup = val;
	triggerHistoryUpdate();
}

void AyuSettings::set_showLReadToggleInDrawer(bool val) {
	showLReadToggleInDrawer = val;
}

void AyuSettings::set_showSReadToggleInDrawer(bool val) {
	showSReadToggleInDrawer = val;
}

void AyuSettings::set_showGhostToggleInDrawer(bool val) {
	showGhostToggleInDrawer = val;
}

void AyuSettings::set_showStreamerToggleInDrawer(bool val) {
	showStreamerToggleInDrawer = val;
}

void AyuSettings::set_showGhostToggleInTray(bool val) {
	showGhostToggleInTray = val;
}

void AyuSettings::set_showStreamerToggleInTray(bool val) {
	showStreamerToggleInTray = val;
}

void AyuSettings::set_monoFont(QString val) {
	monoFont = val;
}

void AyuSettings::set_showPeerId(int val) {
	showPeerId = val;
	showPeerIdReactive = val;
}

void AyuSettings::set_hideNotificationCounters(bool val) {
	hideNotificationCounters = val;
}

void AyuSettings::set_hideNotificationBadge(bool val) {
	hideNotificationBadge = val;
}

void AyuSettings::set_hideAllChatsFolder(bool val) {
	hideAllChatsFolder = val;
}

void AyuSettings::set_channelBottomButton(int val) {
	channelBottomButton = val;
}

void AyuSettings::set_showMessageSeconds(bool val) {
	showMessageSeconds = val;
}

void AyuSettings::set_showMessageShot(bool val) {
	showMessageShot = val;
}

void AyuSettings::set_stickerConfirmation(bool val) {
	stickerConfirmation = val;
}

void AyuSettings::set_gifConfirmation(bool val) {
	gifConfirmation = val;
}

void AyuSettings::set_voiceConfirmation(bool val) {
	voiceConfirmation = val;
}

// Глобальные утилиты для совместимости с существующим кодом
bool isUseScheduledMessages() {
	const auto settings = AyuSettings::GetInstance();
	return isGhostModeActive() && settings->useScheduledMessages;
}

bool isGhostModeActive() {
	return ghostModeEnabled.current();
}

rpl::producer<QString> get_deletedMarkReactive() {
	return deletedMarkReactive.value();
}

rpl::producer<QString> get_editedMarkReactive() {
	return editedMarkReactive.value();
}

rpl::producer<int> get_showPeerIdReactive() {
	return showPeerIdReactive.value();
}

rpl::producer<bool> get_ghostModeEnabledReactive() {
	return ghostModeEnabled.value();
}

rpl::producer<bool> get_hideFromBlockedReactive() {
	return hideFromBlockedReactive.value();
}

void triggerHistoryUpdate() {
	historyUpdateReactive.fire({});
}

rpl::producer<> get_historyUpdateReactive() {
	return historyUpdateReactive.events();
}

} // namespace Ayu