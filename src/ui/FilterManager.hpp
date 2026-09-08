#ifndef FILTERMANAGER_HPP
#define FILTERMANAGER_HPP

#include <QListWidget>
#include <QString>

class FilterManager
{
    public:
        static void filterListWidget(QListWidget* listWidget, const QString& filterText);

    private:
        static bool matchesFilter(QWidget* widget, const QString& filterText);
};

#endif
