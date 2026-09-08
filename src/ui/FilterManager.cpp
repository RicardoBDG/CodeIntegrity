#include "FilterManager.hpp"
#include <QLabel>

void FilterManager::filterListWidget(QListWidget* listWidget, const QString& filterText)
{
    if (!listWidget) return;

    for (int i = 0; i < listWidget->count(); ++i)
    {
        auto item = listWidget->item(i);
        auto widget = listWidget->itemWidget(item);

        bool found = filterText.isEmpty() || matchesFilter(widget, filterText);
        item->setHidden(!found);
    }
}

bool FilterManager::matchesFilter(QWidget* widget, const QString& filterText)
{
    if (!widget) return false;

    for (auto label : widget->findChildren<QLabel*>())
    {
        if (label->text().toLower().contains(filterText.toLower()))
        {
            return true;
        }
    }
    return false;
}
