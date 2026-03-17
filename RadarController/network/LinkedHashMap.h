/*
 * LinkedHashMap.h
 *
 *  Created on: Nov 14, 2015
 *      Author: root
 */

#ifndef LINKEDHASHMAP_H_
#define LINKEDHASHMAP_H_

#include <list>
#include <QHash>      // Qt替代boost的哈希表
#include <QMutex>

namespace moss {
template<class T1, class T2>
class LinkedHashMap {
public:
    LinkedHashMap() {
    }
    bool push_front(const T1& key, const T2& value) {
        QMutexLocker locker(&mutex_);
        typename NewMap::const_iterator mapIt = map_.find(key);
        if (mapIt != map_.constEnd()) {  // Qt的find返回const_iterator
            // 键已存在
            return false;
        }
        list_.push_front(std::make_pair(key, value));
        map_.insert(key, list_.begin());  // Qt的insert直接插入键值对
        return true;
    }

    T2* find(const T1& key) {
        QMutexLocker locker(&mutex_);
        typename NewMap::const_iterator mapIt = map_.find(key);
        if (mapIt == map_.constEnd()) {
            return nullptr;
        }
        typename NewList::iterator listIt = mapIt.value();  // Qt迭代器用value()获取值
        return &((*listIt).second);
    }

    void remove(const T1& key) {
        QMutexLocker locker(&mutex_);
        remove_(key);
    }

    T2& back() {
        QMutexLocker locker(&mutex_);
        typename NewList::reference val = list_.back();
        return (val.second);
    }

    void pop_back() {
        QMutexLocker locker(&mutex_);
        if (!list_.empty()) {
            typename NewList::reference val = list_.back();
            T1 key = val.first;
            remove_(key);
        }
    }

    void updateKey(const T1& key) {
        QMutexLocker locker(&mutex_);
        typename NewMap::iterator mapIt = map_.find(key);
        if (mapIt == map_.end()) {
            return;
        }
        typename NewList::iterator listIt = mapIt.value();  // 获取值
        list_.push_front(*listIt);
        map_[key] = list_.begin();  // 更新为新迭代器
        list_.erase(listIt);
    }

private:
    void remove_(const T1& key) {
        typename NewMap::iterator mapIt = map_.find(key);
        if (mapIt == map_.end()) {
            return;
        }
        typename NewList::iterator listIt = mapIt.value();  // 获取链表迭代器
        list_.erase(listIt);
        map_.erase(mapIt);  // Qt的erase接受迭代器
    }

private:
    typedef std::list<std::pair<T1, T2>> NewList;
    typedef QHash<T1, typename NewList::iterator> NewMap;  // 使用QHash替代

public:
    typedef typename NewMap::size_type size_type;
    size_type size() {
        return map_.size();
    }

private:
    NewList list_;
    NewMap map_;
    QMutex mutex_;
};

} /* namespace moss */

#endif /* LINKEDHASHMAP_H_ */
