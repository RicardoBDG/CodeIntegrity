#include "Teacher.hpp"
#include "../model/SubmissionManager.hpp"
#include "../model/AnalysisCoordinator.hpp"
#include "UIComponentFactory.hpp"
#include "FilterManager.hpp"
#include "../model/DataBaseManager.hpp"
#include "StyleManager.hpp"
#include "../model/Logger.hpp"

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QFrame>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QFormLayout>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <memory>

Teacher::Teacher(int userId, const QString& email, QWidget *parent)
    : QMainWindow(parent)
    , mUserId(userId)
    , mEmailProfesor(email)
    , mSubmissionManager(new SubmissionManager(this))
    , mAnalysisCoordinator(new AnalysisCoordinator(this))
{
    setWindowTitle("CodeIntegrity - Profesor");
    setMinimumSize(800, 1050);
    setWindowIcon(QIcon(":/icons/app_icon.png"));

    mDb           = DataBaseManager::getInstance();
    mStyleManager = new StyleManager();
    mUIFactory    = std::make_unique<UIComponentFactory>(*mStyleManager);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    setupUI();
    setupLayout();
    applyStyles();
    setupConnections();
    setupAnalysisTools();
    cargarAsignaturasProfesor();

    Logger::log(Logger::INFO, "Ventana de Teacher inicializada");
}

Teacher::~Teacher()
{
    delete mStyleManager;
    Logger::log(Logger::INFO, "Ventana de Teacher cerrada");
}

bool Teacher::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick)
    {
        QListWidget* lists[] = { mAsignaturasListWidget, mTareasListWidget };
        for (QListWidget* list : lists)
        {
            if (obj == list->viewport())
            {
                QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
                QListWidgetItem* item = list->itemAt(mouseEvent->pos());
                if (item && item->isSelected())
                {
                    list->clearSelection();
                    return true;
                }
                break;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void Teacher::closeEvent(QCloseEvent *event)
{
    Logger::log(Logger::INFO, "Cerrando ventana Teacher");
    this->hide();
    if (parentWidget()) parentWidget()->show();
    event->accept();
}

void Teacher::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}

void Teacher::setupUI()
{
    mTitleLabel = new QLabel("CodeIntegrity - Profesor");
    mTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mLogoutButton = new QPushButton("Cerrar Sesión");
    mLogoutButton->setMinimumHeight(40);
    mLogoutButton->setMaximumWidth(150);
    mLogoutButton->setCursor(Qt::PointingHandCursor);

    mSignatureSearchBar = new QLineEdit();
    mSignatureSearchBar->setPlaceholderText("Buscar asignaturas...");
    mSignatureSearchBar->setClearButtonEnabled(true);

    mAsignaturasListWidget = new QListWidget();
    mAsignaturasListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mAsignaturasListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mAsignaturasListWidget->setUniformItemSizes(true);
    mAsignaturasListWidget->setSpacing(2);

    mTaskSearchBar = new QLineEdit();
    mTaskSearchBar->setPlaceholderText("Buscar tareas...");
    mTaskSearchBar->setClearButtonEnabled(true);

    mTareasListWidget = new QListWidget();
    mTareasListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mTareasListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mTareasListWidget->setUniformItemSizes(true);
    mTareasListWidget->setSpacing(2);

    mAddTaskButton = new QPushButton("➕ Agregar Tarea");
    mAddTaskButton->setMinimumHeight(40);
    mAddTaskButton->setCursor(Qt::PointingHandCursor);

    mTaskSelectedTextEdit = new QTextEdit();
    mTaskSelectedTextEdit->setReadOnly(true);
    mTaskSelectedTextEdit->setText("Ninguna tarea seleccionada");
    mTaskSelectedTextEdit->setFixedHeight(50);
    mTaskSelectedTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mCheckBoxLayout = new QHBoxLayout();

    mRunAnalysisButton = new QPushButton("▶️ Ejecutar Análisis");
    mRunAnalysisButton->setMinimumHeight(40);
    mRunAnalysisButton->setCursor(Qt::PointingHandCursor);
    mRunAnalysisButton->setEnabled(false);
}

void Teacher::setupLayout()
{
    QWidget* central = centralWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QFrame* headerFrame = new QFrame();
    headerFrame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(20);
    headerLayout->addWidget(mTitleLabel, 1);
    headerLayout->addWidget(mLogoutButton);
    mainLayout->addWidget(headerFrame);

    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep1);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #1e1e1e; }");

    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(20);

    QLabel* signaturesLabel = new QLabel("Mis Asignaturas");
    signaturesLabel->setAlignment(Qt::AlignLeft);
    scrollLayout->addWidget(signaturesLabel);
    scrollLayout->addWidget(mSignatureSearchBar);
    scrollLayout->addWidget(mAsignaturasListWidget, 1);

    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    scrollLayout->addWidget(sep2);

    QLabel* tasksLabel = new QLabel("Gestión de Tareas");
    tasksLabel->setAlignment(Qt::AlignLeft);
    scrollLayout->addWidget(tasksLabel);

    QHBoxLayout* taskCtrl = new QHBoxLayout();
    taskCtrl->setSpacing(10);
    taskCtrl->addWidget(mTaskSearchBar, 1);
    taskCtrl->addWidget(mAddTaskButton);
    scrollLayout->addLayout(taskCtrl);
    scrollLayout->addWidget(mTareasListWidget, 1);

    QFrame* sep3 = new QFrame();
    sep3->setFrameShape(QFrame::HLine);
    sep3->setFrameShadow(QFrame::Sunken);
    scrollLayout->addWidget(sep3);

    QLabel* analysisLabel = new QLabel("Análisis de Código");
    analysisLabel->setAlignment(Qt::AlignLeft);
    scrollLayout->addWidget(analysisLabel);

    QLabel* taskSelectedLabel = new QLabel("Tarea seleccionada:");
    scrollLayout->addWidget(taskSelectedLabel);
    scrollLayout->addWidget(mTaskSelectedTextEdit);
    scrollLayout->addLayout(mCheckBoxLayout);
    scrollLayout->addWidget(mRunAnalysisButton);
    scrollLayout->addStretch();

    central->setProperty("signaturesLabel", QVariant::fromValue(signaturesLabel));
    central->setProperty("tasksLabel",      QVariant::fromValue(tasksLabel));
    central->setProperty("analysisLabel",   QVariant::fromValue(analysisLabel));

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);
}

