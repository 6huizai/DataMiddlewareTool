#include <iostream>
#include <fstream>
#include <vector>
#include <QDebug>
#include "RandomForest.h"
#include <vector>
#include <cmath>
#include <algorithm>



RandomForest:: RandomForest(const std::string& filename)
{

    // 加载模型
    if (!loadFromFile(filename)) {
        qDebug() << "Failed to load RandomForest model";
    }

    // 连接信号槽：定时到达时更新标签文本
    // QObject::connect(timer, &QTimer::timeout, [=]() {
    //     uint64_t currtime = QDateTime::currentMSecsSinceEpoch();
    //     for (int i = m_all_track.size() - 1; i >= 0; i--) {
    //         if (currtime - m_all_track.at(i).refreshTime > 4000) {
    //             m_all_track.erase(i);
    //         }
    //     }
    // });
    QObject::connect(timer, &QTimer::timeout, [this]() { // 注意：用[this]捕获当前对象，才能修改成员变量m_all_track
        uint64_t currtime = QDateTime::currentMSecsSinceEpoch();

        // QMap通过迭代器遍历，需用QMap::iterator
        // 从开始迭代，删除元素时利用erase返回的下一个迭代器继续遍历（避免迭代器失效）
        auto it = m_all_track.begin();
        while (it != m_all_track.end()) {
            // 通过it.value()获取键对应的值（TrackData对象）
            if (currtime - it.value().refreshTime > 6800) {
                // QMap::erase(it)删除当前元素，并返回下一个有效的迭代器
                it = m_all_track.erase(it);
            } else {
                // 未超时，迭代器移至下一个元素
                ++it;
            }
        }
    });

    // 启动定时器（开始周期性触发）
    timer->start(1000); //1000ms
}

RandomForest:: ~RandomForest()
{
    if(timer)
    {
        delete timer;
        timer = nullptr;
    }
}


// 类别名称映射
const int RandomForest::getClassName(int classIndex) {
    // static const char* CLASS_NAMES[] = {"1", "2", "3", "4", "6"};
    // if (classIndex < 0 || classIndex >= 5) return "7";
    // static const char* CLASS_NAMES[] = {"Pedestrian", "Vehicle", "Drone", "Bird", "Airplane"};
    // if (classIndex < 0 || classIndex >= 5) return "Unknown";

    static const int CLASS_NAMES[] = {1, 2, 3, 4, 6};
    if (classIndex < 0 || classIndex >= 5) return 7;

    return CLASS_NAMES[classIndex];
}

// 加载随机森林模型
bool RandomForest::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        qDebug() << "file error.";
        return false;
    }

    TreeFileHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.magic != 0x52464F52)
    {
        qDebug() << "head error.";
        return false;
    }

    trees.resize(header.tree_count);

    // 读取树元数据
    std::vector<TreeMeta> tree_metas(header.tree_count);
    file.read(reinterpret_cast<char*>(tree_metas.data()),
              header.tree_count * sizeof(TreeMeta));

    // 读取每棵树的节点数据
    for (const auto& meta : tree_metas) {
        file.seekg(meta.data_offset);
        trees[meta.tree_index].resize(meta.node_count);

        for (uint32_t i = 0; i < meta.node_count; ++i) {
            SerializedTreeNode snode;
            file.read(reinterpret_cast<char*>(&snode), sizeof(snode));

            auto& node = trees[meta.tree_index][i];
            node.isLeaf = snode.isLeaf;
            node.featureIndex = snode.featureIndex;
            node.threshold = snode.threshold;
            node.leftChild = snode.leftChild;
            node.rightChild = snode.rightChild;
            node.classIndex = snode.classIndex;
        }
    }

    file.close();
    return true;
}


