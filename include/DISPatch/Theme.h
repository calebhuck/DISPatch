#pragma once

#include <QtCore/QString>
#include <QtCore/QtGlobal>
#include <QtGui/QPalette>

namespace dispatch {

enum class Theme : quint8 {
    Dark,
    Light,
    Gruvbox
};

auto themePalette(Theme theme) -> QPalette;
auto themeStyleSheet(Theme theme) -> QString;

} // namespace dispatch
