#include <iostream>
#include <fstream>
#include <opencv2/imgcodecs.hpp>
#include "Eigen/Geometry"
#include "pcl/point_types.h"
#include "pcl/io/pcd_io.h"

using namespace std;
using namespace cv;

int main() {
    vector<Mat> colorImgs, depthImgs;       // 彩色图与深度图
    vector<Eigen::Isometry3d> poses;        // 相机位姿

    ifstream fin("../pose.txt");
    if (!fin){
        cout << "cannot open pose.txt" << endl;
        return -1;
    }

    for (int i = 0; i < 5; ++i){
        colorImgs.emplace_back(imread("../color/" + to_string(i) + ".png", IMREAD_UNCHANGED));
        depthImgs.emplace_back(imread("../depth/" + to_string(i) + ".pgm", IMREAD_UNCHANGED));

        double data[7] = {0};
        for (auto& d :data){
            fin >> d;
        }
        Eigen::Quaterniond q(data[6], data[3], data[4], data[5]);
        Eigen::Isometry3d T(q);
        T.pretranslate(Eigen::Vector3d(data[0], data[1], data[2]));
        poses.push_back(T);
    }

    //计算点云拼接
    double cx = 325.5, cy = 253.5, fx = 518.0, fy = 519.0, depthScale = 1000.0;         // 相机内参

    cout << "Convert image to point cloud..." << endl;
    // 定义点云使用的格式，这里使用的是XYRGB
    typedef pcl::PointXYZRGB PointT;
    typedef pcl::PointCloud<PointT> PointCloud;

    // 新建一个点云
    PointCloud::Ptr pointCloud(new PointCloud);
    for (int i = 0; i < 5; ++i) {
        cout << "Convert image..." << i + 1 << endl;
        Mat color = colorImgs[i];
        Mat depth = depthImgs[i];
        Eigen::Isometry3d T = poses[i];
        for (int v = 0; v < color.rows; ++v){
            for (int u = 0; u < color.cols; ++u) {
                unsigned int d = depth.ptr<unsigned short>(v)[u];       // 深度值
                if (0 == d) continue;       // 表示没有测量到
                Eigen::Vector3d point;
                point[2] = double(d) / depthScale;
                point[0] = (u - cx) * point[2] / fx;
                point[1] = (v - cy) * point[2] / fy;
                Eigen::Vector3d pointWorld = T * point;

                PointT p;
                p.x = pointWorld[0];
                p.y = pointWorld[1];
                p.z = pointWorld[2];
                p.b = color.data[v * color.step + u * color.channels()];
                p.g = color.data[v * color.step + u * color.channels() + 1];
                p.r = color.data[v * color.step + u * color.channels() + 1];
                pointCloud->points.emplace_back(p);
            }
        }
    }

    pointCloud->is_dense = false;
    cout << "Point Cloud Number = " << pointCloud->size() << endl;
    pcl::io::savePCDFileBinary("map.pcd", *pointCloud);

    return 0;
}