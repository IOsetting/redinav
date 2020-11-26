#pragma once
#include <QList>
#include <QObject>
#include <QVariant>
#include <QVector>

namespace MainTree {

class TreeItem : public QObject {

    Q_OBJECT

public:
    // Ctor and Dtor
    TreeItem(const QMap<int, QVariant>& data, TreeItem* parent = 0);
    ~TreeItem();

    // Get a pointer to a child at position "number" (if any)
    TreeItem* child(int number);

    int childCount() const;
    QVariant data(int role) const;
    QMap<int, QVariant> itemData() const;
    bool insertChildren(int position, int count);
    TreeItem* parent();
    bool removeChildren(int position, int count);
    int position() const;
    bool setData(int role, const QVariant& value);

    bool canFetchMore();
    void fetchMore();
    bool setItemData(QMap<int, QVariant>& itemData);

    bool isLocked() const;
    void lock();
    void unlock();

private:
    QList<TreeItem*> m_childItems;
    QMap<int, QVariant> m_itemData;
    TreeItem* m_parentItem;
    bool m_locked;
};
}
