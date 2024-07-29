#include "main_window.h"
#include "contour_detection.h"
#include "sql_query_handler.h"

MainWindow::MainWindow() {
  setWindowTitle("Contours");
  setMinimumSize(800, 600);

  establishDbConnection();
  createWidgets();
  createTables();
  createActions();
  createMenus();
  createLayout();

  connect(show_contours_button, &QPushButton::clicked, this,
          displayAllContours);
  connect(save_contours_button, &QPushButton::clicked, this, saveContours);

  connect(found_contour_label, SIGNAL(lmbClicked(const QPoint &)), this,
          SLOT(selectClickedContours(const QPoint &)));
  connect(found_contour_label, SIGNAL(rmbClicked(const QPoint &)), this,
          SLOT(showAddContextMenuLabel(const QPoint &)));

  connect(found_contours_table, &QTableWidget::itemSelectionChanged, this,
          displayAllContours);
  connect(saved_contours_table, &QTableWidget::itemSelectionChanged, this,
          displaySavedContours);

  connect(found_contours_table,
          SIGNAL(customContextMenuRequested(const QPoint &)), this,
          SLOT(showAddContextMenuTable(const QPoint &)));
  connect(saved_contours_table,
          SIGNAL(customContextMenuRequested(const QPoint &)), this,
          SLOT(showDeleteContextMenu(const QPoint &)));

  connect(saved_contours_table, &QTableWidget::itemChanged, this,
          [this]() { m_table_has_changed = true; });
}

void MainWindow::openImage() {
  int ret{QMessageBox::Yes};
  if (m_table_has_changed) {
    ret = createPopup(
        "Proceed Without Saving?",
        "The changes have not been saved. Opening another image will discard "
        "all the changes. Are you sure you want to proceed?",
        QMessageBox::Warning);
  }

  if (ret == QMessageBox::Yes) {
    QFileDialog dialog(this, "Open Image");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Images (*.png *.jpg *.jpeg)");

    if (dialog.exec() == QDialog::Accepted &&
        m_image_path != dialog.selectedFiles().first()) {
      m_image_path = dialog.selectedFiles().first();
      QPixmap image{m_image_path};
      m_image_name = QFileInfo(m_image_path).fileName();

      // Resize image if it's too tall
      if (image.height() > 800) {
        image = image.scaled(image.width(), 800, Qt::KeepAspectRatio);
      }

      m_image_height = image.height();
      m_image_width = image.width();
      m_found_contours = getContourVector(fromQPixmapToCvMat(image));

      image_label->setFixedHeight(m_image_height);
      image_label->setFixedWidth(m_image_width);
      image_label->setPixmap(image);
      image_label->show();

      saved_contour_label->setFixedHeight(m_image_height);
      saved_contour_label->setFixedWidth(m_image_width);

      found_contour_label->setFixedHeight(m_image_height);
      found_contour_label->setFixedWidth(m_image_width);

      highlight_label->setFixedHeight(m_image_height);
      highlight_label->setFixedWidth(m_image_width);

      m_saved_contours.clear();
      found_contours_table->setRowCount(0);
      fillFoundContoursTable();
      saved_contours_table->setRowCount(0);
      fillContoursToAddTable();
      saved_contour_label->clear();
      displaySavedContours();
      found_contour_label->clear();
      highlight_label->clear();
      displayAllContours();
      m_table_has_changed = false;
    }
  }
}

void MainWindow::exitApp() { this->close(); }

void MainWindow::selectClickedContours(const QPoint &click_pos) {
  if (clickedContourNumber(m_found_contours, click_pos.x(), click_pos.y()) ==
      -1) {
    found_contours_table->clearSelection();
  } else {
    found_contours_table->selectRow(
        clickedContourNumber(m_found_contours, click_pos.x(), click_pos.y()));
  }
}

void MainWindow::showAddContextMenuTable(const QPoint &click_pos) {
  QModelIndex index{found_contours_table->indexAt(click_pos)};
  if (index.isValid()) {
    add_context_menu->popup(found_contours_table->mapToGlobal(click_pos));
  }
}

void MainWindow::showAddContextMenuLabel(const QPoint &click_pos) {
  if (clickedContourNumber(m_found_contours, click_pos.x(), click_pos.y()) !=
      -1) {
    add_context_menu->popup(image_label->mapToGlobal(click_pos));
  }
}

