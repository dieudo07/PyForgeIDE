#pragma once
#include <QWidget>

namespace PyForge {

class EditorWidget;

class GutterWidget : public QWidget {
    Q_OBJECT
public:
    explicit GutterWidget(EditorWidget* editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;

private:
    EditorWidget* editor_;
};

} // namespace PyForge