void Teacher::applyStyles()
{
    centralWidget()->setStyleSheet(
        "QWidget { background-color: #1e1e1e; color: #ffffff; }" +
        mStyleManager->getLineEditStyle() +
        mStyleManager->getButtonStyle() +
        "QScrollBar:vertical { border: none; background: #1f1f1f; width: 12px; "
        "  border-radius: 6px; margin: 15px 0; } "
        "QScrollBar::handle:vertical { background-color: #4a4a4a; min-height: 30px; "
        "  border-radius: 6px; } "
        "QScrollBar::handle:vertical:hover { background-color: #5a5a5a; } "
        "QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical { height: 0px; } "
        "QFrame { color: #3d3d3d; }"
        );

    mTitleLabel->setStyleSheet(
        "QLabel { color: #ffffff; font-size: 24px; font-weight: bold; "
        "font-family: 'Segoe UI', Arial, sans-serif; background-color: transparent; }"
        );

    QWidget* central = centralWidget();
    auto applyLabelStyle = [this](QLabel* label) {
        if (label) label->setStyleSheet(mStyleManager->getLabelSectionStyle());
    };
    applyLabelStyle(qvariant_cast<QLabel*>(central->property("signaturesLabel")));
    applyLabelStyle(qvariant_cast<QLabel*>(central->property("tasksLabel")));
    applyLabelStyle(qvariant_cast<QLabel*>(central->property("analysisLabel")));

    mSignatureSearchBar->setStyleSheet(mStyleManager->getLineEditStyle());
    mSignatureSearchBar->setMinimumHeight(40);

    mTaskSearchBar->setStyleSheet(mStyleManager->getLineEditStyle());
    mTaskSearchBar->setMinimumHeight(40);

    mAsignaturasListWidget->setStyleSheet(mStyleManager->getTeacherListWidgetStyle());
    mAsignaturasListWidget->setMinimumHeight(200);

    mTareasListWidget->setStyleSheet(mStyleManager->getTeacherListWidgetStyle());
    mTareasListWidget->setMinimumHeight(200);

    mTaskSelectedTextEdit->setStyleSheet(
        "QTextEdit { background-color: #2d2d2d; color: #ffffff; "
        "  border: 1px solid #3d3d3d; border-radius: 6px; padding: 10px; "
        "  font-size: 16px; font-weight: 600; } "
        "QTextEdit:focus { border: 2px solid #4CAF50; }"
        );

    mLogoutButton->setStyleSheet(mStyleManager->getButtonStyle("#f44336", "#ffffff"));
    mAddTaskButton->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
    mRunAnalysisButton->setStyleSheet(mStyleManager->getDisabledButtonStyle("#4a4a4a", "#888888"));
}

