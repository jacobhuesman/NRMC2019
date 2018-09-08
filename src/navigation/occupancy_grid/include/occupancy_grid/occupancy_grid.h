#include <opencv2/opencv.hpp>
#include <math.h>

#include <occupancy_grid/point.h>
#include <occupancy_grid/line.h>
#include <occupancy_grid/circle.h>

#ifndef OCCUPANCY_GRID_OCCUPANCY_GRID_H
#define OCCUPANCY_GRID_OCCUPANCY_GRID_H

namespace occupancy_grid
{

class OccupancyGrid : public cv::Mat_<double>
{
  public:
    OccupancyGrid(int rows, int cols) : cv::Mat_<double>(rows, cols, 0.0) {};
    OccupancyGrid(int rows, int cols, double value) : cv::Mat_<double>(rows, cols, value) {};

    static void max(OccupancyGrid const &a, OccupancyGrid const &b, OccupancyGrid *out);

    void show();
    void write();

    void draw(Point const &a);
    void draw(Line const &a, int thickness);
    void draw(Circle const &a);

    static void inflate(OccupancyGrid const &in, OccupancyGrid *out,
                        double cutoff=0.7, int kernel_size=100, int passes=2);
};



}

#endif //OCCUPANCY_GRID_OCCUPANCY_GRID_H
