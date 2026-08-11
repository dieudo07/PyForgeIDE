#pragma once
#include <QColor>
#include <QString>
#include <QFont>

namespace PyForge {

struct ColorScheme {
    // Fond
    QColor bg_editor     = QColor("#0d1117");
    QColor bg_panel      = QColor("#161b22");
    QColor bg_sidebar    = QColor("#010409");
    QColor bg_toolbar    = QColor("#161b22");
    QColor bg_tab        = QColor("#21262d");
    QColor bg_tab_active = QColor("#0d1117");
    QColor bg_selected   = QColor("#264f78");
    QColor bg_hover      = QColor("#1f2937");
    QColor bg_input      = QColor("#21262d");
    QColor bg_dropdown   = QColor("#1c2128");

    // Texte
    QColor text_primary   = QColor("#e6edf3");
    QColor text_secondary = QColor("#7d8590");
    QColor text_dim       = QColor("#484f58");
    QColor text_accent    = QColor("#58a6ff");

    // Syntaxe Python
    QColor syn_keyword   = QColor("#ff7b72");
    QColor syn_builtin   = QColor("#d2a8ff");
    QColor syn_string    = QColor("#a5d6ff");
    QColor syn_number    = QColor("#79c0ff");
    QColor syn_comment   = QColor("#8b949e");
    QColor syn_decorator = QColor("#d2a8ff");
    QColor syn_function  = QColor("#d2a8ff");
    QColor syn_class     = QColor("#ffa657");
    QColor syn_operator  = QColor("#ff7b72");
    QColor syn_self      = QColor("#ff7b72");
    QColor syn_type      = QColor("#ffa657");
    QColor syn_import    = QColor("#ff7b72");
    QColor syn_constant  = QColor("#79c0ff");
    QColor syn_escape    = QColor("#ffa657");
    QColor syn_fstring   = QColor("#79c0ff");

    // UI
    QColor accent        = QColor("#58a6ff");
    QColor accent_hover  = QColor("#388bfd");
    QColor success       = QColor("#3fb950");
    QColor warning       = QColor("#d29922");
    QColor error         = QColor("#f85149");

    // Éditeur
    QColor gutter_bg     = QColor("#0d1117");
    QColor gutter_text   = QColor("#484f58");
    QColor gutter_active = QColor("#e6edf3");
    QColor cursor_line   = QColor("#161b22");
    QColor selection     = QColor("#264f78");
    QColor match_bracket = QColor("#388bfd");
    QColor indent_guide  = QColor("#21262d");

    // Diagnostics
    QColor diag_error    = QColor("#f85149");
    QColor diag_warning  = QColor("#d29922");
    QColor diag_info     = QColor("#58a6ff");
    QColor diag_hint     = QColor("#3fb950");
};

class Theme {
public:
    static Theme& instance();
    ColorScheme colors;
    QString     name = "GitHub Dark";

    void applyGitHubDark();
    void applyOneDarkPro();
    void applyMonokai();
    void applyDracula();

private:
    Theme() { applyGitHubDark(); }
};

} // namespace PyForge