/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "animelab_config.h"

#include "application.h"
#include "preferences.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KConfigGroup>
#include <KSharedConfig>

#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

AnimeLab_Config::AnimeLab_Config(QWidget* parent, const char* name)
    : QWidget(parent)
    , m_changed(false)
{
    setObjectName(QString::fromLatin1(name ? name : "AnimeLab"));
    buildUi();
    loadSettings();
}

AnimeLab_Config::~AnimeLab_Config() = default;

void AnimeLab_Config::buildUi()
{
    auto* root = new QVBoxLayout(this);

    auto* modeGroup = new QGroupBox(i18n("Anime Experience"), this);
    auto* modeLayout = new QFormLayout(modeGroup);

    m_profileCombo = new QComboBox(modeGroup);
    m_profileCombo->addItems({QStringLiteral("Sakura"), QStringLiteral("Neon Night"), QStringLiteral("Ghibli Soft")});

    m_animationCombo = new QComboBox(modeGroup);
    m_animationCombo->addItems({QStringLiteral("None"), QStringLiteral("Subtle Gradient"), QStringLiteral("Parallax")});

    m_soundPackCombo = new QComboBox(modeGroup);
    m_soundPackCombo->addItems({QStringLiteral("Kawaii Ping"), QStringLiteral("Neon Arcade"), QStringLiteral("Soft Wind")});

    m_perNetworkThemes = new QCheckBox(i18n("Enable per-network themes"), modeGroup);
    m_bubbleMode = new QCheckBox(i18n("Enable message bubble mode"), modeGroup);
    m_assistantSidebar = new QCheckBox(i18n("Enable waifu assistant sidebar"), modeGroup);
    m_stickerShortcuts = new QCheckBox(i18n("Enable sticker/emote shortcuts"), modeGroup);
    m_livePreview = new QCheckBox(i18n("Enable live preview"), modeGroup);
    m_streamerMode = new QCheckBox(i18n("Enable streamer mode"), modeGroup);

    modeLayout->addRow(i18n("Profile"), m_profileCombo);
    modeLayout->addRow(i18n("Background Animation"), m_animationCombo);
    modeLayout->addRow(i18n("Sound Pack"), m_soundPackCombo);
    modeLayout->addRow(m_perNetworkThemes);
    modeLayout->addRow(m_bubbleMode);
    modeLayout->addRow(m_assistantSidebar);
    modeLayout->addRow(m_stickerShortcuts);
    modeLayout->addRow(m_livePreview);
    modeLayout->addRow(m_streamerMode);

    auto* mappingGroup = new QGroupBox(i18n("Per-Network Theme Mapping"), this);
    auto* mappingLayout = new QVBoxLayout(mappingGroup);
    mappingLayout->addWidget(new QLabel(i18n("Use one mapping per line: <network>=<profile>"), mappingGroup));
    m_networkThemeMap = new QPlainTextEdit(mappingGroup);
    m_networkThemeMap->setPlaceholderText(QStringLiteral("libera=Sakura\nwork=Neon Night"));
    mappingLayout->addWidget(m_networkThemeMap);

    auto* actionRow = new QHBoxLayout;
    m_applyButton = new QPushButton(i18n("Apply Profile Now"), this);
    m_exportButton = new QPushButton(i18n("Export Theme JSON"), this);
    m_importButton = new QPushButton(i18n("Import Theme JSON"), this);
    actionRow->addWidget(m_applyButton);
    actionRow->addWidget(m_exportButton);
    actionRow->addWidget(m_importButton);
    actionRow->addStretch();

    m_preview = new QTextBrowser(this);
    m_preview->setMinimumHeight(140);

    root->addWidget(modeGroup);
    root->addWidget(mappingGroup);
    root->addLayout(actionRow);
    root->addWidget(new QLabel(i18n("Live Preview"), this));
    root->addWidget(m_preview);
    root->addStretch();

    connect(m_profileCombo, &QComboBox::currentTextChanged, this, &AnimeLab_Config::markModified);
    connect(m_profileCombo, &QComboBox::currentTextChanged, this, &AnimeLab_Config::updatePreview);
    connect(m_animationCombo, &QComboBox::currentTextChanged, this, &AnimeLab_Config::markModified);
    connect(m_soundPackCombo, &QComboBox::currentTextChanged, this, &AnimeLab_Config::markModified);
    connect(m_perNetworkThemes, &QCheckBox::toggled, this, &AnimeLab_Config::markModified);
    connect(m_bubbleMode, &QCheckBox::toggled, this, &AnimeLab_Config::markModified);
    connect(m_assistantSidebar, &QCheckBox::toggled, this, &AnimeLab_Config::markModified);
    connect(m_stickerShortcuts, &QCheckBox::toggled, this, &AnimeLab_Config::markModified);
    connect(m_livePreview, &QCheckBox::toggled, this, &AnimeLab_Config::markModified);
    connect(m_livePreview, &QCheckBox::toggled, this, &AnimeLab_Config::updatePreview);
    connect(m_streamerMode, &QCheckBox::toggled, this, &AnimeLab_Config::markModified);
    connect(m_networkThemeMap, &QPlainTextEdit::textChanged, this, &AnimeLab_Config::markModified);
    connect(m_applyButton, &QPushButton::clicked, this, &AnimeLab_Config::applyProfileNow);
    connect(m_exportButton, &QPushButton::clicked, this, &AnimeLab_Config::exportThemeJson);
    connect(m_importButton, &QPushButton::clicked, this, &AnimeLab_Config::importThemeJson);
}

