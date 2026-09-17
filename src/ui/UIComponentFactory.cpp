#include "UIComponentFactory.hpp"
#include "../model/DatabaseTypes.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSize>

UIComponentFactory::UIComponentFactory(StyleManager& styleManager)
    : mStyleManager(styleManager) {}

QPushButton* UIComponentFactory::createButton(const QString& text,
                                              const QString& color,
                                              const QString& textColor)
{
    auto btn = new QPushButton(text);

    QFontMetrics fm(btn->font());
    int textWidth = fm.horizontalAdvance(text);
    int buttonWidth = qMax(BUTTON_WIDTH, textWidth + 40);

    btn->setFixedWidth(buttonWidth);
    btn->setFixedHeight(BUTTON_HEIGHT);
    btn->setStyleSheet(mStyleManager.getButtonStyle(color, textColor));
    return btn;
}


QWidget* UIComponentFactory::createAssignmentWidget(const SubjectInfo& assignment,
                                                    std::function<void()> onViewTasks)
{
    auto widget = new QWidget();
    widget->setFixedHeight(ITEM_HEIGHT);

    auto mainLayout = new QHBoxLayout(widget);
    mainLayout->setContentsMargins(ITEM_MARGIN, ITEM_MARGIN, ITEM_MARGIN, ITEM_MARGIN);
    mainLayout->setSpacing(15);

    auto infoContainer = new QWidget();
    auto infoLayout = new QVBoxLayout(infoContainer);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(5);

    auto nombreLabel = new QLabel(assignment.nombre);
    nombreLabel->setStyleSheet(mStyleManager.getLabelTitleStyle());
    nombreLabel->setFixedHeight(25);

    auto descripcionLabel = new QLabel(assignment.descripcion);
    descripcionLabel->setStyleSheet(mStyleManager.getLabelDescriptionStyle());
    descripcionLabel->setFixedHeight(20);
    descripcionLabel->setWordWrap(true);

    infoLayout->addWidget(nombreLabel);
    infoLayout->addWidget(descripcionLabel);
    infoLayout->addStretch();

    auto verTareasBtn = createButton("Ver Tareas", "#4CAF50", "#ffffff");
    QAbstractButton::connect(verTareasBtn, &QPushButton::clicked, onViewTasks);

    mainLayout->addWidget(infoContainer, 1);
    mainLayout->addWidget(verTareasBtn, 0, Qt::AlignRight | Qt::AlignVCenter);

    widget->setStyleSheet(mStyleManager.getItemWidgetStyle());

    return widget;
}

QWidget* UIComponentFactory::createTaskWidget(const TaskInfo& task,
                                              std::function<void(QPushButton*)> onUpload,
                                              std::function<void()> onEdit,
                                              std::function<void()> onDelete,
                                              std::function<void(QPushButton*)> onSelect)
{
    auto widget = new QWidget();
    widget->setFixedHeight(ITEM_HEIGHT);

    auto mainLayout = new QHBoxLayout(widget);
    mainLayout->setContentsMargins(ITEM_MARGIN, ITEM_MARGIN, ITEM_MARGIN, ITEM_MARGIN);
    mainLayout->setSpacing(15);

    auto infoContainer = new QWidget();
    auto infoLayout = new QVBoxLayout(infoContainer);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->setSpacing(5);

    auto tituloLabel = new QLabel(task.titulo);
    tituloLabel->setStyleSheet(mStyleManager.getLabelTitleStyle());
    tituloLabel->setFixedHeight(25);

    auto descripcionLabel = new QLabel(task.descripcion);
    descripcionLabel->setStyleSheet(mStyleManager.getLabelDescriptionStyle());
    descripcionLabel->setFixedHeight(20);
    descripcionLabel->setWordWrap(true);

    infoLayout->addWidget(tituloLabel);
    infoLayout->addWidget(descripcionLabel);
    infoLayout->addStretch();

    auto botonesContainer = new QWidget();
    auto botonesLayout = new QHBoxLayout(botonesContainer);
    botonesLayout->setContentsMargins(0, 0, 0, 0);
    botonesLayout->setSpacing(8);

    auto subirBtn = new QPushButton("Subir");
    // Nombre de objeto fijo para que Teacher pueda recuperar con findChildren()
    // todos los botones "Subir" visibles tras reconstruir la lista de tareas
    // (ver Teacher::cargarTareasAsignatura), sin tener que devolver el puntero
    // de cada fila por otra vía.
    subirBtn->setObjectName(QStringLiteral("btnSubir"));
    subirBtn->setFixedSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    subirBtn->setStyleSheet(mStyleManager.getDisabledButtonStyle("#4a4a4a", "#888888"));
    subirBtn->setEnabled(false);

    auto editarBtn = createButton("Editar", "#ffc107", "#000000");
    auto eliminarBtn = createButton("Eliminar", "#dc3545", "#ffffff");
    auto seleccionarBtn = createButton("Seleccionar", "#17a2b8", "#ffffff");

    QAbstractButton::connect(subirBtn, &QPushButton::clicked, [onUpload, subirBtn]() {
        onUpload(subirBtn);
    });

    QAbstractButton::connect(editarBtn, &QPushButton::clicked, onEdit);
    QAbstractButton::connect(eliminarBtn, &QPushButton::clicked, onDelete);

    QAbstractButton::connect(seleccionarBtn, &QPushButton::clicked, [onSelect, subirBtn, this]()
                             {
        subirBtn->setStyleSheet(mStyleManager.getButtonStyle("#6f42c1", "#ffffff"));
        subirBtn->setEnabled(true);

        onSelect(subirBtn);
    });

    botonesLayout->addWidget(subirBtn);
    botonesLayout->addWidget(editarBtn);
    botonesLayout->addWidget(eliminarBtn);
    botonesLayout->addWidget(seleccionarBtn);

    mainLayout->addWidget(infoContainer, 1);
    mainLayout->addWidget(botonesContainer, 0, Qt::AlignRight | Qt::AlignVCenter);

    widget->setStyleSheet(mStyleManager.getItemWidgetStyle());

    return widget;
}


void UIComponentFactory::applyUniformStyle(QListWidgetItem* item)
{
    if (item)
    {
        item->setSizeHint(QSize(0, ITEM_HEIGHT + 4));
    }
}
