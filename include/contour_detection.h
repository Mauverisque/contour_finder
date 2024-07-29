#ifndef CONTOUR_DETECTION_H_
#define CONTOUR_DETECTION_H_

#include <QPixmap>
#include <opencv2/opencv.hpp>

cv::Mat fromQPixmapToCvMat(QPixmap &pixmap);
QPixmap fromCvMatToQPixmap(cv::Mat mat);

std::vector<std::vector<cv::Point>> getContourVector(cv::Mat mat);

cv::Mat drawAllContours(const std::vector<std::vector<cv::Point>> &contours,
                        int img_height, int img_width);
cv::Mat drawSavedContours(const std::vector<std::vector<cv::Point>> &contours,
                          int img_height, int img_width,
                          const std::vector<int> &rows);
cv::Mat drawHighlights(const std::vector<std::vector<cv::Point>> &contours,
                       int img_height, int img_width,
                       const std::vector<int> &rows);

cv::Scalar hueToBgraCvScalar(int hue, int alpha);
QColor hueToRgbaQColor(int hue, int alpha);

int clickedContourNumber(const std::vector<std::vector<cv::Point>> &contours,
                         int x, int y);

#endif // CONTOUR_DETECTION_H_
