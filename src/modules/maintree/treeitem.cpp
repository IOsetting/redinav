#include "treeitem.h"
#include "roles.h"
#include <QStringList>

using namespace MainTree;

TreeItem::TreeItem(const QMap<int, QVariant>& data, TreeItem* parent)
{
    m_parentItem = parent;
    m_itemData = data;
    m_locked = false;
}

TreeItem::~TreeItem()
{
    qDeleteAll(m_childItems);
}

TreeItem* TreeItem::child(int number)
{
    if (childCount() > 0)
        return m_childItems.value(number);
    return nullptr;
}

int TreeItem::childCount() const
{
    return m_childItems.count();
}

int TreeItem::position() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

QVariant TreeItem::data(int role) const
{
    return m_itemData[role];
}

QMap<int, QVariant> TreeItem::itemData() const
{
    return m_itemData;
}

/**
 * Insert <count> empty ietms at <position>
 *
 * @brief TreeItem::insertChildren
 * @param position
 * @param count
 * @return
 */
bool TreeItem::insertChildren(int position, int count)
{
    if (position < 0 || position > m_childItems.size())
        return false;

    QMap<int, QVariant> data;

    for (int row = 0; row < count; ++row) {
        TreeItem* item = new TreeItem(data, this);
        m_childItems.insert(position, item);
    }

    return true;
}

TreeItem* TreeItem::parent()
{
    return m_parentItem;
}

bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete m_childItems.takeAt(position);

    return true;
}

bool TreeItem::setData(int role, const QVariant& value)
{
    m_itemData[role] = value;
    return true;
}

bool TreeItem::setItemData(QMap<int, QVariant>& itemData)
{
    m_itemData = itemData;
    return true;
}

bool TreeItem::canFetchMore()
{
    return false;
}

void TreeItem::fetchMore()
{
    return;
}

bool TreeItem::isLocked() const
{
    return m_locked;
}

void TreeItem::lock()
{
    m_locked = true;
}

void TreeItem::unlock()
{
    m_locked = false;
}
