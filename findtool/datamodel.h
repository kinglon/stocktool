#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QString>

class Stock
{
public:
    // 股票名字
    QString m_stockName;

    // 股票代码
    QString m_stockId;

    // 股票数据完整目录
    QString m_stockPath;
};

#endif // DATAMODEL_H
