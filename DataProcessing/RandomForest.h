// Auto-generated Random Forest Header
// Model: RandomForest
#ifndef RANDOMFOREST_H
#define RANDOMFOREST_H

#include <vector>
#include <string>
//#include <map>
#include <QMap>
#include <cstring>
#include <cstdint>
#include <vector>
#include <QTime>
#include <QTimer>

#include "prjmessage.h"

#define QUICK_HISTORY_POINT_NUM 8
#define HISTORY_POINT_NUM 15
#define SHOW_SCORE 3
#define SHOW_SCORE2 10
#define FIRST_TIME 50000
#define SECOND_TIME 100000
#define DI_STD 3

using namespace radar;
using namespace radar::net;

enum TargetObjectEnum
{
    TGT_UNDETECTED = 0,
    TGT_PEDESTRIAN,     //1行人
    TGT_VEHICLE,        //2车辆
    TGT_DRONE,          //3无人机
    TGT_BIRD,           //4鸟类
    TGT_AIRPLANE = 6,   //6飞机
    TGT_UNKNOWN         //7其他
};


// 文件头
struct TreeFileHeader {
    uint32_t magic;        // 文件标识符，比如 0x52464F52 ("RFOR")
    uint32_t version;      // 版本号
    uint32_t tree_count;   // 树的数量
    uint32_t total_nodes;  // 总节点数
};

// 每个树的元数据
struct TreeMeta {
    uint32_t tree_index;   // 树索引
    uint32_t node_count;   // 该树的节点数
    uint64_t data_offset;  // 节点数据在文件中的偏移量
};

// 节点数据（与TreeNode结构对应）
struct SerializedTreeNode {
    bool isLeaf;
    int featureIndex;
    double threshold;
    int leftChild;
    int rightChild;
    int classIndex;
};



class RandomForest :QObject {
public:
    // 树节点结构
    struct TreeNode {
        bool isLeaf;
        int featureIndex;
        double threshold;
        int leftChild;
        int rightChild;
        int classIndex;
    };

    struct Info {
        std::vector< radar::net::UploadTrackPointsMessageV3::Point> points;
        uint64_t refreshTime;
        uint64_t startTime;
        int targetType;
        int longType;
        int score;
        int score_brid;
        std::vector<double> t;
        std::vector<double> x;
        std::vector<double> y;
        std::vector<double> z;
        std::vector<double> v;
        std::vector<double> rcs;
        std::vector<double> di;
        // std::vector<double> distance;
    };

    // 模型信息
    static const int NUM_TREES = 100;
    static const int NUM_FEATURES = 6;
    static const int NUM_CLASSES = 5;

    radar::net::UploadTrackPointsMessageV3 all_points_in;

    QMap<uint, Info> m_all_track;

    // 创建定时器
    QTimer *timer = new QTimer();


private:
    std::vector<std::vector<TreeNode>> trees; // 动态加载的树

public:

    RandomForest(const std::string& filename);
    ~RandomForest();

    radar::net::UploadTrackPointsMessageV3 updateData(radar::net::UploadTrackPointsMessageV3 data);

    //  加载模型
    bool loadFromFile(const std::string& filename);

    // 预测单棵树
    int predictTree(int treeIndex, const std::vector<double>& features);

    // 预测函数
    int predict(const std::vector<double>& features,  bool IsCorrect, double meanHeigth, double meanV, int& MAXVOTE_OUT);

    // 获取类别名称
    const int getClassName(int classIndex);

    // 计算航迹特征
    std::vector<double> Func_cal_features(const std::vector<double>& t, const std::vector<double>& x, const std::vector<double>& y,
                                          const std::vector<double>& z, const std::vector<double>& v, const std::vector<double>& rcs, const std::vector<double>& di);


    double hight_mad_filtered_mean(std::vector<double> working_data, double mad_threshold);

    // 航迹历史点
    void getHistoryPoint();
};

#endif // RANDOMFOREST_H