void Teacher::setupConnections()
{
    mAsignaturasListWidget->viewport()->installEventFilter(this);
    mTareasListWidget->viewport()->installEventFilter(this);

    connect(mLogoutButton,       &QPushButton::clicked,    this, &Teacher::logoutButtonClicked);
    connect(mSignatureSearchBar, &QLineEdit::textChanged,  this, &Teacher::filtrarAsignaturas);
    connect(mTaskSearchBar,      &QLineEdit::textChanged,  this, &Teacher::filtrarTareas);
    connect(mAddTaskButton,      &QPushButton::clicked,    this, &Teacher::anadirTarea);
    connect(mRunAnalysisButton,  &QPushButton::clicked,    this, &Teacher::checkRunAnalysis);

    connect(mSubmissionManager.get(),   &SubmissionManager::extractionFinished,
            this, &Teacher::onSubmissionExtractionFinished);
    connect(mAnalysisCoordinator.get(), &AnalysisCoordinator::analysisFinished,
            this, &Teacher::onAnalysisFinished);
    connect(mAnalysisCoordinator.get(), &AnalysisCoordinator::analysisError,
            this, &Teacher::onAnalysisError);
}

void Teacher::setupAnalysisTools()
{
    mMossCheckBox  = new QCheckBox("MOSS",  this);
    mJplagCheckBox = new QCheckBox("JPlag", this);

    mAnalysisToolGroup = new QButtonGroup(this);
    mAnalysisToolGroup->setExclusive(false);
    mAnalysisToolGroup->addButton(mMossCheckBox);
    mAnalysisToolGroup->addButton(mJplagCheckBox);

    connect(mMossCheckBox,  &QCheckBox::toggled, this, &Teacher::handleToolSelection);
    connect(mJplagCheckBox, &QCheckBox::toggled, this, &Teacher::handleToolSelection);

    mMossCheckBox->setStyleSheet(mStyleManager->getCheckBoxStyle());
    mJplagCheckBox->setStyleSheet(mStyleManager->getCheckBoxStyle());

    if (mCheckBoxLayout)
    {
        mCheckBoxLayout->addWidget(mMossCheckBox);
        mCheckBoxLayout->addWidget(mJplagCheckBox);
        mCheckBoxLayout->addStretch();
    }
}

void Teacher::cargarAsignaturasProfesor()
{
    mAsignaturasListWidget->clear();
    mAsignaturas = mDb->getSubjectsByTeacher(mUserId);

    if (mAsignaturas.empty())
    {
        QListWidgetItem* item = new QListWidgetItem("No tienes asignaturas asignadas");
        item->setFlags(Qt::NoItemFlags);
        mAsignaturasListWidget->addItem(item);
        Logger::log(Logger::WARNING, QString("Profesor %1 sin asignaturas").arg(mUserId));
        return;
    }

    for (const auto& asig : mAsignaturas)
        crearWidgetAsignatura(asig);

    Logger::log(Logger::INFO, QString("Cargadas %1 asignaturas del profesor %2")
                                  .arg(mAsignaturas.size()).arg(mUserId));
}