void MainWindow::showDeleteContextMenu(const QPoint &click_pos) {
  QModelIndex index{saved_contours_table->indexAt(click_pos)};
  if (index.isValid()) {
    delete_context_menu->popup(saved_contours_table->mapToGlobal(click_pos));
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (m_table_has_changed) {
    int ret =
        createPopup("Exit Without Saving?",
                    "The changes have not been saved. Exiting will discard all "
                    "the changes. Are you sure you want to proceed?",
                    QMessageBox::Warning);
    if (ret == QMessageBox::Yes) {
      event->accept();
    } else {
      event->ignore();
    }
  }
}

void MainWindow::establishDbConnection() {
  // Use your database credentials
  m_conn = std::make_unique<pqxx::connection>(
      "dbname = ... user = ... password = ... hostaddr = ... port = ...");
}

void MainWindow::createWidgets() {
  central_widget = new QWidget{this};
  setCentralWidget(central_widget);

  stacked_labels = new QStackedWidget{this};
  image_label = new QLabel{this};
  saved_contour_label = new QLabel{this};
  found_contour_label = new ClickableLabel{this};
  highlight_label = new QLabel{this};
  stacked_labels->addWidget(found_contour_label);
  stacked_labels->addWidget(highlight_label);
  stacked_labels->addWidget(saved_contour_label);
  stacked_labels->addWidget(image_label);
  image_label->setAlignment(Qt::AlignCenter);

  show_contours_button = new QPushButton{"Show Contours", this};
  show_contours_button->setCheckable(true);
  save_contours_button = new QPushButton{"Save Changes", this};
}

void MainWindow::createTables() {
  found_contours_table = new QTableWidget{this};
  found_contours_table->setColumnCount(1);
  found_contours_table->verticalHeader()->setSectionResizeMode(
      QHeaderView::Fixed);
  found_contours_table->horizontalHeader()->setStretchLastSection(true);
  found_contours_table->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Fixed);
  QList<QString> found_contours_table_headers{"All Found Contours"};
  found_contours_table->setHorizontalHeaderLabels(found_contours_table_headers);
  found_contours_table->setContextMenuPolicy(Qt::CustomContextMenu);
  found_contours_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  saved_contours_table = new QTableWidget{this};
  saved_contours_table->setColumnCount(1);
  saved_contours_table->verticalHeader()->setSectionResizeMode(
      QHeaderView::Fixed);
  saved_contours_table->horizontalHeader()->setStretchLastSection(true);
  saved_contours_table->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Fixed);
  QList<QString> saved_contours_table_headers{"Saved Contours"};
  saved_contours_table->setHorizontalHeaderLabels(saved_contours_table_headers);
  saved_contours_table->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainWindow::createActions() {
  open_action = new QAction{QIcon::fromTheme(QIcon::ThemeIcon::FolderOpen),
                            "Open Image", this};
  connect(open_action, &QAction::triggered, this, openImage);

  exit_action = new QAction{QIcon::fromTheme(QIcon::ThemeIcon::WindowClose),
                            "Exit Application", this};
  connect(exit_action, &QAction::triggered, this, exitApp);

  add_action =
      new QAction{QIcon::fromTheme(QIcon::ThemeIcon::ListAdd), "Add", this};
  connect(add_action, &QAction::triggered, this, addContours);

  delete_action = new QAction{QIcon::fromTheme(QIcon::ThemeIcon::EditDelete),
                              "Delete", this};
  connect(delete_action, &QAction::triggered, this, deleteContours);
}

void MainWindow::createMenus() {
  file_menu = menuBar()->addMenu("File");
  QList action_list{open_action, exit_action};
  file_menu->addActions(action_list);

  add_context_menu = new QMenu{this};
  add_context_menu->addAction(add_action);

  delete_context_menu = new QMenu{this};
  delete_context_menu->addAction(delete_action);
}

void MainWindow::createLayout() {
  QVBoxLayout *v_main_layout = new QVBoxLayout;
  v_main_layout->addSpacing(10);
  v_main_layout->addWidget(stacked_labels, 0, Qt::AlignCenter);
  stacked_labels->setMinimumHeight(this->height() / 2);
  QHBoxLayout *h_tables_layout = new QHBoxLayout;
  h_tables_layout->addSpacing(50);
  QVBoxLayout *v_found_contours_table_layout = new QVBoxLayout;
  v_found_contours_table_layout->addWidget(show_contours_button);
  v_found_contours_table_layout->addWidget(found_contours_table);
  h_tables_layout->addLayout(v_found_contours_table_layout);
  h_tables_layout->addSpacing(50);
  QVBoxLayout *v_saved_contours_table_layout = new QVBoxLayout;
  v_saved_contours_table_layout->addWidget(save_contours_button);
  v_saved_contours_table_layout->addWidget(saved_contours_table);
  h_tables_layout->addLayout(v_saved_contours_table_layout);
  h_tables_layout->addSpacing(50);
  v_main_layout->addLayout(h_tables_layout);
  v_main_layout->addSpacing(10);

  central_widget->setLayout(v_main_layout);
}

int MainWindow::createPopup(const QString &window_title, const QString &text,
                            QMessageBox::Icon icon) {
  QMessageBox message_box{this};
  message_box.setWindowTitle(window_title);
  message_box.setText(text);
  message_box.setIcon(icon);
  message_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  message_box.setDefaultButton(QMessageBox::No);

  return message_box.exec();
}

