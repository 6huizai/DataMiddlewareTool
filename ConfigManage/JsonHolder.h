#ifndef JSONHOLDER_H
#define JSONHOLDER_H

#include "AIGCJson.hpp"
#include <fstream>
#include <QString>
#include <QDebug>

template <class Data>
class JsonHolder
{
public:
    explicit JsonHolder();
    explicit JsonHolder(const QString &filename);
    ~JsonHolder();

    virtual bool load();
    virtual bool save();
    virtual void reset();

    const Data &constData() const;
    Data &data();

    QString filename() const;
    void setFilename(const QString &filename);

private:
    bool readJson(const std::string& file, std::string& jsonString);
    bool writeJson(const std::string& file, const std::string& jsonString);
    template <typename T>
    bool parseJson(T& obj, const std::string& file);

protected:
    QString m_filename;
    Data m_data;
};

template <class Data>
JsonHolder<Data>::JsonHolder() {}

template <class Data>
JsonHolder<Data>::JsonHolder(const QString &filename) {
    setFilename(filename);
}

template <class Data>
JsonHolder<Data>::~JsonHolder() {}

template <class Data>
bool JsonHolder<Data>::load() {
    if (!parseJson(m_data, m_filename.toStdString())) {
        qDebug() << "Failed to parse json file: " << m_filename;
        return false;
    }
    return true;
}

template <class Data>
bool JsonHolder<Data>::save() {
    std::string errmsg;
    std::string str;
    if (!aigc::JsonHelper::ObjectToJson(m_data, str, &errmsg))
    {
        qDebug() << "Failed to save json file: " << errmsg.c_str();
        return false;
    }
    writeJson(m_filename.toStdString(), str);
    return true;
}

template <class Data>
void JsonHolder<Data>::reset() {
}

template <class Data>
const Data &JsonHolder<Data>::constData() const {
    return m_data;
}

template <class Data>
Data &JsonHolder<Data>::data() {
    return m_data;
}

template <class Data>
QString JsonHolder<Data>::filename() const
{
    return m_filename;
}

template <class Data>
void JsonHolder<Data>::setFilename(const QString &filename)
{
    m_filename = filename;
}

template <class Data>
bool JsonHolder<Data>::readJson(const std::string& path, std::string& jstr)
{
    //打开文件，以二进制方式打开文件
    std::ifstream is(path, std::ios::binary);
    if (!is.is_open()) {
        return false;
    }
    jstr = std::string((std::istreambuf_iterator<char>(is)),
                        std::istreambuf_iterator<char>());
    is.close();
    return true;
}

template <class Data>
bool JsonHolder<Data>::writeJson(const std::string& path, const std::string& jstr)
{
    std::ofstream os(path, std::ios::trunc);
    if(!os.is_open()) {
        return false;
    }
    os << jstr;
    os.close();
    return true;
}

template <class Data>
template <class T>
bool JsonHolder<Data>::parseJson(T& obj, const std::string& file)
{
    std::string jstr;
    if (!readJson(file, jstr))
    {
        return false;
    }
    return aigc::JsonHelper::JsonToObject(obj, jstr);
}

#endif // JSONHOLDER_H