void Teacher::cargarTareasAsignatura(int idAsignatura)
{
    mTareasListWidget->clear();
    mAsignaturaSeleccionada = idAsignatura;

    mTareasActuales = mDb->getTasksBySubject(idAsignatura);

    if (mTareasActuales.empty())
    {
        QListWidgetItem* item = new QListWidgetItem("No hay tareas para esta asignatura");
        item->setFlags(Qt::NoItemFlags);
        mTareasListWidget->addItem(item);
        return;
    }

    for (const auto& tarea : mTareasActuales)
        crearWidgetTarea(tarea);
}

void Teacher::crearWidgetAsignatura(const SubjectInfo& asignatura)
{
    QListWidgetItem* item = new QListWidgetItem(mAsignaturasListWidget);
    int idCapturado = asignatura.id;

    QWidget* widget = mUIFactory->createAssignmentWidget(
        asignatura,
        [this, idCapturado]() { cargarTareasAsignatura(idCapturado); }
        );

    mUIFactory->applyUniformStyle(widget, item);
    mAsignaturasListWidget->setItemWidget(item, widget);
}

void Teacher::crearWidgetTarea(const TaskInfo& tarea)
{
    QListWidgetItem* item = new QListWidgetItem(mTareasListWidget);
    int    idCapturado    = tarea.id;
    QString titulo        = tarea.titulo;
    QString descripcion   = tarea.descripcion;

    QWidget* widget = mUIFactory->createTaskWidget(
        tarea,
        [this, idCapturado, titulo](QPushButton* btn) {
            Q_UNUSED(btn)
            subirArchivoTarea(idCapturado, titulo);
        },
        [this, idCapturado, titulo, descripcion]() {
            editarTarea(idCapturado, titulo, descripcion);
        },
        [this, idCapturado, titulo]() {
            eliminarTarea(idCapturado, titulo);
        },
        [this, titulo](QPushButton* btn) {
            Q_UNUSED(btn)
            seleccionarTarea(titulo);
        }
        );

    mUIFactory->applyUniformStyle(widget, item);
    mTareasListWidget->setItemWidget(item, widget);
}

void Teacher::logoutButtonClicked()
{
    this->hide();
    if (parentWidget()) parentWidget()->show();
}

void Teacher::filtrarAsignaturas(const QString& texto)
{
    FilterManager::filterListWidget(mAsignaturasListWidget, texto);
}

void Teacher::filtrarTareas(const QString& texto)
{
    FilterManager::filterListWidget(mTareasListWidget, texto);
}

void Teacher::actualizarTareaSeleccionada(const QString& nombreTarea)
{
    mTareaSeleccionada = nombreTarea;
    mTaskSelectedTextEdit->setText(nombreTarea.isEmpty() ? "Ninguna tarea seleccionada" : nombreTarea);
}

static QString dateTimeEditStyle(StyleManager* sm)
{
    return QString(
        "QDateTimeEdit {"
        "  background-color: #2d2d2d; color: #ffffff;"
        "  border: 1px solid #3d3d3d; border-radius: 6px;"
        "  padding: 8px 12px; min-height: 40px;"
        "  font-size: 13px; font-family: 'Segoe UI', Arial, sans-serif;"
        "}"
        "QDateTimeEdit:focus { border: 2px solid #4CAF50; }"
        "QDateTimeEdit::drop-down { border: none; }"
        "QDateTimeEdit::up-button, QDateTimeEdit::down-button { width: 0px; }"
    );
}