void AnimeLab_Config::restorePageToDefaults()
{
    m_profileCombo->setCurrentText(QStringLiteral("Sakura"));
    m_animationCombo->setCurrentText(QStringLiteral("Subtle Gradient"));
    m_soundPackCombo->setCurrentText(QStringLiteral("Kawaii Ping"));
    m_perNetworkThemes->setChecked(true);
    m_bubbleMode->setChecked(true);
    m_assistantSidebar->setChecked(true);
    m_stickerShortcuts->setChecked(true);
    m_livePreview->setChecked(true);
    m_streamerMode->setChecked(false);
    m_networkThemeMap->setPlainText(QStringLiteral("libera=Sakura"));
    updatePreview();
    markModified();
}

void AnimeLab_Config::saveSettings()
{
    KConfigGroup g(KSharedConfig::openConfig(), QStringLiteral("AnimeLab"));
    g.writeEntry("Profile", m_profileCombo->currentText());
    g.writeEntry("Animation", m_animationCombo->currentText());
    g.writeEntry("SoundPack", m_soundPackCombo->currentText());
    g.writeEntry("PerNetworkThemes", m_perNetworkThemes->isChecked());
    g.writeEntry("BubbleMode", m_bubbleMode->isChecked());
    g.writeEntry("AssistantSidebar", m_assistantSidebar->isChecked());
    g.writeEntry("StickerShortcuts", m_stickerShortcuts->isChecked());
    g.writeEntry("LivePreview", m_livePreview->isChecked());
    g.writeEntry("StreamerMode", m_streamerMode->isChecked());
    g.writeEntry("NetworkThemeMap", m_networkThemeMap->toPlainText());
    g.sync();

    if (m_stickerShortcuts->isChecked()) {
        installStickerShortcuts();
    }

    applyStreamerModeGuards(m_streamerMode->isChecked());
    m_changed = false;
}

void AnimeLab_Config::loadSettings()
{
    KConfigGroup g(KSharedConfig::openConfig(), QStringLiteral("AnimeLab"));
    m_profileCombo->setCurrentText(g.readEntry("Profile", QStringLiteral("Sakura")));
    m_animationCombo->setCurrentText(g.readEntry("Animation", QStringLiteral("Subtle Gradient")));
    m_soundPackCombo->setCurrentText(g.readEntry("SoundPack", QStringLiteral("Kawaii Ping")));
    m_perNetworkThemes->setChecked(g.readEntry("PerNetworkThemes", true));
    m_bubbleMode->setChecked(g.readEntry("BubbleMode", true));
    m_assistantSidebar->setChecked(g.readEntry("AssistantSidebar", true));
    m_stickerShortcuts->setChecked(g.readEntry("StickerShortcuts", true));
    m_livePreview->setChecked(g.readEntry("LivePreview", true));
    m_streamerMode->setChecked(g.readEntry("StreamerMode", false));
    m_networkThemeMap->setPlainText(g.readEntry("NetworkThemeMap", QStringLiteral("libera=Sakura")));
    updatePreview();
    m_changed = false;
}

bool AnimeLab_Config::hasChanged()
{
    return m_changed;
}

void AnimeLab_Config::markModified()
{
    m_changed = true;
    Q_EMIT modified();
}

