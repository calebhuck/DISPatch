#include <DISPatch/Theme.h>

#include <QtGui/QColor>

namespace dispatch {

auto themePalette(Theme theme) -> QPalette
{
    // NOLINTBEGIN(readability-magic-numbers) RGB theme constants are clearer inline.
    QPalette palette;
    if (theme == Theme::Dark) {
        palette.setColor(QPalette::Window, QColor(28, 31, 36));
        palette.setColor(QPalette::WindowText, QColor(232, 236, 241));
        palette.setColor(QPalette::Base, QColor(20, 23, 27));
        palette.setColor(QPalette::AlternateBase, QColor(33, 38, 45));
        palette.setColor(QPalette::ToolTipBase, QColor(232, 236, 241));
        palette.setColor(QPalette::ToolTipText, QColor(20, 23, 27));
        palette.setColor(QPalette::Text, QColor(232, 236, 241));
        palette.setColor(QPalette::Button, QColor(39, 44, 52));
        palette.setColor(QPalette::ButtonText, QColor(232, 236, 241));
        palette.setColor(QPalette::BrightText, QColor(255, 255, 255));
        palette.setColor(QPalette::Highlight, QColor(43, 130, 190));
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(134, 142, 150));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(134, 142, 150));
        return palette;
    }

    if (theme == Theme::OneDark) {
        palette.setColor(QPalette::Window, QColor(40, 44, 52));
        palette.setColor(QPalette::WindowText, QColor(171, 178, 191));
        palette.setColor(QPalette::Base, QColor(33, 37, 43));
        palette.setColor(QPalette::AlternateBase, QColor(44, 49, 58));
        palette.setColor(QPalette::ToolTipBase, QColor(171, 178, 191));
        palette.setColor(QPalette::ToolTipText, QColor(33, 37, 43));
        palette.setColor(QPalette::Text, QColor(171, 178, 191));
        palette.setColor(QPalette::Button, QColor(58, 63, 75));
        palette.setColor(QPalette::ButtonText, QColor(220, 223, 228));
        palette.setColor(QPalette::BrightText, QColor(224, 108, 117));
        palette.setColor(QPalette::Highlight, QColor(97, 175, 239));
        palette.setColor(QPalette::HighlightedText, QColor(24, 26, 31));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(92, 99, 112));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(92, 99, 112));
        return palette;
    }

    if (theme == Theme::VsCodeDefault) {
        palette.setColor(QPalette::Window, QColor(24, 24, 24));
        palette.setColor(QPalette::WindowText, QColor(204, 204, 204));
        palette.setColor(QPalette::Base, QColor(31, 31, 31));
        palette.setColor(QPalette::AlternateBase, QColor(43, 43, 43));
        palette.setColor(QPalette::ToolTipBase, QColor(204, 204, 204));
        palette.setColor(QPalette::ToolTipText, QColor(31, 31, 31));
        palette.setColor(QPalette::Text, QColor(204, 204, 204));
        palette.setColor(QPalette::Button, QColor(49, 49, 49));
        palette.setColor(QPalette::ButtonText, QColor(204, 204, 204));
        palette.setColor(QPalette::BrightText, QColor(248, 81, 73));
        palette.setColor(QPalette::Highlight, QColor(0, 120, 212));
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(134, 134, 134));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(134, 134, 134));
        return palette;
    }

    if (theme == Theme::TokyoNight) {
        palette.setColor(QPalette::Window, QColor(26, 27, 38));
        palette.setColor(QPalette::WindowText, QColor(192, 202, 245));
        palette.setColor(QPalette::Base, QColor(22, 22, 30));
        palette.setColor(QPalette::AlternateBase, QColor(36, 40, 59));
        palette.setColor(QPalette::ToolTipBase, QColor(192, 202, 245));
        palette.setColor(QPalette::ToolTipText, QColor(22, 22, 30));
        palette.setColor(QPalette::Text, QColor(192, 202, 245));
        palette.setColor(QPalette::Button, QColor(41, 46, 66));
        palette.setColor(QPalette::ButtonText, QColor(192, 202, 245));
        palette.setColor(QPalette::BrightText, QColor(247, 118, 142));
        palette.setColor(QPalette::Highlight, QColor(122, 162, 247));
        palette.setColor(QPalette::HighlightedText, QColor(26, 27, 38));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(86, 95, 137));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(86, 95, 137));
        return palette;
    }

    if (theme == Theme::Dracula) {
        palette.setColor(QPalette::Window, QColor(40, 42, 54));
        palette.setColor(QPalette::WindowText, QColor(248, 248, 242));
        palette.setColor(QPalette::Base, QColor(33, 34, 44));
        palette.setColor(QPalette::AlternateBase, QColor(68, 71, 90));
        palette.setColor(QPalette::ToolTipBase, QColor(248, 248, 242));
        palette.setColor(QPalette::ToolTipText, QColor(40, 42, 54));
        palette.setColor(QPalette::Text, QColor(248, 248, 242));
        palette.setColor(QPalette::Button, QColor(52, 55, 70));
        palette.setColor(QPalette::ButtonText, QColor(248, 248, 242));
        palette.setColor(QPalette::BrightText, QColor(255, 85, 85));
        palette.setColor(QPalette::Highlight, QColor(189, 147, 249));
        palette.setColor(QPalette::HighlightedText, QColor(40, 42, 54));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(98, 114, 164));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(98, 114, 164));
        return palette;
    }

    if (theme == Theme::Gruvbox) {
        palette.setColor(QPalette::Window, QColor(40, 40, 40));
        palette.setColor(QPalette::WindowText, QColor(235, 219, 178));
        palette.setColor(QPalette::Base, QColor(29, 32, 33));
        palette.setColor(QPalette::AlternateBase, QColor(50, 48, 47));
        palette.setColor(QPalette::ToolTipBase, QColor(235, 219, 178));
        palette.setColor(QPalette::ToolTipText, QColor(29, 32, 33));
        palette.setColor(QPalette::Text, QColor(235, 219, 178));
        palette.setColor(QPalette::Button, QColor(60, 56, 54));
        palette.setColor(QPalette::ButtonText, QColor(235, 219, 178));
        palette.setColor(QPalette::BrightText, QColor(251, 241, 199));
        palette.setColor(QPalette::Highlight, QColor(215, 153, 33));
        palette.setColor(QPalette::HighlightedText, QColor(40, 40, 40));
        palette.setColor(QPalette::Disabled, QPalette::Text, QColor(146, 131, 116));
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(146, 131, 116));
        return palette;
    }

    palette.setColor(QPalette::Window, QColor(244, 246, 248));
    palette.setColor(QPalette::WindowText, QColor(31, 35, 40));
    palette.setColor(QPalette::Base, QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase, QColor(235, 239, 244));
    palette.setColor(QPalette::ToolTipBase, QColor(31, 35, 40));
    palette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
    palette.setColor(QPalette::Text, QColor(31, 35, 40));
    palette.setColor(QPalette::Button, QColor(250, 251, 252));
    palette.setColor(QPalette::ButtonText, QColor(31, 35, 40));
    palette.setColor(QPalette::BrightText, QColor(191, 38, 38));
    palette.setColor(QPalette::Highlight, QColor(0, 105, 170));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    return palette;
    // NOLINTEND(readability-magic-numbers)
}

