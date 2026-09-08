#ifndef UICOMPONENTFACTORY_HPP
#define UICOMPONENTFACTORY_HPP

#include <QWidget>
#include <QListWidgetItem>
#include <QString>
#include <functional>
#include "StyleManager.hpp"
#include "../model/DatabaseTypes.hpp"

class UIComponentFactory
{
public:
    explicit UIComponentFactory(StyleManager& styleManager);

    QWidget* createAssignmentWidget(const SubjectInfo& assignment,
                                    std::function<void()> onViewTasks);

    QWidget* createTaskWidget(const TaskInfo& task,
                              std::function<void(QPushButton*)> onUpload,
                              std::function<void()> onEdit,
                              std::function<void()> onDelete,
                              std::function<void(QPushButton*)> onSelect);

    void applyUniformStyle(QWidget* widget, QListWidgetItem* item);

    QPushButton* createButton(const QString& text, const QString& color,
                              const QString& textColor);
private:

    StyleManager& mStyleManager;

    static constexpr int ITEM_HEIGHT = 80;
    static constexpr int ITEM_MARGIN = 10;
    static constexpr int BUTTON_WIDTH = 80;
    static constexpr int BUTTON_HEIGHT = 32;
};

#endif
