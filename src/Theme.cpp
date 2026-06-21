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
