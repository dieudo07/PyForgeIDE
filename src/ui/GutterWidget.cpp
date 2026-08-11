#include "GutterWidget.hpp"
#include "EditorWidget.hpp"
#include "../utils/Theme.hpp"
#include <QPainter>
#include <QMouseEvent>

namespace PyForge {

GutterWidget::GutterWidget(EditorWidget* editor)
    : QWidget(editor), editor_(editor) {}

QSize GutterWidget::sizeHint() const {
    return QSize(editor_->gutterWidth(), 0);
}

void GutterWidget::paintEvent(QPaintEvent* e) {
    const auto& c = Theme::instance().colors;
    QPainter p(this);
    p.fillRect(e->rect(), c.gutter_bg);

    // Ligne de séparation droite
    p.setPen(QPen(c.indent_guide, 1));
    p.drawLine(width() - 1, 0, width() - 1, height());

    QTextBlock blk = editor_->firstVisibleBlock();
    int top = static_cast<int>(
        editor_->blockBoundingGeometry(blk)
        .translated(editor_->contentOffset()).top());

    int ln    = blk.blockNumber() + 1;
    int curLn = editor_->textCursor().blockNumber() + 1;

    while (blk.isValid() && top <= e->rect().bottom()) {
        if (blk.isVisible() && top >= e->rect().top()) {
            const int h = static_cast<int>(
                editor_->blockBoundingRect(blk).height());
            p.setFont(editor_->font());
            p.setPen(ln == curLn
                     ? c.gutter_active
                     : c.gutter_text);
            p.drawText(0, top, width() - 8, h,
                       Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(ln));
        }
        top += static_cast<int>(
            editor_->blockBoundingRect(blk).height());
        blk = blk.next();
        ++ln;
    }
}

void GutterWidget::mousePressEvent(QMouseEvent*) {}

} // namespace PyForge