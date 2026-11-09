#pragma once
#include "platform/api/keyboard_controller.h"

namespace kobalt::ui {
// A portable widget keyboard for hosts without a native input method.
// The controller owns its panel; the supplied parent places it in the host.
// When language is omitted the application's selected UI language is used.
std::unique_ptr<KeyboardController> makeWidgetKeyboard(QWidget *parent, QLineEdit *editor,
                                                       const QString &language = {});
} // namespace kobalt::ui
