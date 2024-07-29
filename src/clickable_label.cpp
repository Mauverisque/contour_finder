#include "clickable_label.h"

ClickableLabel::ClickableLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent) {}

ClickableLabel::~ClickableLabel() {}

/**
 * @brief Reimplemented method created to handle LMB and RMB presses
 * 
 * @param event Used to get mouse position when pressed
 */
void ClickableLabel::mousePressEvent(QMouseEvent *event) {
  QPoint click_pos{event->pos()};
  if (event->button() == Qt::LeftButton) {
    emit lmbClicked(click_pos);
  } else if (event->button() == Qt::RightButton) {
    emit rmbClicked(click_pos);
  }
}
