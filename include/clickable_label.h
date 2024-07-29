#ifndef CLICKABLE_LABEL_H_
#define CLICKABLE_LABEL_H_

#include <QLabel>
#include <QMouseEvent>

class ClickableLabel : public QLabel {
  Q_OBJECT

public:
  explicit ClickableLabel(QWidget *parent = Q_NULLPTR,
                          Qt::WindowFlags f = Qt::WindowFlags());
  ~ClickableLabel();

signals:
  void lmbClicked(const QPoint &click_pos);
  void rmbClicked(const QPoint &click_pos);

protected:
  void mousePressEvent(QMouseEvent *event);
};

#endif // CLICKABLE_LABEL_H_
