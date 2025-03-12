#ifndef STOCKDATAMANAGER_H
#define STOCKDATAMANAGER_H

#include <QString>

class StockDataManager
{
protected:
    StockDataManager();

public:
    static StockDataManager* getInstance();

    // 判断有没结果数据
    bool hasResultData() { return !m_result.isEmpty(); }

    bool saveResult(const QString& savePath);

private:
    QString m_result;
};

#endif // STOCKDATAMANAGER_H