void Teacher::anadirTarea()
{
    if (mAsignaturaSeleccionada == -1)
    {
        QMessageBox::warning(this, "Advertencia",
                             "Primero selecciona una asignatura usando el botón 'Ver Tareas'");
        return;
    }

    int academicYearId = 0;
    for (const auto& a : mAsignaturas)
    {
        if (a.id == mAsignaturaSeleccionada)
        {
            academicYearId = a.academicYearId;
            break;
        }
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Agregar Tarea");
    dialog.setMinimumSize(520, 420);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel* titleLabel = new QLabel("Crear nueva tarea");
    titleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());
    mainLayout->addWidget(titleLabel);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLineEdit* tituloInput = new QLineEdit();
    tituloInput->setPlaceholderText("Ej: Práctica 1 - Herencia");
    tituloInput->setStyleSheet(mStyleManager->getLineEditStyle());
    tituloInput->setMinimumHeight(40);

    QTextEdit* descInput = new QTextEdit();
    descInput->setPlaceholderText("Descripción de la tarea, requisitos, instrucciones...");
    descInput->setStyleSheet(
        "QTextEdit { background-color: #2d2d2d; color: #ffffff;"
        "  border: 1px solid #3d3d3d; border-radius: 6px; padding: 8px; }"
        "QTextEdit:focus { border: 2px solid #4CAF50; }"
    );
    descInput->setMaximumHeight(110);

    QDateTimeEdit* fechaEdit = new QDateTimeEdit();
    fechaEdit->setDisplayFormat("dd/MM/yyyy HH:mm");
    fechaEdit->setDateTime(QDateTime::currentDateTime().addDays(14));
    fechaEdit->setCalendarPopup(true);
    fechaEdit->setMinimumDateTime(QDateTime::currentDateTime());
    fechaEdit->setStyleSheet(dateTimeEditStyle(mStyleManager));

    QLabel* tituloLabel = new QLabel("Título:");
    tituloLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* descLabel = new QLabel("Descripción:");
    descLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* fechaLabel = new QLabel("Fecha límite:");
    fechaLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());

    formLayout->addRow(tituloLabel, tituloInput);
    formLayout->addRow(descLabel,   descInput);
    formLayout->addRow(fechaLabel,  fechaEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* cancelBtn = new QPushButton("Cancelar");
    cancelBtn->setStyleSheet(mStyleManager->getButtonStyle("#555555", "#ffffff"));
    cancelBtn->setMinimumSize(120, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);

    QPushButton* saveBtn = new QPushButton("✓ Crear Tarea");
    saveBtn->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
    saveBtn->setMinimumSize(120, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        QString titulo = tituloInput->text().trimmed();

        if (titulo.isEmpty())
        {
            QMessageBox::warning(&dialog, "Campo obligatorio",
                                 "El título de la tarea es obligatorio");
            return;
        }

        QString descripcion = descInput->toPlainText().trimmed();
        QDateTime fechaLimite = fechaEdit->dateTime();

        QString errorMsg;
        if (mDb->insertTask(titulo, descripcion, mAsignaturaSeleccionada,
                            academicYearId, fechaLimite, &errorMsg))
        {
            Logger::log(Logger::INFO, QString("Tarea '%1' creada").arg(titulo));
            dialog.accept();
            cargarTareasAsignatura(mAsignaturaSeleccionada);
        }
        else
        {
            QMessageBox::critical(&dialog, "Error",
                                  "No se pudo crear la tarea:\n" + errorMsg);
        }
    });

    dialog.exec();
}

