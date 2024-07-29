#include "contour_detection.h"

cv::Mat fromQPixmapToCvMat(QPixmap &q_pixmap) {
  QImage img{q_pixmap.toImage()};
  if (img.format() != QImage::Format_RGB888) {
    img = img.convertToFormat(QImage::Format_RGB888);
  }

  return cv::Mat(img.height(), img.width(), CV_8UC3, img.bits(),
                 img.bytesPerLine())
      .clone();
}

QPixmap fromCvMatToQPixmap(cv::Mat mat) {
  cv::cvtColor(mat, mat, cv::COLOR_BGRA2RGBA);
  QImage image((uchar *)mat.data, mat.cols, mat.rows, QImage::Format_RGBA8888);

  return QPixmap::fromImage(image);
}

/**
 * @brief Contour detection logic. Currently detects contours of red objects.
 *        Can be changed as per programmer's needs.
 *
 * @param img Image to process.
 * @return An array of contours presented as arrays of 'cv::Point's.
 */
std::vector<std::vector<cv::Point>> getContourVector(cv::Mat img) {
  cv::Mat img_HSV{};
  cv::cvtColor(img, img_HSV, cv::COLOR_RGB2HSV);

  // Color red in HSV color space has both 0 and 180 hue value,
  // so to get all of red's shades two pairs of thresholds are needed.
  cv::Scalar red_lower{0, 90, 90};
  cv::Scalar red_upper{4, 255, 255};
  cv::Scalar red_lower_2{176, 90, 90};
  cv::Scalar red_upper_2{180, 255, 255};

  cv::Mat red_range{};
  cv::Mat red_range_2{};
  cv::inRange(img_HSV, red_lower, red_upper, red_range);
  cv::inRange(img_HSV, red_lower_2, red_upper_2, red_range_2);
  cv::add(red_range, red_range_2, red_range);

  // Image erosion followed by dilation.
  // Used to remove contours with tiny area from the image.
  cv::morphologyEx(
      red_range, red_range, cv::MORPH_OPEN,
      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
  // Image dilation followed by erosion.
  // Used to fuse together nearby contours.
  cv::morphologyEx(
      red_range, red_range, cv::MORPH_CLOSE,
      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9)));

  std::vector<std::vector<cv::Point>> red_contours{};
  // cv::RETR_EXTERNAL mode is used to prevent having contours inside other
  // contours.
  cv::findContours(red_range, red_contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_SIMPLE);

  return red_contours;
}

cv::Mat drawAllContours(const std::vector<std::vector<cv::Point>> &contours,
                        int img_height, int img_width) {
  cv::Mat mat = cv::Mat::zeros(img_height, img_width, CV_8UC4);

  for (int i = 0; i != contours.size(); ++i) {
    int hue = 80 * i % 360;
    cv::drawContours(mat, contours, i, hueToBgraCvScalar(hue, 255), 2);
  }

  return mat;
}

cv::Mat drawSavedContours(const std::vector<std::vector<cv::Point>> &contours,
                          int img_height, int img_width,
                          const std::vector<int> &rows) {
  cv::Mat mat = cv::Mat::zeros(img_height, img_width, CV_8UC4);

  for (int row : rows) {
    int hue = 80 * row % 360;
    cv::drawContours(mat, contours, row, hueToBgraCvScalar(hue, 63), -1);
    cv::drawContours(mat, contours, row, hueToBgraCvScalar(hue, 255), 2);
  }

  return mat;
}

cv::Mat drawHighlights(const std::vector<std::vector<cv::Point>> &contours,
                       int img_height, int img_width,
                       const std::vector<int> &rows) {
  cv::Mat mat = cv::Mat::zeros(img_height, img_width, CV_8UC4);

  for (int row : rows) {
    int hue = 80 * row % 360;
    cv::drawContours(mat, contours, row, hueToBgraCvScalar(hue, 255), -1);
  }

  return mat;
}

cv::Scalar hueToBgraCvScalar(int hue, int alpha) {
  int k_red = 127.5 * ((10 + hue / 30) % 12);
  int k_green = 127.5 * ((6 + hue / 30) % 12);
  int k_blue = 127.5 * ((2 + hue / 30) % 12);

  int red = 255 - std::max(std::min(std::min(k_red, 1020 - k_red), 255), 0);
  int green =
      255 - std::max(std::min(std::min(k_green, 1020 - k_green), 255), 0);
  int blue = 255 - std::max(std::min(std::min(k_blue, 1020 - k_blue), 255), 0);

  return cv::Scalar(blue, green, red, alpha);
}

QColor hueToRgbaQColor(int hue, int alpha) {
  int k_red = 127.5 * ((10 + hue / 30) % 12);
  int k_green = 127.5 * ((6 + hue / 30) % 12);
  int k_blue = 127.5 * ((2 + hue / 30) % 12);

  int red = 255 - std::max(std::min(std::min(k_red, 1020 - k_red), 255), 0);
  int green =
      255 - std::max(std::min(std::min(k_green, 1020 - k_green), 255), 0);
  int blue = 255 - std::max(std::min(std::min(k_blue, 1020 - k_blue), 255), 0);

  return QColor{red, green, blue, alpha};
}

/**
 * @brief Tests whether (x, y) is within any of contours and if so, which one.
 *
 * @param contours Array of contours to test;
 * @param x X coordinate;
 * @param y Y coordinate.
 * @return If such contour is found returns its number in the array, else
 *         returns -1.
 */
int clickedContourNumber(const std::vector<std::vector<cv::Point>> &contours,
                         int x, int y) {
  for (int i = 0; i != contours.size(); ++i) {
    if (cv::pointPolygonTest(contours[i], cv::Point(x, y), false) != -1) {
      return i;
    }
  }

  return -1;
}