void AnimeLab_Config::applyAnimeProfile(const QString& profileName)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup app(config, QStringLiteral("Appearance"));
    KConfigGroup msg(config, QStringLiteral("Message Text Colors"));
    KConfigGroup themes(config, QStringLiteral("Themes"));

    QString bg = QStringLiteral("#fff8fd");
    QString altBg = QStringLiteral("#f8ecff");
    QString channel = QStringLiteral("#2f244d");
    QString action = QStringLiteral("#ff5d8f");
    QString server = QStringLiteral("#8b3d6a");
    QString command = QStringLiteral("#4e3c7c");
    QString query = channel;
    QString time = QStringLiteral("#9f8bb6");
    QString backlink = QStringLiteral("#b8a6c6");
    QString hyperlink = QStringLiteral("#6d45c8");

    if (profileName == QLatin1String("Neon Night")) {
        bg = QStringLiteral("#1b132b");
        altBg = QStringLiteral("#24183a");
        channel = QStringLiteral("#efe8ff");
        action = QStringLiteral("#ff4fa6");
        server = QStringLiteral("#ff77c8");
        command = QStringLiteral("#9f7cff");
        query = QStringLiteral("#f3edff");
        time = QStringLiteral("#baa4e3");
        backlink = QStringLiteral("#7d6c99");
        hyperlink = QStringLiteral("#73d8ff");
    } else if (profileName == QLatin1String("Ghibli Soft")) {
        bg = QStringLiteral("#f7f6ec");
        altBg = QStringLiteral("#eef2df");
        channel = QStringLiteral("#2f3a31");
        action = QStringLiteral("#d06c8f");
        server = QStringLiteral("#7a8f63");
        command = QStringLiteral("#5a6e8e");
        query = QStringLiteral("#2f3a31");
        time = QStringLiteral("#8d8a73");
        backlink = QStringLiteral("#a5ad93");
        hyperlink = QStringLiteral("#4f74a6");
    }

    app.writeEntry("InputFieldsBackgroundColor", true);
    app.writeEntry("UseColoredNicks", true);
    app.writeEntry("AllowColorCodes", true);

    msg.writeEntry("TextViewBackground", bg);
    msg.writeEntry("AlternateBackground", altBg);
    msg.writeEntry("ChannelMessage", channel);
    msg.writeEntry("ActionMessage", action);
    msg.writeEntry("ServerMessage", server);
    msg.writeEntry("CommandMessage", command);
    msg.writeEntry("QueryMessage", query);
    msg.writeEntry("Time", time);
    msg.writeEntry("BacklogMessage", backlink);
    msg.writeEntry("Hyperlink", hyperlink);
    msg.writeEntry("Action", action);

    themes.writeEntry("IconTheme", QStringLiteral("anime-sakura"));

    config->sync();
    Preferences::self()->setIconTheme(QStringLiteral("anime-sakura"));
    Application::instance()->images()->initializeNickIcons();
    Q_EMIT Application::instance()->appearanceChanged();
}

void AnimeLab_Config::applyProfileNow()
{
    applyAnimeProfile(m_profileCombo->currentText());
}

void AnimeLab_Config::installStickerShortcuts()
{
    QStringList list = Preferences::quickButtonList();
    const QStringList stickerButtons = {
        QStringLiteral("Sakura,/say (✿◠‿◠)"),
        QStringLiteral("Cat,/say (=^･ω･^=)"),
        QStringLiteral("Sparkle,/say ✨"),
        QStringLiteral("Blush,/say (⁄ ⁄•⁄ω⁄•⁄ ⁄)"),
    };

    for (const QString& b : stickerButtons) {
        if (!list.contains(b)) {
            list.append(b);
        }
    }
    Preferences::setQuickButtonList(list);
}

void AnimeLab_Config::applyStreamerModeGuards(bool on)
{
    if (!on) {
        return;
    }

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup general(config, QStringLiteral("General Options"));
    KConfigGroup flags(config, QStringLiteral("Flags"));
    KConfigGroup appearance(config, QStringLiteral("Appearance"));

    general.writeEntry("ShowRealNames", false);
    flags.writeEntry("Log", false);
    flags.writeEntry("AddHostnameToLog", false);
    appearance.writeEntry("Timestamping", true);
    config->sync();
}