void Teacher::editarTarea(int idTarea, const QString& nombreTarea, const QString& descripcionTarea)
{
    QDateTime fechaActual = QDateTime::currentDateTime().addDays(14);
    for (const auto& t : mTareasActuales)
    {
        if (t.id == idTarea)
        {
            if (t.fechaLimite.isValid())
                fechaActual = t.fechaLimite;
            break;
        }
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Editar Tarea");
    dialog.setMinimumSize(520, 420);

    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel* titleLabel = new QLabel("Editar tarea: " + nombreTarea);
    titleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());
    titleLabel->setWordWrap(true);
    mainLayout->addWidget(titleLabel);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QLineEdit* tituloInput = new QLineEdit(nombreTarea);
    tituloInput->setStyleSheet(mStyleManager->getLineEditStyle());
    tituloInput->setMinimumHeight(40);

    QTextEdit* descInput = new QTextEdit();
    descInput->setPlainText(descripcionTarea);
    descInput->setStyleSheet(
        "QTextEdit { background-color: #2d2d2d; color: #ffffff;"
        "  border: 1px solid #3d3d3d; border-radius: 6px; padding: 8px; }"
        "QTextEdit:focus { border: 2px solid #4CAF50; }"
    );
    descInput->setMaximumHeight(110);

    QDateTimeEdit* fechaEdit = new QDateTimeEdit();
    fechaEdit->setDisplayFormat("dd/MM/yyyy HH:mm");
    fechaEdit->setDateTime(fechaActual);
    fechaEdit->setCalendarPopup(true);
    fechaEdit->setStyleSheet(dateTimeEditStyle(mStyleManager));

    QLabel* tituloLabel = new QLabel("Título:");
    tituloLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* descLabel = new QLabel("Descripción:");
    descLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* fechaLabel = new QLabel("Fecha límite:");
    fechaLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());

    formLayout->addRow(tituloLabel, tituloInput);
    formLayout->addRow(descLabel,   descInput);
    formLayout->addRow(fechaLabel,  fechaEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton* cancelBtn = new QPushButton("Cancelar");
    cancelBtn->setStyleSheet(mStyleManager->getButtonStyle("#555555", "#ffffff"));
    cancelBtn->setMinimumSize(120, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);

    QPushButton* saveBtn = new QPushButton("✓ Guardar Cambios");
    saveBtn->setStyleSheet(mStyleManager->getButtonStyle("#2196F3", "#ffffff"));
    saveBtn->setMinimumSize(120, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        QString nuevoTitulo = tituloInput->text().trimmed();

        if (nuevoTitulo.isEmpty())
        {
            QMessageBox::warning(&dialog, "Campo obligatorio",
                                 "El título no puede estar vacío");
            return;
        }

        QString nuevaDesc = descInput->toPlainText().trimmed();
        QDateTime fechaLimite = fechaEdit->dateTime();

        QString errorMsg;
        if (mDb->updateTask(idTarea, nuevoTitulo, nuevaDesc, fechaLimite, &errorMsg))
        {
            Logger::log(Logger::INFO, QString("Tarea %1 actualizada").arg(idTarea));
            if (mTareaSeleccionada == nombreTarea)
                actualizarTareaSeleccionada(nuevoTitulo);
            dialog.accept();
            cargarTareasAsignatura(mAsignaturaSeleccionada);
        }
        else
        {
            QMessageBox::critical(&dialog, "Error",
                                  "No se pudo actualizar la tarea:\n" + errorMsg);
        }
    });

    dialog.exec();
}

void Teacher::eliminarTarea(int idTarea, const QString& nombreTarea)
{
    if (QMessageBox::question(this, "Confirmar eliminación",
                              QString("¿Eliminar la tarea '%1'?\n"
                                      "Esta acción no se puede deshacer.").arg(nombreTarea),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    QString errorMsg;
    if (mDb->deleteTask(idTarea, &errorMsg))
    {
        Logger::log(Logger::INFO, QString("Tarea %1 eliminada").arg(idTarea));
        if (mTareaSeleccionada == nombreTarea)
        {
            actualizarTareaSeleccionada("");
            mRunAnalysisButton->setEnabled(false);
            mRunAnalysisButton->setStyleSheet(mStyleManager->getDisabledButtonStyle("#4a4a4a", "#888888"));
        }
        cargarTareasAsignatura(mAsignaturaSeleccionada);
    }
    else
    {
        QMessageBox::critical(this, "Error", "No se pudo eliminar la tarea:\n" + errorMsg);
    }
}

void Teacher::seleccionarTarea(const QString& nombreTarea)
{
    actualizarTareaSeleccionada(nombreTarea);

    bool hayHerramienta = mMossCheckBox->isChecked() || mJplagCheckBox->isChecked();
    bool hayArchivo     = !mCurrentSubmissionPath.isEmpty();

    if (hayHerramienta && hayArchivo)
    {
        mRunAnalysisButton->setEnabled(true);
        mRunAnalysisButton->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
    }

    Logger::log(Logger::INFO, QString("Tarea seleccionada: %1").arg(nombreTarea));
}

void Teacher::subirArchivoTarea(int idTarea, const QString& nombreTarea)
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        QString("Seleccionar archivo de entregas para: %1").arg(nombreTarea),
        "",
        "Archivos comprimidos (*.rar *.zip)"
        );

    if (filePath.isEmpty()) return;

    Logger::log(Logger::INFO, QString("Subiendo archivo para tarea %1: %2").arg(idTarea).arg(filePath));
    mSubmissionManager->processSubmissionFile(idTarea, filePath);
}