void MainWindow::fillFoundContoursTable() {
  found_contours_table->setRowCount(0);
  for (const auto &vect : m_found_contours) {
    found_contours_table->insertRow(found_contours_table->rowCount());
    QTableWidgetItem *new_item = new QTableWidgetItem(
        tr("Contour №%1").arg(found_contours_table->rowCount()));
    // Item data stores its contour number,
    // in this case it equals to (item's row - 1).
    new_item->setData(Qt::UserRole, found_contours_table->rowCount() - 1);
    new_item->setBackground(
        hueToRgbaQColor(80 * (new_item->data(Qt::UserRole).toInt()) % 360, 63));
    found_contours_table->setItem(found_contours_table->rowCount() - 1, 0,
                                  new_item);
  }
}

void MainWindow::fillContoursToAddTable() {
  saved_contours_table->setRowCount(0);
  std::vector<std::pair<int, std::string>> saved_contours{
      getContoursFromDb(*m_conn, m_image_name.toStdString())};
  for (const auto &pair : saved_contours) {
    saved_contours_table->insertRow(saved_contours_table->rowCount());
    QTableWidgetItem *new_item =
        new QTableWidgetItem{QString::fromStdString(pair.second)};
    // Item data stores its contour number,
    new_item->setData(Qt::UserRole, pair.first);
    new_item->setBackground(
        hueToRgbaQColor(80 * (new_item->data(Qt::UserRole).toInt()) % 360, 63));
    saved_contours_table->setItem(saved_contours_table->rowCount() - 1, 0,
                                  new_item);
    m_saved_contours.insert({pair.first, pair.second});
  }
}

void MainWindow::displayAllContours() {
  if (!m_image_path.isEmpty()) {
    if (show_contours_button->isChecked()) {
      found_contour_label->setPixmap(fromCvMatToQPixmap(
          drawAllContours(m_found_contours, m_image_height, m_image_width)));
      found_contour_label->show();
      highlight_label->setPixmap(fromCvMatToQPixmap(
          drawHighlights(m_found_contours, m_image_height, m_image_width,
                         getSelectedContourNum(*found_contours_table))));
      highlight_label->show();
    } else {
      found_contour_label->hide();
      highlight_label->hide();
    }
  }
}

void MainWindow::displaySavedContours() {
  // Extracting the numbers of saved contours.
  std::vector<int> saved_contours_num;
  for (const auto &elem : m_saved_contours) {
    saved_contours_num.push_back(elem.first);
  }
  saved_contour_label->setPixmap(fromCvMatToQPixmap(drawSavedContours(
      m_found_contours, m_image_height, m_image_width, saved_contours_num)));
  saved_contour_label->show();
  highlight_label->setPixmap(fromCvMatToQPixmap(
      drawHighlights(m_found_contours, m_image_height, m_image_width,
                     getSelectedContourNum(*saved_contours_table))));
  highlight_label->show();
}

void MainWindow::addContours() {
  for (int row : getSelectedContourNum(*found_contours_table)) {
    // Using std::unordered_map to store added contours prevents user from
    // adding contours that have already been added.
    if (auto search = m_saved_contours.find(row);
        search == m_saved_contours.end()) {
      // Contour names in m_saved_contours are not entered until the user
      // commits the changes via saveContours method.
      m_saved_contours.insert({row, ""});
      saved_contours_table->insertRow(saved_contours_table->rowCount());
      QTableWidgetItem *new_item =
          new QTableWidgetItem{*(found_contours_table->item(row, 0))};
      saved_contours_table->setItem(saved_contours_table->rowCount() - 1, 0,
                                    new_item);
      displaySavedContours();
      m_table_has_changed = true;
    }
  }
  found_contours_table->clearSelection();
}

void MainWindow::deleteContours() {
  int ret = createPopup("Delete Contours?",
                        "Are you sure you want to delete these contours?",
                        QMessageBox::Question);
  if (ret == QMessageBox::Yes) {
    QList<QTableWidgetItem *> selected_items =
        saved_contours_table->selectedItems();
    for (const QTableWidgetItem *item : selected_items) {
      m_saved_contours.erase(item->data(Qt::UserRole).toInt());
      saved_contours_table->removeRow(item->row());
    }
    displaySavedContours();
    m_table_has_changed = true;
  }
}

void MainWindow::saveContours() {
  if (!m_image_path.isEmpty()) {
    int ret = createPopup("Save Changes?",
                          "Are you sure you want to save these changes?",
                          QMessageBox::Question);
    if (ret == QMessageBox::Yes) {
      for (int row = 0; row < saved_contours_table->rowCount(); ++row) {
        QTableWidgetItem *item = saved_contours_table->item(row, 0);
        m_saved_contours[item->data(Qt::UserRole).toInt()] =
            item->text().toStdString();
      }
      addContoursToDb(*m_conn, m_image_name.toStdString(), m_saved_contours);
      m_table_has_changed = false;
    }
  }
}

std::vector<int> MainWindow::getSelectedContourNum(const QTableWidget &table) {
  std::vector<int> selected_rows{};
  QList<QTableWidgetItem *> selected_items = table.selectedItems();

  for (const QTableWidgetItem *item : selected_items) {
    selected_rows.push_back(item->data(Qt::UserRole).toInt());
  }

  return selected_rows;
}