// 单棵树预测
int RandomForest::predictTree(int treeIndex, const std::vector<double>& features) {

    // 检查树索引是否有效
    if (treeIndex < 0 || treeIndex >= trees.size()) {
        return 0; // 返回默认类别
    }

    const std::vector<TreeNode>& tree = trees[treeIndex];
    int nodeIndex = 0;

    // 遍历决策树
    while (!tree[nodeIndex].isLeaf) {
        const TreeNode& node = tree[nodeIndex];

        // 检查特征索引是否有效
        if (node.featureIndex < 0 || node.featureIndex >= features.size()) {
            return 0; // 特征索引越界，返回默认类别
        }

        if (features[node.featureIndex] <= node.threshold) {
            nodeIndex = node.leftChild;
        } else {
            nodeIndex = node.rightChild;
        }

        // 检查节点索引是否有效
        if (nodeIndex < 0 || nodeIndex >= tree.size()) {
            return 0; // 节点索引越界，返回默认类别
        }
    }

    return tree[nodeIndex].classIndex;

}

// 主预测函数
int RandomForest::predict(const std::vector<double>& features, bool IsCorrect, double meanHeigth, double meanV, int& MAXVOTE_OUT) {
    if (features.size() != NUM_FEATURES) {
        qDebug()<< "Error: Feature size mismatch";
        return -1;
    }

    std::map<int, int> votes;

    // 收集所有树的投票
    for (int i = 0; i < NUM_TREES; ++i) {
        int classIdx = predictTree(i, features);
        votes[classIdx]++;
    }

    // 找到得票最多的类别
    int bestClass = 0;
    int maxVotes = 0;
    for (const auto& vote : votes) {
        if (vote.second > maxVotes) {
            maxVotes = vote.second;
            bestClass = vote.first;
        }
    }

    // MAXVOTE_OUT = maxVotes;


    if(IsCorrect)
    {
        if(meanHeigth < 80)
        {
            if(bestClass == 4) //高度很小的情况下，判为飞机时，修正为汽车
            {
                bestClass = 1;
            }

            if( bestClass == 0) //判为行人时，速度较大修正为汽车
            {
                if(meanV > 5)
                {
                    bestClass = 1;
                }
            }

            if(bestClass == 2) //判为无人机时，修正为汽车
            {


                if(meanHeigth < 0)
                {
                    bestClass = 1;
                }
                else if(meanHeigth < 10)
                {
                    bestClass = -1;
                }

                // qDebug() <<"ID: " << MAXVOTE_OUT << "before bestClass = 2, meanHeigth= " << meanHeigth << ", after bestClass = " << bestClass;

            }

        }

        if(meanHeigth > 300.0)
        {
            if(bestClass == 1) //高度大的情况下，判为汽车时，修正为飞机
            {
                if(meanV > 25) //同时速度较大，修正为飞机
                {
                    bestClass = 4;
                }
                else            //否则修正为鸟类
                {
                    bestClass = 3;
                }
            }

            if( bestClass == 0) //判为行人时，修正为鸟类
            {
                bestClass = 3;
            }
        }
    }

    if(bestClass == 2)
    {
        if(maxVotes < 50)
        {
            bestClass = -1; // 最多类型得票少于80票，归为未知类别
        }
    }
    else if(bestClass == 3)
    {
        if(maxVotes < 70)
        {
            bestClass = -1; // 最多类型得票少于70票，归为未知类别
        }
    }
    else
    {
        if(maxVotes < 60)
        {
            bestClass = -1; // 最多类型得票少于60票，归为未知类别
        }
    }


    MAXVOTE_OUT = maxVotes;


    // return bestClass;
    return getClassName(bestClass);
}