void AnimeLab_Config::updatePreview()
{
    if (!m_livePreview->isChecked()) {
        m_preview->setHtml(i18n("<b>Live preview disabled.</b>"));
        return;
    }

    QString bg = QStringLiteral("#fff8fd");
    QString fg = QStringLiteral("#2f244d");
    QString accent = QStringLiteral("#ff5d8f");

    if (m_profileCombo->currentText() == QLatin1String("Neon Night")) {
        bg = QStringLiteral("#1b132b");
        fg = QStringLiteral("#f3edff");
        accent = QStringLiteral("#ff4fa6");
    } else if (m_profileCombo->currentText() == QLatin1String("Ghibli Soft")) {
        bg = QStringLiteral("#f7f6ec");
        fg = QStringLiteral("#2f3a31");
        accent = QStringLiteral("#7a8f63");
    }

    const QString html = QStringLiteral(
        "<div style='background:%1;color:%2;padding:10px;border-radius:8px;'>"
        "<p><b style='color:%3'>[%4]</b> Konvi-chan: Theme looks great.</p>"
        "<p><b style='color:%3'>[%5]</b> You: /join #anime</p>"
        "<p><i>Animation: %6 | Bubble: %7 | Sound: %8</i></p>"
        "</div>")
        .arg(bg, fg, accent,
            QStringLiteral("19:30"),
            QStringLiteral("19:31"),
            m_animationCombo->currentText(),
            m_bubbleMode->isChecked() ? QStringLiteral("On") : QStringLiteral("Off"),
            m_soundPackCombo->currentText());
    m_preview->setHtml(html);
}

void AnimeLab_Config::exportThemeJson()
{
    const QString fileName = QFileDialog::getSaveFileName(this,
        i18n("Export Anime Theme"),
        QStringLiteral("anime-theme.json"),
        i18n("JSON Files (*.json)"));
    if (fileName.isEmpty()) {
        return;
    }

    QJsonObject obj;
    obj.insert(QStringLiteral("profile"), m_profileCombo->currentText());
    obj.insert(QStringLiteral("animation"), m_animationCombo->currentText());
    obj.insert(QStringLiteral("soundPack"), m_soundPackCombo->currentText());
    obj.insert(QStringLiteral("perNetworkThemes"), m_perNetworkThemes->isChecked());
    obj.insert(QStringLiteral("bubbleMode"), m_bubbleMode->isChecked());
    obj.insert(QStringLiteral("assistantSidebar"), m_assistantSidebar->isChecked());
    obj.insert(QStringLiteral("stickerShortcuts"), m_stickerShortcuts->isChecked());
    obj.insert(QStringLiteral("livePreview"), m_livePreview->isChecked());
    obj.insert(QStringLiteral("streamerMode"), m_streamerMode->isChecked());
    obj.insert(QStringLiteral("networkThemeMap"), m_networkThemeMap->toPlainText());

    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        KMessageBox::error(this, i18n("Failed to write file: %1", fileName));
        return;
    }
    f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
}

void AnimeLab_Config::importThemeJson()
{
    const QString fileName = QFileDialog::getOpenFileName(this,
        i18n("Import Anime Theme"),
        QString(),
        i18n("JSON Files (*.json)"));
    if (fileName.isEmpty()) {
        return;
    }

    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        KMessageBox::error(this, i18n("Failed to read file: %1", fileName));
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) {
        KMessageBox::error(this, i18n("Invalid JSON format."));
        return;
    }
    const QJsonObject obj = doc.object();

    m_profileCombo->setCurrentText(obj.value(QStringLiteral("profile")).toString(QStringLiteral("Sakura")));
    m_animationCombo->setCurrentText(obj.value(QStringLiteral("animation")).toString(QStringLiteral("Subtle Gradient")));
    m_soundPackCombo->setCurrentText(obj.value(QStringLiteral("soundPack")).toString(QStringLiteral("Kawaii Ping")));
    m_perNetworkThemes->setChecked(obj.value(QStringLiteral("perNetworkThemes")).toBool(true));
    m_bubbleMode->setChecked(obj.value(QStringLiteral("bubbleMode")).toBool(true));
    m_assistantSidebar->setChecked(obj.value(QStringLiteral("assistantSidebar")).toBool(true));
    m_stickerShortcuts->setChecked(obj.value(QStringLiteral("stickerShortcuts")).toBool(true));
    m_livePreview->setChecked(obj.value(QStringLiteral("livePreview")).toBool(true));
    m_streamerMode->setChecked(obj.value(QStringLiteral("streamerMode")).toBool(false));
    m_networkThemeMap->setPlainText(obj.value(QStringLiteral("networkThemeMap")).toString());
    updatePreview();
    markModified();
}

#include "moc_animelab_config.cpp"