void Teacher::handleToolSelection(bool checked)
{
    QCheckBox* sender = qobject_cast<QCheckBox*>(QObject::sender());
    if (checked)
    {
        for (QAbstractButton* btn : mAnalysisToolGroup->buttons())
            if (btn != sender) btn->setChecked(false);
    }

    bool hayHerramienta = mMossCheckBox->isChecked() || mJplagCheckBox->isChecked();
    bool hayTarea       = !mTareaSeleccionada.isEmpty();
    bool hayArchivo     = !mCurrentSubmissionPath.isEmpty();

    bool listo = hayHerramienta && hayTarea && hayArchivo;
    mRunAnalysisButton->setEnabled(listo);
    mRunAnalysisButton->setStyleSheet(
        listo ? mStyleManager->getButtonStyle("#4CAF50", "#ffffff")
              : mStyleManager->getDisabledButtonStyle("#4a4a4a", "#888888")
        );
}

void Teacher::checkRunAnalysis()
{
    if (mCurrentSubmissionPath.isEmpty())
    {
        QMessageBox::warning(this, "Advertencia", "Primero sube un archivo de entregas");
        return;
    }
    if (mTareaSeleccionada.isEmpty())
    {
        QMessageBox::warning(this, "Advertencia", "Selecciona una tarea primero");
        return;
    }

    AnalysisTool tool = AnalysisTool::None;
    if (mJplagCheckBox->isChecked())     tool = AnalysisTool::JPlag;
    else if (mMossCheckBox->isChecked()) tool = AnalysisTool::MOSS;

    if (tool == AnalysisTool::None)
    {
        QMessageBox::warning(this, "Advertencia", "Selecciona una herramienta de análisis (MOSS o JPlag)");
        return;
    }

    mRunAnalysisButton->setEnabled(false);
    mRunAnalysisButton->setStyleSheet(mStyleManager->getDisabledButtonStyle("#4a4a4a", "#888888"));
    mRunAnalysisButton->setText("⏳ Analizando...");

    Logger::log(Logger::INFO, QString("Iniciando análisis con %1 sobre: %2")
                                  .arg(tool == AnalysisTool::JPlag ? "JPlag" : "MOSS", mCurrentSubmissionPath));

    mAnalysisCoordinator->runAnalysis(tool, mCurrentSubmissionPath);
}

void Teacher::onSubmissionExtractionFinished(bool success, const QString& message)
{
    if (success)
    {
        mCurrentSubmissionPath = mSubmissionManager->getCurrentSubmissionPath();
        Logger::log(Logger::INFO, "Extracción completada: " + message);

        bool hayHerramienta = mMossCheckBox->isChecked() || mJplagCheckBox->isChecked();
        bool hayTarea       = !mTareaSeleccionada.isEmpty();

        if (hayHerramienta && hayTarea)
        {
            mRunAnalysisButton->setEnabled(true);
            mRunAnalysisButton->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
        }

        QMessageBox::information(this, "Extracción completada", message);
    }
    else
    {
        mCurrentSubmissionPath.clear();
        Logger::log(Logger::ERROR_LEVEL, "Error en extracción: " + message);
        QMessageBox::critical(this, "Error en extracción", message);
    }
}

void Teacher::onAnalysisFinished(bool success, const QString& reportUrl)
{
    mRunAnalysisButton->setEnabled(true);
    mRunAnalysisButton->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
    mRunAnalysisButton->setText("▶️ Ejecutar Análisis");

    if (success)
    {
        Logger::log(Logger::INFO, "Análisis completado. Reporte: " + reportUrl);
        QMessageBox::information(this, "Análisis completado",
                                 "El análisis ha finalizado correctamente.\n"
                                 "Reporte disponible en:\n" + reportUrl);
    }
    else
    {
        QMessageBox::warning(this, "Análisis incompleto",
                             "El análisis finalizó pero no se generó reporte");
    }
}

void Teacher::onAnalysisError(const QString& errorMessage)
{
    mRunAnalysisButton->setEnabled(true);
    mRunAnalysisButton->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
    mRunAnalysisButton->setText("▶️ Ejecutar Análisis");

    Logger::log(Logger::ERROR_LEVEL, "Error en análisis: " + errorMessage);
    QMessageBox::critical(this, "Error en análisis", errorMessage);
}
