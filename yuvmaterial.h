#ifndef YUVMATERIAL_H
#define YUVMATERIAL_H

#include <QQuickItem>

class SimpleMaterialItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    QML_ELEMENT

public:

    SimpleMaterialItem() { setFlag(ItemHasContents, true); }

    void setColor(const QColor &color);
    QColor color() const { return m_color; }

signals:
    void colorChanged();

private:
  QColor m_color;

public:
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;
};

#endif // YUVMATERIAL_H
