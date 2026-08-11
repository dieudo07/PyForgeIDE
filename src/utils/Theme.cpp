#include "Theme.hpp"

namespace PyForge {

Theme& Theme::instance() {
    static Theme t;
    return t;
}

void Theme::applyGitHubDark() {
    name = "GitHub Dark";
    // Couleurs par défaut déjà dans ColorScheme
}

void Theme::applyOneDarkPro() {
    name = "One Dark Pro";
    colors.bg_editor    = QColor("#282c34");
    colors.bg_panel     = QColor("#21252b");
    colors.bg_toolbar   = QColor("#21252b");
    colors.text_primary = QColor("#abb2bf");
    colors.accent       = QColor("#61afef");
    colors.syn_keyword  = QColor("#c678dd");
    colors.syn_string   = QColor("#98c379");
    colors.syn_number   = QColor("#d19a66");
    colors.syn_comment  = QColor("#5c6370");
    colors.syn_function = QColor("#61afef");
    colors.syn_class    = QColor("#e5c07b");
    colors.cursor_line  = QColor("#2c313c");
    colors.selection    = QColor("#3e4451");
}

void Theme::applyMonokai() {
    name = "Monokai Pro";
    colors.bg_editor    = QColor("#2d2a2e");
    colors.bg_panel     = QColor("#221f22");
    colors.bg_toolbar   = QColor("#221f22");
    colors.text_primary = QColor("#fcfcfa");
    colors.accent       = QColor("#ffd866");
    colors.syn_keyword  = QColor("#ff6188");
    colors.syn_string   = QColor("#ffd866");
    colors.syn_number   = QColor("#ab9df2");
    colors.syn_comment  = QColor("#727072");
    colors.syn_function = QColor("#a9dc76");
    colors.syn_class    = QColor("#78dce8");
    colors.cursor_line  = QColor("#3a3a3d");
}

void Theme::applyDracula() {
    name = "Dracula";
    colors.bg_editor    = QColor("#282a36");
    colors.bg_panel     = QColor("#21222c");
    colors.bg_toolbar   = QColor("#21222c");
    colors.text_primary = QColor("#f8f8f2");
    colors.accent       = QColor("#bd93f9");
    colors.syn_keyword  = QColor("#ff79c6");
    colors.syn_string   = QColor("#f1fa8c");
    colors.syn_number   = QColor("#bd93f9");
    colors.syn_comment  = QColor("#6272a4");
    colors.syn_function = QColor("#50fa7b");
    colors.syn_class    = QColor("#8be9fd");
    colors.cursor_line  = QColor("#44475a");
    colors.selection    = QColor("#44475a");
}

} // namespace PyForge