auto themeStyleSheet(Theme theme) -> QString
{
    if (theme == Theme::Dark) {
        return QStringLiteral(R"(
            QWidget { font-size: 10pt; }
            QMainWindow, QWidget { background: #1c1f24; color: #e8ecf1; }
            QGroupBox {
                border: 1px solid #3a414c;
                border-radius: 6px;
                margin-top: 16px;
                padding: 14px 12px 12px 12px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 6px;
                color: #f4f7fb;
            }
            QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QTableWidget {
                background: #14171b;
                border: 1px solid #3a414c;
                border-radius: 4px;
                color: #e8ecf1;
                selection-background-color: #2b82be;
            }
            QLineEdit, QSpinBox, QComboBox { min-height: 26px; padding: 2px 6px; }
            QCheckBox { spacing: 8px; }
            QCheckBox::indicator {
                width: 15px;
                height: 15px;
                background: #14171b;
                border: 2px solid #8a96a8;
                border-radius: 3px;
            }
            QCheckBox::indicator:checked {
                background: #2b82be;
                border-color: #89caf4;
                image: url(:/dispatch/check-white.xpm);
            }
            QCheckBox::indicator:disabled {
                background: #272c34;
                border-color: #4a5362;
            }
            QPushButton {
                background: #2b82be;
                border: 1px solid #3b91cc;
                border-radius: 4px;
                color: white;
                font-weight: 600;
                padding: 7px 14px;
            }
            QPushButton:hover { background: #3393d5; }
            QPushButton:pressed { background: #1f6fa6; }
            QHeaderView::section {
                background: #272c34;
                border: 0;
                border-right: 1px solid #3a414c;
                border-bottom: 1px solid #3a414c;
                color: #e8ecf1;
                font-weight: 600;
                padding: 6px;
            }
            QTableWidget::item { padding: 4px; }
            QStatusBar { border-top: 1px solid #3a414c; }
        )");
    }

    if (theme == Theme::OneDark) {
        return QStringLiteral(R"(
            QWidget { font-size: 10pt; }
            QMainWindow, QWidget { background: #282c34; color: #abb2bf; }
            QGroupBox {
                border: 1px solid #3e4451;
                border-radius: 6px;
                margin-top: 16px;
                padding: 14px 12px 12px 12px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 6px;
                color: #dcdfe4;
            }
            QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QTableWidget {
                background: #21252b;
                border: 1px solid #3e4451;
                border-radius: 4px;
                color: #abb2bf;
                selection-background-color: #61afef;
                selection-color: #181a1f;
            }
            QLineEdit, QSpinBox, QComboBox { min-height: 26px; padding: 2px 6px; }
            QCheckBox { spacing: 8px; }
            QCheckBox::indicator {
                width: 15px;
                height: 15px;
                background: #21252b;
                border: 2px solid #828997;
                border-radius: 3px;
            }
            QCheckBox::indicator:checked {
                background: #61afef;
                border-color: #98c379;
                image: url(:/dispatch/check-dark.xpm);
            }
            QCheckBox::indicator:disabled {
                background: #3a3f4b;
                border-color: #5c6370;
            }
            QPushButton {
                background: #4078b7;
                border: 1px solid #61afef;
                border-radius: 4px;
                color: #f7f9fb;
                font-weight: 600;
                padding: 7px 14px;
            }
            QPushButton:hover { background: #4b89c8; }
            QPushButton:pressed { background: #315f91; }
            QHeaderView::section {
                background: #2c313a;
                border: 0;
                border-right: 1px solid #3e4451;
                border-bottom: 1px solid #3e4451;
                color: #dcdfe4;
                font-weight: 600;
                padding: 6px;
            }
            QTableWidget::item { padding: 4px; }
            QStatusBar { border-top: 1px solid #3e4451; }
        )");
    }

    if (theme == Theme::VsCodeDefault) {
        return QStringLiteral(R"(
            QWidget { font-size: 10pt; }
            QMainWindow, QWidget { background: #181818; color: #cccccc; }
            QGroupBox {
                border: 1px solid #2b2b2b;
                border-radius: 6px;
                margin-top: 16px;
                padding: 14px 12px 12px 12px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 6px;
                color: #ffffff;
            }
            QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QTableWidget {
                background: #1f1f1f;
                border: 1px solid #3c3c3c;
                border-radius: 4px;
                color: #cccccc;
                selection-background-color: #0078d4;
                selection-color: #ffffff;
            }
            QLineEdit, QSpinBox, QComboBox { min-height: 26px; padding: 2px 6px; }
            QCheckBox { spacing: 8px; }
            QCheckBox::indicator {
                width: 15px;
                height: 15px;
                background: #313131;
                border: 2px solid #3c3c3c;
                border-radius: 3px;
            }
            QCheckBox::indicator:checked {
                background: #0078d4;
                border-color: #4daafc;
                image: url(:/dispatch/check-white.xpm);
            }
            QCheckBox::indicator:disabled {
                background: #2b2b2b;
                border-color: #616161;
            }
            QPushButton {
                background: #0078d4;
                border: 1px solid #2488db;
                border-radius: 4px;
                color: #ffffff;
                font-weight: 600;
                padding: 7px 14px;
            }
            QPushButton:hover { background: #026ec1; }
            QPushButton:pressed { background: #005a9e; }
            QHeaderView::section {
                background: #2b2b2b;
                border: 0;
                border-right: 1px solid #3c3c3c;
                border-bottom: 1px solid #3c3c3c;
                color: #cccccc;
                font-weight: 600;
                padding: 6px;
            }
            QTableWidget::item { padding: 4px; }
            QStatusBar { border-top: 1px solid #2b2b2b; }
        )");
    }

    if (theme == Theme::TokyoNight) {
        return QStringLiteral(R"(
            QWidget { font-size: 10pt; }
            QMainWindow, QWidget { background: #1a1b26; color: #c0caf5; }
            QGroupBox {
                border: 1px solid #3b4261;
                border-radius: 6px;
                margin-top: 16px;
                padding: 14px 12px 12px 12px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 6px;
                color: #c0caf5;
            }
            QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QTableWidget {
                background: #16161e;
                border: 1px solid #3b4261;
                border-radius: 4px;
                color: #c0caf5;
                selection-background-color: #7aa2f7;
                selection-color: #1a1b26;
            }
            QLineEdit, QSpinBox, QComboBox { min-height: 26px; padding: 2px 6px; }
            QCheckBox { spacing: 8px; }
            QCheckBox::indicator {
                width: 15px;
                height: 15px;
                background: #16161e;
                border: 2px solid #565f89;
                border-radius: 3px;
            }
            QCheckBox::indicator:checked {
                background: #7aa2f7;
                border-color: #9ece6a;
                image: url(:/dispatch/check-dark.xpm);
            }
            QCheckBox::indicator:disabled {
                background: #24283b;
                border-color: #414868;
            }
            QPushButton {
                background: #3d59a1;
                border: 1px solid #7aa2f7;
                border-radius: 4px;
                color: #ffffff;
                font-weight: 600;
                padding: 7px 14px;
            }
            QPushButton:hover { background: #4d6fc9; }
            QPushButton:pressed { background: #2f477f; }
            QHeaderView::section {
                background: #24283b;
                border: 0;
                border-right: 1px solid #3b4261;
                border-bottom: 1px solid #3b4261;
                color: #c0caf5;
                font-weight: 600;
                padding: 6px;
            }
            QTableWidget::item { padding: 4px; }
            QStatusBar { border-top: 1px solid #3b4261; }
        )");
    }

    if (theme == Theme::Dracula) {
        return QStringLiteral(R"(
            QWidget { font-size: 10pt; }
            QMainWindow, QWidget { background: #282a36; color: #f8f8f2; }
            QGroupBox {
                border: 1px solid #44475a;
                border-radius: 6px;
                margin-top: 16px;
                padding: 14px 12px 12px 12px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 6px;
                color: #f8f8f2;
            }
            QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QTableWidget {
                background: #21222c;
                border: 1px solid #191a21;
                border-radius: 4px;
                color: #f8f8f2;
                selection-background-color: #bd93f9;
                selection-color: #282a36;
            }
            QLineEdit, QSpinBox, QComboBox { min-height: 26px; padding: 2px 6px; }
            QCheckBox { spacing: 8px; }
            QCheckBox::indicator {
                width: 15px;
                height: 15px;
                background: #21222c;
                border: 2px solid #6272a4;
                border-radius: 3px;
            }
            QCheckBox::indicator:checked {
                background: #bd93f9;
                border-color: #ff79c6;
                image: url(:/dispatch/check-dark.xpm);
            }
            QCheckBox::indicator:disabled {
                background: #343746;
                border-color: #44475a;
            }
            QPushButton {
                background: #bd93f9;
                border: 1px solid #ff79c6;
                border-radius: 4px;
                color: #282a36;
                font-weight: 600;
                padding: 7px 14px;
            }
            QPushButton:hover { background: #d6acff; }
            QPushButton:pressed { background: #9b72d0; }
            QHeaderView::section {
                background: #343746;
                border: 0;
                border-right: 1px solid #44475a;
                border-bottom: 1px solid #44475a;
                color: #f8f8f2;
                font-weight: 600;
                padding: 6px;
            }
            QTableWidget::item { padding: 4px; }
            QStatusBar { border-top: 1px solid #44475a; }
        )");
    }

    if (theme == Theme::Gruvbox) {
        return QStringLiteral(R"(
            QWidget { font-size: 10pt; }
            QMainWindow, QWidget { background: #282828; color: #ebdbb2; }
            QGroupBox {
                border: 1px solid #665c54;
                border-radius: 6px;
                margin-top: 16px;
                padding: 14px 12px 12px 12px;
                font-weight: 600;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 6px;
                color: #fbf1c7;
            }
            QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QTableWidget {
                background: #1d2021;
                border: 1px solid #665c54;
                border-radius: 4px;
                color: #ebdbb2;
                selection-background-color: #d79921;
                selection-color: #282828;
            }
            QLineEdit, QSpinBox, QComboBox { min-height: 26px; padding: 2px 6px; }
            QCheckBox { spacing: 8px; }
            QCheckBox::indicator {
                width: 15px;
                height: 15px;
                background: #1d2021;
                border: 2px solid #d5c4a1;
                border-radius: 3px;
            }
            QCheckBox::indicator:checked {
                background: #d79921;
                border-color: #fabd2f;
                image: url(:/dispatch/check-dark.xpm);
            }
            QCheckBox::indicator:disabled {
                background: #3c3836;
                border-color: #665c54;
            }
            QPushButton {
                background: #458588;
                border: 1px solid #83a598;
                border-radius: 4px;
                color: #fbf1c7;
                font-weight: 600;
                padding: 7px 14px;
            }
            QPushButton:hover { background: #689d6a; }
            QPushButton:pressed { background: #3c3836; }
            QHeaderView::section {
                background: #3c3836;
                border: 0;
                border-right: 1px solid #665c54;
                border-bottom: 1px solid #665c54;
                color: #fbf1c7;
                font-weight: 600;
                padding: 6px;
            }
            QTableWidget::item { padding: 4px; }
            QStatusBar { border-top: 1px solid #665c54; }
        )");
    }

    return QStringLiteral(R"(
        QWidget { font-size: 10pt; }
        QMainWindow, QWidget { background: #f4f6f8; color: #1f2328; }
        QGroupBox {
            background: #ffffff;
            border: 1px solid #d0d7de;
            border-radius: 6px;
            margin-top: 16px;
            padding: 14px 12px 12px 12px;
            font-weight: 600;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px;
            color: #1f2328;
        }
        QLineEdit, QSpinBox, QComboBox, QPlainTextEdit, QTableWidget {
            background: #ffffff;
            border: 1px solid #c9d1d9;
            border-radius: 4px;
            color: #1f2328;
            selection-background-color: #0069aa;
        }
        QLineEdit, QSpinBox, QComboBox { min-height: 26px; padding: 2px 6px; }
        QCheckBox { spacing: 8px; }
        QCheckBox::indicator {
            width: 15px;
            height: 15px;
            background: #ffffff;
            border: 2px solid #6e7781;
            border-radius: 3px;
        }
        QCheckBox::indicator:checked {
            background: #0069aa;
            border-color: #005d97;
            image: url(:/dispatch/check-white.xpm);
        }
        QCheckBox::indicator:disabled {
            background: #e9edf2;
            border-color: #afb8c1;
        }
        QPushButton {
            background: #0069aa;
            border: 1px solid #005d97;
            border-radius: 4px;
            color: white;
            font-weight: 600;
            padding: 7px 14px;
        }
        QPushButton:hover { background: #0878bd; }
        QPushButton:pressed { background: #005489; }
        QHeaderView::section {
            background: #e9edf2;
            border: 0;
            border-right: 1px solid #d0d7de;
            border-bottom: 1px solid #d0d7de;
            color: #1f2328;
            font-weight: 600;
            padding: 6px;
        }
        QTableWidget::item { padding: 4px; }
        QStatusBar { border-top: 1px solid #d0d7de; }
    )");
}

} // namespace dispatch
