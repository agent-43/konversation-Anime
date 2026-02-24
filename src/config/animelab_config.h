/*
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ANIMELAB_CONFIG_H
#define ANIMELAB_CONFIG_H

#include "settingspage.h"

#include <QWidget>

class QCheckBox;
class QComboBox;
class QPlainTextEdit;
class QPushButton;
class QTextBrowser;

class AnimeLab_Config : public QWidget, public KonviSettingsPage
{
    Q_OBJECT

    public:
        explicit AnimeLab_Config(QWidget* parent = nullptr, const char* name = nullptr);
        ~AnimeLab_Config() override;

        void restorePageToDefaults() override;
        void saveSettings() override;
        void loadSettings() override;
        bool hasChanged() override;

    Q_SIGNALS:
        void modified();

    private Q_SLOTS:
        void applyProfileNow();
        void exportThemeJson();
        void importThemeJson();
        void updatePreview();
        void markModified();

    private:
        QComboBox* m_profileCombo;
        QComboBox* m_animationCombo;
        QComboBox* m_soundPackCombo;
        QCheckBox* m_perNetworkThemes;
        QCheckBox* m_bubbleMode;
        QCheckBox* m_assistantSidebar;
        QCheckBox* m_stickerShortcuts;
        QCheckBox* m_livePreview;
        QCheckBox* m_streamerMode;
        QPlainTextEdit* m_networkThemeMap;
        QTextBrowser* m_preview;
        QPushButton* m_applyButton;
        QPushButton* m_exportButton;
        QPushButton* m_importButton;

        bool m_changed;

        void buildUi();
        void applyAnimeProfile(const QString& profileName);
        void installStickerShortcuts();
        void applyStreamerModeGuards(bool on);
};

#endif
