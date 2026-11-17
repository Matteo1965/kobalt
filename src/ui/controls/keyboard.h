#pragma once
#include "platform/api/keyboard_controller.h"
#include <QString>

namespace kobalt::ui {
// A portable widget keyboard for hosts without a native input method.
// The controller owns its panel; the supplied parent places it in the host.
// Preserve the original two-argument factory used by desktop and existing hosts.
std::unique_ptr<KeyboardController> makeWidgetKeyboard(QWidget *parent, QLineEdit *editor);
// Kobo Hungarian Edition selects an explicit layout without altering host APIs.
std::unique_ptr<KeyboardController> makeLocalizedWidgetKeyboard(QWidget *parent, QLineEdit *editor,
                                                                const QString &language);
} // namespace kobalt::ui