void RandomForest::getHistoryPoint()
{
    if(all_points_in.points.size() > 0)
    {
        for(int ii=0; ii< all_points_in.points.size(); ii++)
        {
            double vx2 = ((all_points_in.points.at(ii).vx * 0.01) * (all_points_in.points.at(ii).vx * 0.01));
            double vy2 = ((all_points_in.points.at(ii).vy * 0.01) * (all_points_in.points.at(ii).vy * 0.01));
            double vz2 = ((all_points_in.points.at(ii).vz * 0.01) * (all_points_in.points.at(ii).vz * 0.01));
            double speed = sqrt(vx2 + vy2 + vz2);

            uint ID = all_points_in.points.at(ii).id;
            if (!m_all_track.keys().contains(ID))
            {

                if(all_points_in.points.at(ii).di < 65535)
                {
                    Info info;
                    // info.points.push_back(all_points_in.points.at(ii));
                    info.points.push_back(all_points_in.points.at(ii));
                    info.refreshTime = QDateTime::currentMSecsSinceEpoch();
                    info.startTime = info.refreshTime;
                    info.x.push_back(all_points_in.points.at(ii).x * 0.01); //单位: m
                    info.y.push_back(all_points_in.points.at(ii).y * 0.01); //单位: m
                    info.z.push_back(all_points_in.points.at(ii).z * 0.01); //单位: m
                    info.v.push_back(speed); //单位: m/s
                    info.t.push_back(all_points_in.points.at(ii).timestamp * 0.001); //单位: s
                    info.rcs.push_back(all_points_in.points.at(ii).rcs2 * 0.01); //单位 m^2
                    info.di.push_back(all_points_in.points.at(ii).di);
                    info.score = 0;
                    info.score_brid = 0;
                    info.longType = 0;

                    m_all_track.insert(ID, info);
                }

            }
            else
            {
                if(all_points_in.points.at(ii).di < 65535)
                {
                    m_all_track[ID].points.push_back(all_points_in.points.at(ii));
                    m_all_track[ID].refreshTime = QDateTime::currentMSecsSinceEpoch();
                    m_all_track[ID].x.push_back(all_points_in.points.at(ii).x * 0.01); //单位: m
                    m_all_track[ID].y.push_back(all_points_in.points.at(ii).y * 0.01); //单位: m
                    m_all_track[ID].z.push_back(all_points_in.points.at(ii).z * 0.01); //单位: m
                    m_all_track[ID].v.push_back(speed); //单位: m/s
                    m_all_track[ID].t.push_back(all_points_in.points.at(ii).timestamp * 0.001); //单位: s
                    m_all_track[ID].rcs.push_back(all_points_in.points.at(ii).rcs2 * 0.01); //单位 m^2
                    m_all_track[ID].di.push_back(all_points_in.points.at(ii).di);
                }
                else
                {
                    if(m_all_track[ID].longType > 0 && m_all_track[ID].score >= SHOW_SCORE)
                    {
                        m_all_track[ID].refreshTime = QDateTime::currentMSecsSinceEpoch(); //无人机航机外推点，时间更新

                    }
                }

            }


            int tempType;
            int max_vote = 0;
            if(m_all_track[ID].points.size() < QUICK_HISTORY_POINT_NUM)
            {
                // qDebug() << "A---ID: " << ID << " len: " << m_all_track[ID].v.size();
                m_all_track[ID].targetType = TGT_UNDETECTED;
            }
            else if((m_all_track[ID].points.size() >= QUICK_HISTORY_POINT_NUM) && ( m_all_track[ID].points.size() <= HISTORY_POINT_NUM-1))
            {
                // qDebug() << "B---ID: " << ID << " len: " << m_all_track[ID].v.size();
                std::vector<double> features_quick = Func_cal_features(m_all_track[ID].t, m_all_track[ID].x, m_all_track[ID].y, m_all_track[ID].z, m_all_track[ID].v, m_all_track[ID].rcs,m_all_track[ID].di); //根据航迹数据计算特征

                double sum_v = 0;
                for(int i=0; i<m_all_track[ID].v.size(); i++)
                {
                    sum_v = sum_v + m_all_track[ID].v.at(i);
                }
                double mean_v = sum_v/m_all_track[ID].v.size();
                double cur_height = m_all_track[ID].z.back();

                // tempType = predict(features_quick, false, 0, 0, max_vote);
                tempType = predict(features_quick, true, cur_height, mean_v, max_vote);

                double temp_Z = hight_mad_filtered_mean(m_all_track[ID].z, 3.0);
                //m_all_track[ID].z.back() = temp_Z;
                all_points_in.points.at(ii).z = temp_Z * 100.0;

                if(tempType==TGT_DRONE)
                {
                    if(max_vote >= 70)
                    {
                        m_all_track[ID].score = m_all_track[ID].score + 1;//3;//5;//10;
                    }
                    else
                    {
                        tempType = TGT_UNDETECTED;
                    }
                    //调试信息
                    // all_points_in.points.at(ii).beam = max_vote;
                }
                // else if(tempType == TGT_BIRD)
                // {
                //     all_points_in.points.at(ii).beam = max_vote + 1000;
                // }
                // else
                // {
                //     all_points_in.points.at(ii).beam = max_vote + 10000;
                // }


                m_all_track[ID].targetType = tempType;
            }
            else
            {
                // qDebug() << "C---ID: " << ID << " len: " << m_all_track[ID].v.size();
                while(m_all_track[ID].points.size() > HISTORY_POINT_NUM)
                {
                    m_all_track[ID].points.erase(m_all_track[ID].points.begin());

                    m_all_track[ID].t.erase(m_all_track[ID].t.begin());
                    m_all_track[ID].x.erase(m_all_track[ID].x.begin());
                    m_all_track[ID].y.erase(m_all_track[ID].y.begin());
                    m_all_track[ID].z.erase(m_all_track[ID].z.begin());
                    m_all_track[ID].v.erase(m_all_track[ID].v.begin());
                    m_all_track[ID].rcs.erase(m_all_track[ID].rcs.begin());
                    m_all_track[ID].di.erase(m_all_track[ID].di.begin());
                }
                // qDebug() << "D---ID: " << ID << " len: " << m_all_track[ID].v.size();
                std::vector<double> features = Func_cal_features(m_all_track[ID].t, m_all_track[ID].x, m_all_track[ID].y, m_all_track[ID].z, m_all_track[ID].v, m_all_track[ID].rcs,m_all_track[ID].di); //根据航迹数据计算特征
                // double sum_height = 0;
                double sum_v = 0;
                for(int i=0; i<m_all_track[ID].v.size(); i++)
                {
                    // sum_height = sum_height + m_all_track[ID].z.at(i);
                    sum_v = sum_v + m_all_track[ID].v.at(i);
                }
                // double mean_height = sum_height/m_all_track[ID].z.size();
                double mean_v = sum_v/m_all_track[ID].v.size();
                double cur_height = m_all_track[ID].z.back();
                // m_all_track[ID].targetType = predict(features, false, mean_height, mean_v);


                // max_vote = ID;
                tempType = predict(features, true, cur_height, mean_v, max_vote);


                double temp_Z = hight_mad_filtered_mean(m_all_track[ID].z, 3.0);
                //m_all_track[ID].z.back() = temp_Z;
                all_points_in.points.at(ii).z = temp_Z * 100.0;


                // tempType = predict(features, false, 0, 0, max_vote);




                if(tempType==TGT_DRONE || tempType==TGT_BIRD)
                {
                    //qDebug() << m_all_track[ID].x.size();
                    double dis_start = sqrt(m_all_track[ID].x.at(0) * m_all_track[ID].x.at(0) +  m_all_track[ID].y.at(0) * m_all_track[ID].y.at(0));
                    double dis_end = sqrt(m_all_track[ID].x.at(m_all_track[ID].x.size()-1) * m_all_track[ID].x.at(m_all_track[ID].x.size()-1) +  m_all_track[ID].y.at(m_all_track[ID].y.size()-1) * m_all_track[ID].y.at(m_all_track[ID].y.size()-1));;

                    if(tempType == TGT_DRONE)
                    {
                        // m_all_track[ID].score++;

                        if(dis_start > dis_end) //靠近
                        {
                            if(max_vote >= 90)
                            {
                                m_all_track[ID].score = m_all_track[ID].score + 3;//3;//5;//10;
                                m_all_track[ID].score_brid = m_all_track[ID].score_brid - 1;//2;//3;//8;
                                if(m_all_track[ID].score_brid < 0)
                                {
                                    m_all_track[ID].score_brid = 0;
                                }
                            }
                            else if(max_vote >= 55)
                            {
                                m_all_track[ID].score = m_all_track[ID].score + 2;//3;//5;//10;
                                // m_all_track[ID].score_brid = m_all_track[ID].score_brid - 1;//2;//3;//8;
                                // if(m_all_track[ID].score_brid < 0)
                                // {
                                //     m_all_track[ID].score_brid = 0;
                                // }
                            }
                        }
                        else //远离
                        {
                            if(max_vote >= 90)
                            {
                                m_all_track[ID].score = m_all_track[ID].score + 2;//3;//5;//10;
                                m_all_track[ID].score_brid = m_all_track[ID].score_brid - 1;//2;//3;//8;
                                if(m_all_track[ID].score_brid < 0)
                                {
                                    m_all_track[ID].score_brid = 0;
                                }
                            }
                            else if(max_vote >= 60)
                            {
                                m_all_track[ID].score = m_all_track[ID].score + 1;
                                // m_all_track[ID].score_brid = m_all_track[ID].score_brid - 1;
                                // if(m_all_track[ID].score_brid < 0)
                                // {
                                //     m_all_track[ID].score_brid = 0;
                                // }
                            }


                        }

                        //调试信息
                        // all_points_in.points.at(ii).beam = max_vote;

                    }
                    else if(tempType == TGT_BIRD)
                    {
                        if(dis_start > dis_end) //靠近
                        {
                            if(max_vote >= 75)
                            {
                                m_all_track[ID].score_brid = m_all_track[ID].score_brid + 1;
                            }
                        }
                        else //远离
                        {
                            if(max_vote >= 95)
                            {
                                m_all_track[ID].score_brid = m_all_track[ID].score_brid + 2;
                                m_all_track[ID].score = m_all_track[ID].score - 1;
                                if(m_all_track[ID].score < 0)
                                {
                                    m_all_track[ID].score = 0;
                                }
                            }
                            else if(max_vote > 75)
                            {
                                m_all_track[ID].score_brid++;
                            }
                        }



                        //调试信息
                        // all_points_in.points.at(ii).beam = max_vote + 1000;

                    }



                    if(m_all_track[ID].score >= m_all_track[ID].score_brid && (m_all_track[ID].score > 0))
                    {
                        m_all_track[ID].targetType = TGT_DRONE;
                    }
                    else
                    {
                        if(m_all_track[ID].score_brid > (m_all_track[ID].score * 4))
                        {
                            m_all_track[ID].targetType = TGT_BIRD;
                        }
                        else
                        {
                            m_all_track[ID].targetType = tempType;
                        }



                    }

                }
                else
                {
                    if((m_all_track[ID].score >= m_all_track[ID].score_brid) && (m_all_track[ID].score >= 20))
                    {
                        m_all_track[ID].targetType = TGT_DRONE;
                    }
                    else
                    {
                        m_all_track[ID].targetType = tempType;
                    }




                    //调试信息
                    // all_points_in.points.at(ii).beam = max_vote + 10000;
                }

                // m_all_track[ID].targetType = tempType;

            }

            all_points_in.points.at(ii).object =  m_all_track[ID].targetType;


            //调试信息
            // if(m_all_track[ID].score < 100 && m_all_track[ID].score_brid < 100)
            // {
            //     all_points_in.points.at(ii).ri = m_all_track[ID].score * 100 + m_all_track[ID].score_brid;
            // }
            // else if(m_all_track[ID].score >= 100 && m_all_track[ID].score_brid < 100)
            // {
            //     int temp_score = m_all_track[ID].score - 100;
            //     all_points_in.points.at(ii).ri = 10000 + temp_score * 100 + m_all_track[ID].score_brid;
            // }
            // else if(m_all_track[ID].score_brid >= 100 && m_all_track[ID].score < 100)
            // {
            //     int temp_score_brid = m_all_track[ID].score_brid - 100;
            //     all_points_in.points.at(ii).ri = 20000 + m_all_track[ID].score * 100 + temp_score_brid;
            // }
            // else
            // {
            //     int temp_score = m_all_track[ID].score - 100;
            //     int temp_score_brid = m_all_track[ID].score_brid - 100;
            //     all_points_in.points.at(ii).ri = 30000 + temp_score * 100 + temp_score_brid;
            // }

            // if(all_points_in.points.at(ii).object == TGT_DRONE)
            // {
            //     if(m_all_track[ID].score < 100 && m_all_track[ID].score_brid < 100)
            //     {
            //         all_points_in.points.at(ii).ri = m_all_track[ID].score * 100 + m_all_track[ID].score_brid;
            //     }
            //     else if(m_all_track[ID].score >= 100 && m_all_track[ID].score_brid < 100)
            //     {
            //         int temp_score = m_all_track[ID].score - 100;
            //         all_points_in.points.at(ii).ri = 10000 + temp_score * 100 + m_all_track[ID].score_brid;
            //     }
            //     else if(m_all_track[ID].score_brid >= 100 && m_all_track[ID].score < 100)
            //     {
            //         int temp_score_brid = m_all_track[ID].score_brid - 100;
            //         all_points_in.points.at(ii).ri = 20000 + m_all_track[ID].score * 100 + temp_score_brid;
            //     }
            //     else
            //     {
            //         int temp_score = m_all_track[ID].score - 100;
            //         int temp_score_brid = m_all_track[ID].score_brid - 100;
            //         all_points_in.points.at(ii).ri = 30000 + temp_score * 100 + temp_score_brid;
            //     }
            //     // all_points_in.points.at(ii).ri = m_all_track[ID].score * 100 + m_all_track[ID].score_brid;
            //     // all_points_in.points.at(ii).beam = max_vote;
            // }
            // else if(all_points_in.points.at(ii).object == TGT_BIRD)
            // {
            //     if(m_all_track[ID].score < 100 && m_all_track[ID].score_brid < 100)
            //     {
            //         all_points_in.points.at(ii).ri = m_all_track[ID].score * 100 + m_all_track[ID].score_brid;
            //     }
            //     else if(m_all_track[ID].score >= 100 && m_all_track[ID].score_brid < 100)
            //     {
            //         int temp_score = m_all_track[ID].score - 100;
            //         all_points_in.points.at(ii).ri = 10000 + temp_score * 100 + m_all_track[ID].score_brid;
            //     }
            //     else if(m_all_track[ID].score_brid >= 100 && m_all_track[ID].score < 100)
            //     {
            //         int temp_score_brid = m_all_track[ID].score_brid - 100;
            //         all_points_in.points.at(ii).ri = 20000 + m_all_track[ID].score * 100 + temp_score_brid;
            //     }
            //     else
            //     {
            //         int temp_score = m_all_track[ID].score - 100;
            //         int temp_score_brid = m_all_track[ID].score_brid - 100;
            //         all_points_in.points.at(ii).ri = 30000 + temp_score * 100 + temp_score_brid;
            //     }
            //     // all_points_in.points.at(ii).beam = max_vote + 100;
            // }
            // else
            // {
            //     all_points_in.points.at(ii).ri = 0;
            //     // all_points_in.points.at(ii).beam = max_vote;
            // }

        }

    }

}

