#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <pqxx/pqxx>


#include "clickable_label.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow();

private slots:
  void openImage();
  void exitApp();
  void selectClickedContours(const QPoint &click_pos);
  void showAddContextMenuTable(const QPoint &click_pos);
  void showAddContextMenuLabel(const QPoint &click_pos);
  void showDeleteContextMenu(const QPoint &click_pos);

private:
  void closeEvent(QCloseEvent *event);

  void establishDbConnection();
  void createWidgets();
  void createTables();
  void createActions();
  void createMenus();
  void createLayout();
  int createPopup(const QString &window_title, const QString &text,
                  QMessageBox::Icon icon);

  void fillFoundContoursTable();
  void fillContoursToAddTable();
  void displayAllContours();
  void displaySavedContours();
  void addContours();
  void deleteContours();
  void saveContours();
  std::vector<int> getSelectedContourNum(const QTableWidget &table);

  std::unique_ptr<pqxx::connection> m_conn{};

  QString m_image_path{};
  QString m_image_name{};
  int m_image_height{};
  int m_image_width{};
  std::vector<std::vector<cv::Point>> m_found_contours{};
  std::unordered_map<int, std::string> m_saved_contours{};
  bool m_table_has_changed{false};

  QWidget *central_widget;
  QStackedWidget *stacked_labels;
  QLabel *image_label;
  QLabel *saved_contour_label;
  ClickableLabel *found_contour_label;
  QLabel *highlight_label;
  QPushButton *show_contours_button;
  QPushButton *save_contours_button;

  QTableWidget *found_contours_table;
  QTableWidget *saved_contours_table;

  QAction *open_action;
  QAction *exit_action;
  QAction *add_action;
  QAction *delete_action;
  QMenu *file_menu;
  QMenu *add_context_menu;
  QMenu *delete_context_menu;
};

#endif // MAIN_WINDOW_H_