radar::net::UploadTrackPointsMessageV3 RandomForest::updateData(radar::net::UploadTrackPointsMessageV3 data)
{
    all_points_in = data;
    getHistoryPoint();
    return all_points_in;
}

// 计算特征值函数
std::vector<double> RandomForest::Func_cal_features(const std::vector<double>& t,const std::vector<double>& x,const std::vector<double>& y,
                                                    const std::vector<double>& z,const std::vector<double>& v,const std::vector<double>& rcs,const std::vector<double>& di)
{

    // 初始化输出向量，包含6个特征值
    std::vector<double> features(6, 0.0);

    int N = x.size();
    if (N < 3) {
        return features; // 返回默认值
    }

    const double Thr_h = 90.0; // 航向阈值（单位：°）
    const double max_v = 100.0; // 速度最值（单位：m/s）
    const double PI = 3.14159265358979323846;

    //  多普勒速度标准差
    double di_sum = 0.0;
    double di_max = 0;
    double di_min = 0;
    int di_max_idx = 0;
    int di_min_idx = 0;
    for (int i = 0; i < N; i++)
    {
        if(i == 0)
        {
            di_max = di[i];
            di_min = di[i];
        }
        else
        {
            if(di[i] > di_max)
            {
                di_max = di[i];
                di_max_idx = i;
            }

            if(di[i] <= di_min)
            {
                di_min = di[i];
                di_min_idx = i;
            }

        }

        di_sum += di[i];
    }
    di_sum = di_sum - di_min;
    di_sum = di_sum - di_max;
    double di_mean = di_sum / (N-2);

    double di_variance = 0.0;
    for (int i = 0; i < N; i++)
    {
        if((i == di_max_idx) || (i == di_min_idx))
        {
            continue;
        }
        di_variance += (di[i] - di_mean) * (di[i] - di_mean);
    }
    double std_di = std::sqrt(di_variance / (N-2));
    features[0] = std_di;

    //  高度标准差
    double z_sum = 0.0;
    for (int i = 0; i < N; i++) {
        z_sum += z[i];
    }
    double z_mean = z_sum / N;

    double z_variance = 0.0;
    for (int i = 0; i < N; i++) {
        z_variance += (z[i] - z_mean) * (z[i] - z_mean);
    }
    double std_height = std::sqrt(z_variance / (N-1));
    // features[8] = std_height;
    features[5] = std_height;

    // 计算差分
    std::vector<double> dif_x(N-1), dif_y(N-1), dif_z(N-1), dif_t(N-1), dif_v(N-1);
    for (int i = 0; i < N-1; i++) {
        dif_x[i] = x[i+1] - x[i];
        dif_y[i] = y[i+1] - y[i];
        dif_z[i] = z[i+1] - z[i];
        dif_t[i] = t[i+1] - t[i];
        dif_v[i] = v[i+1] - v[i];
    }

    //  曲率标准差
    std::vector<double> heading(N-1);
    for (int i = 0; i < N-1; i++) {
        heading[i] = std::atan2(dif_x[i], dif_y[i]) * 180.0 / PI;

        // 调整航向到[-180, 180]范围
        while (heading[i] > 180.0) heading[i] -= 360.0;
        while (heading[i] < -180.0) heading[i] += 360.0;
    }

    std::vector<double> dif_head(N-2);
    for (int i = 0; i < N-2; i++) {
        dif_head[i] = heading[i+1] - heading[i];

        // 调整航向差到[-180, 180]范围
        while (dif_head[i] > 180.0) dif_head[i] -= 360.0;
        while (dif_head[i] < -180.0) dif_head[i] += 360.0;
    }

    // 计算距离
    std::vector<double> dis_r(N-1);
    for (int i = 0; i < N-1; i++) {
        dis_r[i] = std::sqrt(dif_x[i]*dif_x[i] + dif_y[i]*dif_y[i] + dif_z[i]*dif_z[i]);
    }

    // 计算曲率
    std::vector<double> curvature(N-2);
    for (int i = 0; i < N-2; i++) {
        double avg_dis = (dis_r[i] + dis_r[i+1]) / 2.0;
        if (avg_dis > 1e-10) { // 避免除以零
            curvature[i] = std::abs(dif_head[i]) / avg_dis;
        } else {
            curvature[i] = 0.0;
        }
    }

    // 计算曲率标准差
    double curv_sum = 0.0;
    for (int i = 0; i < N-2; i++) {
        curv_sum += curvature[i];
    }
    double curv_mean = curv_sum / (N-2);

    double curv_variance = 0.0;
    for (int i = 0; i < N-2; i++) {
        curv_variance += (curvature[i] - curv_mean) * (curvature[i] - curv_mean);
    }
    double std_curvature = std::sqrt(curv_variance / (N-3));
    features[1] = std_curvature;

    //  加加速度均方根
    std::vector<double> acc(N-1);
    for (int i = 0; i < N-1; i++) {
        if (dif_t[i] > 1e-10) {
            acc[i] = dif_v[i] / dif_t[i];
        } else {
            acc[i] = 0.0;
        }
    }

    std::vector<double> acc_acc(N-2);
    for (int i = 0; i < N-2; i++) {
        if (dif_t[i] > 1e-10) {
            acc_acc[i] = (acc[i+1] - acc[i]) / dif_t[i];
        } else {
            acc_acc[i] = 0.0;
        }
    }

    double acc_acc_sq_sum = 0.0;
    for (int i = 0; i < N-2; i++) {
        acc_acc_sq_sum += acc_acc[i] * acc_acc[i];
    }
    double rms_acc_acc = std::sqrt(acc_acc_sq_sum / (N-2));
    // features[6] = rms_acc_acc;
    features[4] = rms_acc_acc;

    //  RCS均值
    double rcs_sum = 0.0;
    double rcs_min = 0;
    double rcs_max = 0;
    int rcs_min_idx = 0;
    int rcs_max_idx = 0;
    for (int i = 0; i < N; i++)
    {
        if(i == 0)
        {
            rcs_min = rcs[i];
            rcs_max = rcs[i];
        }
        else
        {
            if(rcs_max < rcs[i])
            {
                rcs_max = rcs[i];
                rcs_max_idx = i;
            }

            if(rcs_min >= rcs[i])
            {
                rcs_min = rcs[i];
                rcs_min_idx = i;
            }
        }

        rcs_sum += rcs[i];
    }

    rcs_sum = rcs_sum - rcs_min;
    rcs_sum = rcs_sum - rcs_max;

    double mean_rcs = rcs_sum / (N-2);
    // features[5] = mean_rcs;
    features[3] = mean_rcs;

    //  机动因子
    double h_sum = 0.0;
    for (int i = 0; i < N-1; i++) {
        h_sum += heading[i];
    }
    double h_mean = h_sum / (N-1);

    double delta_h2_sum = 0.0;
    for (int i = 0; i < N-1; i++) {
        double delta_h = std::abs(heading[i] - h_mean);
        if (delta_h > Thr_h) {
            delta_h = std::abs(delta_h - 360.0);
        }
        delta_h2_sum += delta_h * delta_h;
    }
    double h_std = std::sqrt(delta_h2_sum / (N-1));

    double v_sum = 0.0;
    for (int i = 0; i < N; i++) {
        v_sum += v[i];
    }
    double v_mean = v_sum / N;

    double normal_v = v_mean / max_v;
    double normal_h = h_std / Thr_h;
    double maneuver_factor = (normal_h > 1e-10) ? (normal_v / normal_h) : 0.0;
    // features[3] = maneuver_factor;
    features[2] = maneuver_factor;

    return features;
}

double RandomForest::hight_mad_filtered_mean(std::vector<double> working_data, double mad_threshold)
{
    //if (echos_.empty())
    //    return 0.0;
    //
    //std::vector<double> working_data;
    //for (auto it = echos_.begin(); it != echos_.end(); ++it)
    //{
    //    //working_data.push_back(10 * log10(it->raw().rcs));
    //	working_data.push_back((it->raw().z));
    //}

    // 计算中位数
    std::vector<double> sorted_data = working_data;
    std::sort(sorted_data.begin(), sorted_data.end());
    double median = sorted_data[sorted_data.size() / 2];
    if (sorted_data.size() % 2 == 0) {
        median = (sorted_data[sorted_data.size() / 2 - 1] +
                  sorted_data[sorted_data.size() / 2]) /
                 2.0;
    }

    // 计算绝对偏差
    std::vector<double> absolute_deviations;
    for (double val : working_data) {
        absolute_deviations.push_back(std::abs(val - median));
    }

    // 计算MAD (中位数绝对偏差)
    std::sort(absolute_deviations.begin(), absolute_deviations.end());
    double mad = absolute_deviations[absolute_deviations.size() / 2];
    if (absolute_deviations.size() % 2 == 0) {
        mad = (absolute_deviations[absolute_deviations.size() / 2 - 1] +
               absolute_deviations[absolute_deviations.size() / 2]) /
              2.0;
    }

    // 对于正态分布，MAD ≈ 0.6745 * σ，因此需要调整
    double adjusted_mad = mad * 1.4826;

    // 剔除离群点
    std::vector<double> filtered_data;
    double threshold = mad_threshold * adjusted_mad;

    for (double val : working_data) {
        if (std::abs(val - median) <= threshold) {
            filtered_data.push_back(val);
        }
    }

    double hight_med = 0;
    if (filtered_data.empty()) {
        // 如果所有点都被剔除，返回中位数
        //hight_med = pow(10, median / 10);
        hight_med = median;
    } else {
        // 计算剩余数据的均值
        double temp = std::accumulate(filtered_data.begin(),
                                      filtered_data.end(), 0.0) /
                      filtered_data.size();
        //hight_med = pow(10, temp / 10);
        hight_med = temp;
    }

    // if (echos_.size() < windowLen_) {
    //     std::ostringstream oss;
    //     oss << "RCS ARR = [";
    //     for (double val : working_data) {
    //         oss << val << ", ";
    //     }
    //     oss << "], median = " << median << ", MAD = " << mad
    //         << ", adjusted_MAD = " << adjusted_mad
    //         << ", filtered size = " << filtered_data.size()
    //         << ", RCS mean = " << hight_med;
    //     std::cout << oss.str() << std::endl;
    // }

    return hight_med;
}

