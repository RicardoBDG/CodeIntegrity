#include "Admin.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QScrollArea>
#include <QFrame>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QTextEdit>
#include <QFormLayout>
#include <QComboBox>
#include <algorithm>
#include "../model/Logger.hpp"

static QString comboBoxStyle(StyleManager* styleManager)
{
    return "QComboBox { " + styleManager->getLineEditStyle() + " min-height: 40px; }"
           "QComboBox::drop-down { border: none; }"
           "QComboBox QAbstractItemView { background-color: #2d2d2d; color: #ffffff; }";
}

Admin::Admin(QWidget *parent)
    : QMainWindow(parent)
{
    mDb           = DataBaseManager::getInstance();
    mStyleManager = new StyleManager();

    setWindowTitle("CodeIntegrity - Administración");
    setMinimumSize(700, 1000);
    setWindowIcon(QIcon(":/icons/app_icon.png"));

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    setupUI();
    setupLayout();
    applyStyles();
    setupConnections();
    refreshAllData();

    Logger::log(Logger::INFO, "Ventana de Admin inicializada");
}

Admin::~Admin()
{
    delete mStyleManager;
    Logger::log(Logger::INFO, "Ventana de Admin cerrada");
}

bool Admin::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonDblClick)
    {
        QListWidget* lists[] = { mUsersListWidget, mSubjectsListWidget, mCoursesListWidget };
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

void Admin::closeEvent(QCloseEvent *event)
{
    Logger::log(Logger::INFO, "Cerrando ventana Admin");
    this->hide();
    if (parentWidget()) parentWidget()->show();
    event->accept();
}

void Admin::setupUI()
{
    mTitleLabel = new QLabel("CodeIntegrity - Administración");
    mTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    mLogoutButton = new QPushButton("Cerrar Sesión");
    mLogoutButton->setMinimumHeight(40);
    mLogoutButton->setMaximumWidth(150);
    mLogoutButton->setCursor(Qt::PointingHandCursor);

    mUsersSearchInput = new QLineEdit();
    mUsersSearchInput->setPlaceholderText("Buscar usuarios...");
    mUsersSearchInput->setClearButtonEnabled(true);

    mUsersListWidget = new QListWidget();
    mUsersListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mUsersListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mAddUserButton    = new QPushButton("➕ Agregar");
    mEditUserButton   = new QPushButton("✏️ Editar");
    mRemoveUserButton = new QPushButton("🗑️ Eliminar");
    mAddUserButton->setMinimumHeight(35);
    mEditUserButton->setMinimumHeight(35);
    mRemoveUserButton->setMinimumHeight(35);
    mAddUserButton->setCursor(Qt::PointingHandCursor);
    mEditUserButton->setCursor(Qt::PointingHandCursor);
    mRemoveUserButton->setCursor(Qt::PointingHandCursor);

    mSubjectsSearchInput = new QLineEdit();
    mSubjectsSearchInput->setPlaceholderText("Buscar asignaturas...");
    mSubjectsSearchInput->setClearButtonEnabled(true);

    mSubjectsListWidget = new QListWidget();
    mSubjectsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mSubjectsListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mAddSubjectButton    = new QPushButton("➕ Agregar");
    mEditSubjectButton   = new QPushButton("✏️ Editar");
    mRemoveSubjectButton = new QPushButton("🗑️ Eliminar");
    mAssignTeachersButton = new QPushButton("👥 Asignar Profesores");
    mAddSubjectButton->setMinimumHeight(35);
    mEditSubjectButton->setMinimumHeight(35);
    mRemoveSubjectButton->setMinimumHeight(35);
    mAssignTeachersButton->setMinimumHeight(35);
    mAddSubjectButton->setCursor(Qt::PointingHandCursor);
    mEditSubjectButton->setCursor(Qt::PointingHandCursor);
    mRemoveSubjectButton->setCursor(Qt::PointingHandCursor);
    mAssignTeachersButton->setCursor(Qt::PointingHandCursor);

    mCoursesSearchInput = new QLineEdit();
    mCoursesSearchInput->setPlaceholderText("Buscar cursos...");
    mCoursesSearchInput->setClearButtonEnabled(true);

    mCoursesListWidget = new QListWidget();
    mCoursesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    mCoursesListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mAddCourseButton    = new QPushButton("➕ Agregar");
    mEditCourseButton   = new QPushButton("✏️ Editar");
    mRemoveCourseButton = new QPushButton("🗑️ Eliminar");
    mAddCourseButton->setMinimumHeight(35);
    mEditCourseButton->setMinimumHeight(35);
    mRemoveCourseButton->setMinimumHeight(35);
    mAddCourseButton->setCursor(Qt::PointingHandCursor);
    mEditCourseButton->setCursor(Qt::PointingHandCursor);
    mRemoveCourseButton->setCursor(Qt::PointingHandCursor);
}

void Admin::setupLayout()
{
    QWidget* central = centralWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QFrame* headerFrame = new QFrame();
    headerFrame->setFrameShape(QFrame::NoFrame);
    QHBoxLayout* headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(20, 15, 20, 15);
    headerLayout->addWidget(mTitleLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(mLogoutButton);
    mainLayout->addWidget(headerFrame);

    QFrame* sep1 = new QFrame();
    sep1->setFrameShape(QFrame::HLine);
    sep1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(sep1);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #1e1e1e; }");

    QWidget* contentWidget = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(20);
    contentLayout->addSpacing(15);

    // ── USUARIOS ──
    QLabel* usersTitle = new QLabel("Gestión de Usuarios");
    usersTitle->setObjectName("sectionTitle");
    contentLayout->addWidget(usersTitle);

    QHBoxLayout* usersCtrl = new QHBoxLayout();
    usersCtrl->addWidget(mUsersSearchInput, 1);
    usersCtrl->addWidget(mAddUserButton);
    usersCtrl->addWidget(mEditUserButton);
    usersCtrl->addWidget(mRemoveUserButton);
    contentLayout->addLayout(usersCtrl);
    contentLayout->addWidget(mUsersListWidget, 1);

    QFrame* sep2 = new QFrame();
    sep2->setFrameShape(QFrame::HLine);
    sep2->setFrameShadow(QFrame::Sunken);
    contentLayout->addWidget(sep2);

    // ── ASIGNATURAS ──
    QLabel* subjectsTitle = new QLabel("Gestión de Asignaturas");
    subjectsTitle->setObjectName("sectionTitle");
    contentLayout->addWidget(subjectsTitle);

    QHBoxLayout* subjectsCtrl = new QHBoxLayout();
    subjectsCtrl->addWidget(mSubjectsSearchInput, 1);
    subjectsCtrl->addWidget(mAddSubjectButton);
    subjectsCtrl->addWidget(mEditSubjectButton);
    subjectsCtrl->addWidget(mRemoveSubjectButton);
    subjectsCtrl->addWidget(mAssignTeachersButton);
    contentLayout->addLayout(subjectsCtrl);
    contentLayout->addWidget(mSubjectsListWidget, 1);

    QFrame* sep3 = new QFrame();
    sep3->setFrameShape(QFrame::HLine);
    sep3->setFrameShadow(QFrame::Sunken);
    contentLayout->addWidget(sep3);

    // ── CURSOS ──
    QLabel* coursesTitle = new QLabel("Gestión de Cursos Académicos");
    coursesTitle->setObjectName("sectionTitle");
    contentLayout->addWidget(coursesTitle);

    QHBoxLayout* coursesCtrl = new QHBoxLayout();
    coursesCtrl->addWidget(mCoursesSearchInput, 1);
    coursesCtrl->addWidget(mAddCourseButton);
    coursesCtrl->addWidget(mEditCourseButton);
    coursesCtrl->addWidget(mRemoveCourseButton);
    contentLayout->addLayout(coursesCtrl);
    contentLayout->addWidget(mCoursesListWidget, 1);

    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea);
}

void Admin::applyStyles()
{
    centralWidget()->setStyleSheet(
        QString("QWidget { background-color: %1; color: %2; }"
        "QLabel#sectionTitle { color: %2; font-size: 14px; font-weight: 600; "
        "  font-family: 'Segoe UI', Arial, sans-serif; letter-spacing: 1px; }"
        "QFrame { color: %3; }")
            .arg(StyleManager::getDarkBgPrimary(), StyleManager::getTextPrimary(), StyleManager::getDarkBorder()) +
        "QScrollBar:vertical { border: none; background: #1f1f1f; width: 12px; "
        "  border-radius: 6px; margin: 15px 0; }"
        "QScrollBar::handle:vertical { background-color: #4a4a4a; min-height: 30px; "
        "  border-radius: 6px; }"
        "QScrollBar::handle:vertical:hover { background-color: #5a5a5a; }"
        "QScrollBar::sub-line:vertical, QScrollBar::add-line:vertical { height: 0px; }"
        );

    mTitleLabel->setStyleSheet(
        "QLabel { color: #ffffff; font-size: 24px; font-weight: bold; "
        "font-family: 'Segoe UI', Arial, sans-serif; background-color: transparent; }"
        );

    mUsersSearchInput->setStyleSheet(mStyleManager->getLineEditStyle());
    mSubjectsSearchInput->setStyleSheet(mStyleManager->getLineEditStyle());
    mCoursesSearchInput->setStyleSheet(mStyleManager->getLineEditStyle());

    mUsersListWidget->setStyleSheet(mStyleManager->getListWidgetStyle());
    mSubjectsListWidget->setStyleSheet(mStyleManager->getListWidgetStyle());
    mCoursesListWidget->setStyleSheet(mStyleManager->getListWidgetStyle());

    QString addStyle    = mStyleManager->getButtonStyle("#4CAF50", "#ffffff");
    QString editStyle   = mStyleManager->getButtonStyle("#2196F3", "#ffffff");
    QString deleteStyle = mStyleManager->getButtonStyle("#f44336", "#ffffff");

    mAddUserButton->setStyleSheet(addStyle);
    mAddSubjectButton->setStyleSheet(addStyle);
    mAddCourseButton->setStyleSheet(addStyle);

    mEditUserButton->setStyleSheet(editStyle);
    mEditSubjectButton->setStyleSheet(editStyle);
    mEditCourseButton->setStyleSheet(editStyle);

    mRemoveUserButton->setStyleSheet(deleteStyle);
    mRemoveSubjectButton->setStyleSheet(deleteStyle);
    
    QString assignStyle = mStyleManager->getButtonStyle("#6f42c1", "#ffffff");
    mAssignTeachersButton->setStyleSheet(assignStyle);
    mRemoveCourseButton->setStyleSheet(deleteStyle);
    mLogoutButton->setStyleSheet(deleteStyle);
}

void Admin::setupConnections()
{
    mUsersListWidget->viewport()->installEventFilter(this);
    mSubjectsListWidget->viewport()->installEventFilter(this);
    mCoursesListWidget->viewport()->installEventFilter(this);

    connect(mLogoutButton, &QPushButton::clicked, this, &Admin::onLogoutClicked);

    connect(mUsersSearchInput,    &QLineEdit::textChanged, this, &Admin::onUsersFilterChanged);
    connect(mSubjectsSearchInput, &QLineEdit::textChanged, this, &Admin::onSubjectsFilterChanged);
    connect(mCoursesSearchInput,  &QLineEdit::textChanged, this, &Admin::onCoursesFilterChanged);

    connect(mUsersListWidget,    &QListWidget::itemSelectionChanged, this, &Admin::onUserSelected);
    connect(mSubjectsListWidget, &QListWidget::itemSelectionChanged, this, &Admin::onSubjectSelected);
    connect(mCoursesListWidget,  &QListWidget::itemSelectionChanged, this, &Admin::onCourseSelected);

    connect(mAddUserButton,    &QPushButton::clicked, this, &Admin::onAddUserClicked);
    connect(mEditUserButton,   &QPushButton::clicked, this, &Admin::onEditUserClicked);
    connect(mRemoveUserButton, &QPushButton::clicked, this, &Admin::onRemoveUserClicked);

    connect(mAddSubjectButton,    &QPushButton::clicked, this, &Admin::onAddSubjectClicked);
    connect(mEditSubjectButton,   &QPushButton::clicked, this, &Admin::onEditSubjectClicked);
    connect(mRemoveSubjectButton, &QPushButton::clicked, this, &Admin::onRemoveSubjectClicked);
    connect(mAssignTeachersButton, &QPushButton::clicked, this, &Admin::onAssignTeachersClicked);

    connect(mAddCourseButton,    &QPushButton::clicked, this, &Admin::onAddCourseClicked);
    connect(mEditCourseButton,   &QPushButton::clicked, this, &Admin::onEditCourseClicked);
    connect(mRemoveCourseButton, &QPushButton::clicked, this, &Admin::onRemoveCourseClicked);
}

void Admin::loadUsers()
{
    mUsersListWidget->clear();
    auto usuarios = mDb->getUsers();
    for (const auto& u : usuarios)
    {
        QString display = QString("%1 %2 — %3 [%4]")
                              .arg(u.nombre, u.apellido, u.email, u.tipoRol);
        QListWidgetItem* item = new QListWidgetItem(display, mUsersListWidget);
        item->setData(Qt::UserRole, u.id);
    }
}

void Admin::loadSubjects()
{
    mSubjectsListWidget->clear();
    mSubjectsListWidget->setTextElideMode(Qt::ElideRight);

    QMap<int, QString> cursoMap;
    for (const auto& c : mDb->getAcademicYears())
        cursoMap[c.id] = c.año;

    for (const auto& a : mDb->getSubjects())
    {
        auto profesores = mDb->getTeachersBySubject(a.id, a.academicYearId);

        QString profesoresStr;
        if (profesores.empty())
        {
            profesoresStr = "👤";
        }
        else
        {
            QStringList nombres;
            for (const auto& p : profesores)
                nombres << p.nombre + " " + p.apellido;
            profesoresStr = "👤 " + nombres.join(" · ");
        }

        QString curso = cursoMap.value(a.academicYearId, "?");
        QString display = QString("%1  |  %2  |  %3").arg(curso, a.nombre, profesoresStr);

        QListWidgetItem* item = new QListWidgetItem(display, mSubjectsListWidget);
        item->setData(Qt::UserRole,     a.id);
        item->setData(Qt::UserRole + 1, a.academicYearId);

        if (profesores.empty())
            item->setForeground(QColor("#888888"));
    }
}

void Admin::loadCourses()
{
    mCoursesListWidget->clear();
    auto cursos = mDb->getAcademicYears();
    for (const auto& c : cursos)
    {
        QListWidgetItem* item = new QListWidgetItem(c.año, mCoursesListWidget);  // ← QString
        item->setData(Qt::UserRole, c.id);
    }
}

void Admin::refreshAllData()
{
    loadUsers();
    loadSubjects();
    loadCourses();
}

void Admin::filterListWidget(QListWidget* listWidget, const QString& text)
{
    for (int i = 0; i < listWidget->count(); ++i)
    {
        auto item = listWidget->item(i);
        bool matches = text.isEmpty() || item->text().toLower().contains(text.toLower());
        item->setHidden(!matches);
    }
}

void Admin::clearOtherSelections(QListWidget* current)
{
    for (auto* list : {mUsersListWidget, mSubjectsListWidget, mCoursesListWidget})
        if (list != current) list->clearSelection();
}

void Admin::onLogoutClicked()
{
    this->hide();
    if (parentWidget()) parentWidget()->show();
}

void Admin::onUsersFilterChanged(const QString& text)    { filterListWidget(mUsersListWidget,    text); }
void Admin::onSubjectsFilterChanged(const QString& text) { filterListWidget(mSubjectsListWidget, text); }
void Admin::onCoursesFilterChanged(const QString& text)  { filterListWidget(mCoursesListWidget,  text); }

void Admin::onUserSelected()    { if (!mUsersListWidget->selectedItems().isEmpty())    clearOtherSelections(mUsersListWidget); }
void Admin::onSubjectSelected() { if (!mSubjectsListWidget->selectedItems().isEmpty()) clearOtherSelections(mSubjectsListWidget); }
void Admin::onCourseSelected()  { if (!mCoursesListWidget->selectedItems().isEmpty())  clearOtherSelections(mCoursesListWidget); }

void Admin::onAddUserClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Agregar Usuario");
    dialog.setMinimumSize(500, 400);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    
    QLabel* titleLabel = new QLabel("Crear nuevo usuario");
    titleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());
    mainLayout->addWidget(titleLabel);
    
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    QLineEdit* nombreInput = new QLineEdit();
    nombreInput->setPlaceholderText("Introduce el nombre");
    nombreInput->setStyleSheet(mStyleManager->getLineEditStyle());
    nombreInput->setMinimumHeight(40);
    
    QLineEdit* apellidoInput = new QLineEdit();
    apellidoInput->setPlaceholderText("Introduce el apellido");
    apellidoInput->setStyleSheet(mStyleManager->getLineEditStyle());
    apellidoInput->setMinimumHeight(40);
    
    QLineEdit* emailInput = new QLineEdit();
    emailInput->setPlaceholderText("ejemplo@universidad.edu");
    emailInput->setStyleSheet(mStyleManager->getLineEditStyle());
    emailInput->setMinimumHeight(40);
    
    QLineEdit* passInput = new QLineEdit();
    passInput->setPlaceholderText("Mínimo 8 caracteres");
    passInput->setEchoMode(QLineEdit::Password);
    passInput->setStyleSheet(mStyleManager->getLineEditStyle());
    passInput->setMinimumHeight(40);
    
    QComboBox* rolCombo = new QComboBox();
    rolCombo->addItems({"Profesor", "Administrador"});
    rolCombo->setStyleSheet(comboBoxStyle(mStyleManager));
    
    QLabel* nombreLabel = new QLabel("Nombre:");
    nombreLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* apellidoLabel = new QLabel("Apellido:");
    apellidoLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* emailLabel = new QLabel("Email:");
    emailLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* passLabel = new QLabel("Contraseña:");
    passLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* rolLabel = new QLabel("Rol:");
    rolLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    
    formLayout->addRow(nombreLabel, nombreInput);
    formLayout->addRow(apellidoLabel, apellidoInput);
    formLayout->addRow(emailLabel, emailInput);
    formLayout->addRow(passLabel, passInput);
    formLayout->addRow(rolLabel, rolCombo);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    
    QPushButton* cancelBtn = new QPushButton("Cancelar");
    cancelBtn->setStyleSheet(mStyleManager->getButtonStyle("#555555", "#ffffff"));
    cancelBtn->setMinimumSize(120, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    
    QPushButton* saveBtn = new QPushButton("✓ Guardar");
    saveBtn->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
    saveBtn->setMinimumSize(120, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);
    
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        QString nombre = nombreInput->text().trimmed();
        QString apellido = apellidoInput->text().trimmed();
        QString email = emailInput->text().trimmed();
        QString pass = passInput->text();
        QString rol = rolCombo->currentText();
        
        if (nombre.isEmpty() || apellido.isEmpty() || email.isEmpty() || pass.isEmpty())
        {
            QMessageBox::warning(&dialog, "Campos incompletos", 
                               "Todos los campos son obligatorios");
            return;
        }
        
        if (pass.length() < 8)
        {
            QMessageBox::warning(&dialog, "Contraseña débil",
                               "La contraseña debe tener al menos 8 caracteres");
            return;
        }
        
        QString errorMsg;
        if (mDb->insertUser(nombre, apellido, email, pass, rol, &errorMsg))
        {
            QMessageBox::information(&dialog, "Éxito", "Usuario creado correctamente");
            dialog.accept();
            loadUsers();
        }
        else
        {
            QMessageBox::critical(&dialog, "Error", "No se pudo crear el usuario:\n" + errorMsg);
        }
    });
    
    dialog.exec();
}


void Admin::onEditUserClicked()
{
    if (mUsersListWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(this, "Advertencia", "Selecciona un usuario para editar");
        return;
    }
    
    int id = mUsersListWidget->currentItem()->data(Qt::UserRole).toInt();
    auto usuarios = mDb->getUsers();
    auto it = std::find_if(usuarios.begin(), usuarios.end(),
                           [id](const UserInfo& u){ return u.id == id; });
    if (it == usuarios.end()) return;
    
    QDialog dialog(this);
    dialog.setWindowTitle("Editar Usuario");
    dialog.setMinimumSize(500, 450);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    
    QLabel* titleLabel = new QLabel("Editar usuario: " + it->nombre + " " + it->apellido);
    titleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());
    mainLayout->addWidget(titleLabel);
    
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    QLineEdit* nombreInput = new QLineEdit(it->nombre);
    nombreInput->setStyleSheet(mStyleManager->getLineEditStyle());
    nombreInput->setMinimumHeight(40);
    
    QLineEdit* apellidoInput = new QLineEdit(it->apellido);
    apellidoInput->setStyleSheet(mStyleManager->getLineEditStyle());
    apellidoInput->setMinimumHeight(40);
    
    QLineEdit* emailInput = new QLineEdit(it->email);
    emailInput->setStyleSheet(mStyleManager->getLineEditStyle());
    emailInput->setMinimumHeight(40);
    
    QLineEdit* passInput = new QLineEdit();
    passInput->setPlaceholderText("Dejar vacío para no cambiar");
    passInput->setEchoMode(QLineEdit::Password);
    passInput->setStyleSheet(mStyleManager->getLineEditStyle());
    passInput->setMinimumHeight(40);
    
    QComboBox* rolCombo = new QComboBox();
    rolCombo->addItems({"Profesor", "Administrador"});
    rolCombo->setCurrentText(it->tipoRol);
    rolCombo->setStyleSheet(comboBoxStyle(mStyleManager));
    
    QLabel* nombreLabel = new QLabel("Nombre:");
    nombreLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* apellidoLabel = new QLabel("Apellido:");
    apellidoLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* emailLabel = new QLabel("Email:");
    emailLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* passLabel = new QLabel("Nueva contraseña:");
    passLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* rolLabel = new QLabel("Rol:");
    rolLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    
    formLayout->addRow(nombreLabel, nombreInput);
    formLayout->addRow(apellidoLabel, apellidoInput);
    formLayout->addRow(emailLabel, emailInput);
    formLayout->addRow(passLabel, passInput);
    formLayout->addRow(rolLabel, rolCombo);
    
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
        QString nombre = nombreInput->text().trimmed();
        QString apellido = apellidoInput->text().trimmed();
        QString email = emailInput->text().trimmed();
        QString pass = passInput->text();
        QString rol = rolCombo->currentText();
        
        if (nombre.isEmpty() || apellido.isEmpty() || email.isEmpty())
        {
            QMessageBox::warning(&dialog, "Campos incompletos",
                               "Nombre, apellido y email son obligatorios");
            return;
        }
        
        if (!pass.isEmpty() && pass.length() < 8)
        {
            QMessageBox::warning(&dialog, "Contraseña débil",
                               "La contraseña debe tener al menos 8 caracteres");
            return;
        }

        QString errorMsg;
        // Si se deja la contraseña en blanco NO se debe pasar it->contrasena (que ya
        // es un hash) a updateUser: éste vuelve a hashearla con PasswordManager::
        // hashPassword y el usuario se queda sin poder iniciar sesión con su contraseña
        // real. updateUserKeepingPassword actualiza el resto de campos sin tocar la
        // columna Contraseña.
        bool ok = pass.isEmpty()
                  ? mDb->updateUserKeepingPassword(id, nombre, apellido, email, rol, &errorMsg)
                  : mDb->updateUser(id, nombre, apellido, email, pass, rol, &errorMsg);

        if (ok)
        {
            QMessageBox::information(&dialog, "Éxito", "Usuario actualizado correctamente");
            dialog.accept();
            loadUsers();
        }
        else
        {
            QMessageBox::critical(&dialog, "Error", "No se pudo actualizar:\n" + errorMsg);
        }
    });
    
    dialog.exec();
}

void Admin::onRemoveUserClicked()
{
    if (mUsersListWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(this, "Advertencia", "Selecciona un usuario para eliminar");
        return;
    }

    if (QMessageBox::question(this, "Confirmar eliminación",
                              "¿Estás seguro de que deseas eliminar este usuario?\n"
                              "Se eliminarán también todas sus asignaciones.",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    int id = mUsersListWidget->currentItem()->data(Qt::UserRole).toInt();
    QString errorMsg;
    if (mDb->deleteUser(id, &errorMsg))
    {
        QMessageBox::information(this, "Éxito", "Usuario eliminado correctamente");
        loadUsers();
    }
    else
    {
        QMessageBox::critical(this, "Error", "No se pudo eliminar el usuario:\n" + errorMsg);
    }
}

void Admin::onAddSubjectClicked()
{
    auto cursos = mDb->getAcademicYears();
    if (cursos.empty())
    {
        QMessageBox::warning(this, "Sin cursos académicos",
                           "Primero debes crear al menos un curso académico");
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Agregar Asignatura");
    dialog.setMinimumSize(550, 350);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    
    QLabel* titleLabel = new QLabel("Crear nueva asignatura");
    titleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());
    mainLayout->addWidget(titleLabel);
    
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    QLineEdit* nombreInput = new QLineEdit();
    nombreInput->setPlaceholderText("Ej: Programación Avanzada");
    nombreInput->setStyleSheet(mStyleManager->getLineEditStyle());
    nombreInput->setMinimumHeight(40);
    
    QTextEdit* descInput = new QTextEdit();
    descInput->setPlaceholderText("Descripción de la asignatura (opcional)");
    descInput->setStyleSheet(mStyleManager->getLineEditStyle() + " QTextEdit { padding: 8px; }");
    descInput->setMaximumHeight(100);
    
    QComboBox* cursoCombo = new QComboBox();
    for (const auto& c : cursos)
        cursoCombo->addItem(c.año, c.id);
    cursoCombo->setStyleSheet(comboBoxStyle(mStyleManager));
    
    QLabel* nombreLabel = new QLabel("Nombre:");
    nombreLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* descLabel = new QLabel("Descripción:");
    descLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* cursoLabel = new QLabel("Curso académico:");
    cursoLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    
    formLayout->addRow(nombreLabel, nombreInput);
    formLayout->addRow(descLabel, descInput);
    formLayout->addRow(cursoLabel, cursoCombo);
    
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    
    QPushButton* cancelBtn = new QPushButton("Cancelar");
    cancelBtn->setStyleSheet(mStyleManager->getButtonStyle("#555555", "#ffffff"));
    cancelBtn->setMinimumSize(120, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    
    QPushButton* saveBtn = new QPushButton("✓ Crear Asignatura");
    saveBtn->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
    saveBtn->setMinimumSize(120, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);
    
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        QString nombre = nombreInput->text().trimmed();
        QString desc = descInput->toPlainText().trimmed();
        int cursoId = cursoCombo->currentData().toInt();
        
        if (nombre.isEmpty())
        {
            QMessageBox::warning(&dialog, "Campo obligatorio",
                               "El nombre de la asignatura es obligatorio");
            return;
        }
        
        QString errorMsg;
        if (mDb->insertSubject(nombre, desc, cursoId, &errorMsg))
        {
            QMessageBox::information(&dialog, "Éxito", "Asignatura creada correctamente");
            dialog.accept();
            loadSubjects();
        }
        else
        {
            QMessageBox::critical(&dialog, "Error", "No se pudo crear:\n" + errorMsg);
        }
    });
    
    dialog.exec();
}


void Admin::onEditSubjectClicked()
{
    if (mSubjectsListWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(this, "Advertencia", "Selecciona una asignatura para editar");
        return;
    }
    
    int id = mSubjectsListWidget->currentItem()->data(Qt::UserRole).toInt();
    auto asignaturas = mDb->getSubjects();
    auto it = std::find_if(asignaturas.begin(), asignaturas.end(),
                           [id](const SubjectInfo& a){ return a.id == id; });
    if (it == asignaturas.end()) return;
    
    QDialog dialog(this);
    dialog.setWindowTitle("Editar Asignatura");
    dialog.setMinimumSize(550, 300);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    
    QLabel* titleLabel = new QLabel("Editar: " + it->nombre);
    titleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());
    mainLayout->addWidget(titleLabel);
    
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    QLineEdit* nombreInput = new QLineEdit(it->nombre);
    nombreInput->setStyleSheet(mStyleManager->getLineEditStyle());
    nombreInput->setMinimumHeight(40);
    
    QTextEdit* descInput = new QTextEdit();
    descInput->setPlainText(it->descripcion);
    descInput->setStyleSheet(mStyleManager->getLineEditStyle() + " QTextEdit { padding: 8px; }");
    descInput->setMaximumHeight(100);
    
    QLabel* nombreLabel = new QLabel("Nombre:");
    nombreLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    QLabel* descLabel = new QLabel("Descripción:");
    descLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    
    formLayout->addRow(nombreLabel, nombreInput);
    formLayout->addRow(descLabel, descInput);
    
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
        QString nombre = nombreInput->text().trimmed();
        QString desc = descInput->toPlainText().trimmed();
        
        if (nombre.isEmpty())
        {
            QMessageBox::warning(&dialog, "Campo obligatorio",
                               "El nombre no puede estar vacío");
            return;
        }
        
        QString errorMsg;
        if (mDb->updateSubject(id, nombre, desc, &errorMsg))
        {
            QMessageBox::information(&dialog, "Éxito", "Asignatura actualizada");
            dialog.accept();
            loadSubjects();
        }
        else
        {
            QMessageBox::critical(&dialog, "Error", "No se pudo actualizar:\n" + errorMsg);
        }
    });
    
    dialog.exec();
}

void Admin::onRemoveSubjectClicked()
{
    if (mSubjectsListWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(this, "Advertencia", "Selecciona una asignatura para eliminar");
        return;
    }

    if (QMessageBox::question(this, "Confirmar eliminación",
                              "¿Estás seguro?\nSe eliminarán también todas las tareas asociadas.",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    int id = mSubjectsListWidget->currentItem()->data(Qt::UserRole).toInt();
    QString errorMsg;
    if (mDb->deleteSubject(id, &errorMsg))
    {
        QMessageBox::information(this, "Éxito", "Asignatura eliminada correctamente");
        loadSubjects();
    }
    else
    {
        QMessageBox::critical(this, "Error", "No se pudo eliminar la asignatura:\n" + errorMsg);
    }
}

void Admin::onAssignTeachersClicked()
{
    if (mSubjectsListWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(this, "Advertencia", "Selecciona una asignatura primero");
        return;
    }
    
    QListWidgetItem* selectedItem = mSubjectsListWidget->currentItem();
    int subjectId = selectedItem->data(Qt::UserRole).toInt();
    int academicYearId = selectedItem->data(Qt::UserRole + 1).toInt();
    
    if (academicYearId == 0)
    {
        QMessageBox::critical(this, "Error", 
                             "La asignatura no tiene un curso académico asignado.\n"
                             "Esto no debería ocurrir. Contacta con soporte técnico.");
        return;
    }
    
    auto asignaturas = mDb->getSubjects();
    auto it = std::find_if(asignaturas.begin(), asignaturas.end(),
                           [subjectId](const SubjectInfo& a){ return a.id == subjectId; });
    if (it == asignaturas.end()) return;
    
    QString subjectName = it->nombre;
    
    auto assignedTeachers = mDb->getTeachersBySubject(subjectId, academicYearId);
    
    auto allUsers = mDb->getUsers();
    std::vector<UserInfo> allTeachers;
    for (const auto& u : allUsers)
    {
        if (u.tipoRol == "Profesor")
            allTeachers.push_back(u);
    }
    
    if (allTeachers.empty())
    {
        QMessageBox::information(this, "Sin profesores", 
                                "No hay profesores registrados en el sistema.\n"
                                "Crea primero usuarios con rol Profesor.");
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Asignar Profesores - " + subjectName);
    dialog.setMinimumSize(600, 500);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    
    QLabel* titleLabel = new QLabel("Gestionar profesores asignados a la asignatura:");
    titleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());
    mainLayout->addWidget(titleLabel);
    
    mainLayout->addSpacing(10);
    
    QLabel* assignedLabel = new QLabel("Profesores asignados:");
    assignedLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    mainLayout->addWidget(assignedLabel);
    
    QListWidget* assignedList = new QListWidget();
    assignedList->setStyleSheet(mStyleManager->getListWidgetStyle());
    assignedList->setMaximumHeight(150);
    
    for (const auto& teacher : assignedTeachers)
    {
        QString displayText = QString("%1 %2 (%3)")
                             .arg(teacher.nombre, teacher.apellido, teacher.email);
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, teacher.id);
        assignedList->addItem(item);
    }
    mainLayout->addWidget(assignedList);
    
    QPushButton* removeBtn = new QPushButton("🗑️ Desasignar Seleccionado");
    removeBtn->setStyleSheet(mStyleManager->getButtonStyle("#f44336", "#ffffff"));
    removeBtn->setMinimumHeight(35);
    removeBtn->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(removeBtn);
    
    mainLayout->addSpacing(20);
    
    QLabel* availableLabel = new QLabel("Profesores disponibles:");
    availableLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    mainLayout->addWidget(availableLabel);
    
    QListWidget* availableList = new QListWidget();
    availableList->setStyleSheet(mStyleManager->getListWidgetStyle());
    availableList->setMaximumHeight(150);
    
    for (const auto& teacher : allTeachers)
    {
        bool alreadyAssigned = std::find_if(assignedTeachers.begin(), assignedTeachers.end(),
                                           [&teacher](const UserInfo& t){ return t.id == teacher.id; })
                              != assignedTeachers.end();
        if (!alreadyAssigned)
        {
            QString displayText = QString("%1 %2 (%3)")
                                 .arg(teacher.nombre, teacher.apellido, teacher.email);
            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, teacher.id);
            availableList->addItem(item);
        }
    }
    mainLayout->addWidget(availableList);
    
    QPushButton* addBtn = new QPushButton("➕ Asignar Seleccionado");
    addBtn->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
    addBtn->setMinimumHeight(35);
    addBtn->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(addBtn);
    
    mainLayout->addSpacing(20);
    
    QPushButton* closeBtn = new QPushButton("Cerrar");
    closeBtn->setStyleSheet(mStyleManager->getButtonStyle("#2196F3", "#ffffff"));
    closeBtn->setMinimumHeight(35);
    closeBtn->setCursor(Qt::PointingHandCursor);
    mainLayout->addWidget(closeBtn);
    
    connect(addBtn, &QPushButton::clicked, [&]() {
        if (availableList->selectedItems().isEmpty())
        {
            QMessageBox::warning(&dialog, "Advertencia", "Selecciona un profesor para asignar");
            return;
        }
        
        int teacherId = availableList->currentItem()->data(Qt::UserRole).toInt();
        QString errorMsg;
        
        if (mDb->assignTeacherToSubject(teacherId, subjectId, academicYearId, &errorMsg))
        {
            QListWidgetItem* item = availableList->takeItem(availableList->currentRow());
            item->setData(Qt::UserRole, teacherId);
            assignedList->addItem(item);
            
            QMessageBox::information(&dialog, "Éxito", "Profesor asignado correctamente");
        }
        else
        {
            QMessageBox::critical(&dialog, "Error", "No se pudo asignar:\n" + errorMsg);
        }
    });
    
    connect(removeBtn, &QPushButton::clicked, [&]() {
        if (assignedList->selectedItems().isEmpty())
        {
            QMessageBox::warning(&dialog, "Advertencia", "Selecciona un profesor para desasignar");
            return;
        }
        
        int teacherId = assignedList->currentItem()->data(Qt::UserRole).toInt();
        QString errorMsg;
        
        if (mDb->removeTeacherFromSubject(teacherId, subjectId, academicYearId, &errorMsg))
        {
            QListWidgetItem* item = assignedList->takeItem(assignedList->currentRow());
            item->setData(Qt::UserRole, teacherId);
            availableList->addItem(item);
            
            QMessageBox::information(&dialog, "Éxito", "Profesor desasignado correctamente");
        }
        else
        {
            QMessageBox::critical(&dialog, "Error", "No se pudo desasignar:\n" + errorMsg);
        }
    });
    
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    dialog.exec();

    // Refresca la lista de asignaturas para reflejar los cambios de profesorado
    // hechos dentro del diálogo (asignar/desasignar no tocan mSubjectsListWidget).
    loadSubjects();
}

void Admin::onAddCourseClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Agregar Curso Académico");
    dialog.setMinimumSize(450, 250);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    
    QLabel* titleLabel = new QLabel("Crear nuevo curso académico");
    titleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());
    mainLayout->addWidget(titleLabel);
    
    QLabel* infoLabel = new QLabel("Formato: XX_XX (ejemplo: 24_25 para 2024-2025)");
    infoLabel->setStyleSheet(mStyleManager->getLabelDescriptionStyle());
    mainLayout->addWidget(infoLabel);
    
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    QLineEdit* cursoInput = new QLineEdit();
    cursoInput->setPlaceholderText("XX_XX");
    cursoInput->setMaxLength(5);
    cursoInput->setStyleSheet(mStyleManager->getLineEditStyle());
    cursoInput->setMinimumHeight(40);
    
    QLabel* cursoLabel = new QLabel("Año:");
    cursoLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    
    formLayout->addRow(cursoLabel, cursoInput);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    
    QPushButton* cancelBtn = new QPushButton("Cancelar");
    cancelBtn->setStyleSheet(mStyleManager->getButtonStyle("#555555", "#ffffff"));
    cancelBtn->setMinimumSize(120, 40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    
    QPushButton* saveBtn = new QPushButton("✓ Crear Curso");
    saveBtn->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
    saveBtn->setMinimumSize(120, 40);
    saveBtn->setCursor(Qt::PointingHandCursor);
    
    btnLayout->addWidget(cancelBtn);
    btnLayout->addWidget(saveBtn);
    mainLayout->addLayout(btnLayout);
    
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(saveBtn, &QPushButton::clicked, [&]() {
        QString curso = cursoInput->text().trimmed();
        
        if (curso.isEmpty())
        {
            QMessageBox::warning(&dialog, "Campo obligatorio",
                               "Debes especificar el año del curso");
            return;
        }
        
        QRegularExpression re("^\\d{2}_\\d{2}$");
        if (!re.match(curso).hasMatch())
        {
            QMessageBox::warning(&dialog, "Formato inválido",
                               "El formato debe ser XX_XX (ej: 24_25)");
            return;
        }
        
        QString errorMsg;
        if (mDb->insertAcademicYear(curso, &errorMsg))
        {
            QMessageBox::information(&dialog, "Éxito", "Curso académico creado");
            dialog.accept();
            loadCourses();
        }
        else
        {
            QMessageBox::critical(&dialog, "Error", "No se pudo crear:\n" + errorMsg);
        }
    });
    
    dialog.exec();
}


void Admin::onEditCourseClicked()
{
    if (mCoursesListWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(this, "Advertencia", "Selecciona un curso para editar");
        return;
    }
    
    int id = mCoursesListWidget->currentItem()->data(Qt::UserRole).toInt();
    auto cursos = mDb->getAcademicYears();
    auto it = std::find_if(cursos.begin(), cursos.end(),
                           [id](const AcademicYearInfo& c){ return c.id == id; });
    if (it == cursos.end()) return;
    
    QDialog dialog(this);
    dialog.setWindowTitle("Editar Curso Académico");
    dialog.setMinimumSize(450, 250);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(&dialog);
    mainLayout->setSpacing(15);
    
    QLabel* titleLabel = new QLabel("Editar curso: " + it->año);
    titleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());
    mainLayout->addWidget(titleLabel);
    
    QLabel* infoLabel = new QLabel("Formato: XX_XX");
    infoLabel->setStyleSheet(mStyleManager->getLabelDescriptionStyle());
    mainLayout->addWidget(infoLabel);
    
    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    
    QLineEdit* cursoInput = new QLineEdit(it->año);
    cursoInput->setMaxLength(5);
    cursoInput->setStyleSheet(mStyleManager->getLineEditStyle());
    cursoInput->setMinimumHeight(40);
    
    QLabel* cursoLabel = new QLabel("Año:");
    cursoLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    
    formLayout->addRow(cursoLabel, cursoInput);
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
        QString curso = cursoInput->text().trimmed();
        
        if (curso.isEmpty())
        {
            QMessageBox::warning(&dialog, "Campo obligatorio",
                               "El año no puede estar vacío");
            return;
        }
        
        QRegularExpression re("^\\d{2}_\\d{2}$");
        if (!re.match(curso).hasMatch())
        {
            QMessageBox::warning(&dialog, "Formato inválido",
                               "El formato debe ser XX_XX");
            return;
        }
        
        QString errorMsg;
        if (mDb->updateAcademicYear(id, curso, &errorMsg))
        {
            QMessageBox::information(&dialog, "Éxito", "Curso actualizado");
            dialog.accept();
            loadCourses();
        }
        else
        {
            QMessageBox::critical(&dialog, "Error", "No se pudo actualizar:\n" + errorMsg);
        }
    });
    
    dialog.exec();
}


void Admin::onRemoveCourseClicked()
{
    if (mCoursesListWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(this, "Advertencia", "Selecciona un curso para eliminar");
        return;
    }

    int id = mCoursesListWidget->currentItem()->data(Qt::UserRole).toInt();

    auto asignaturas = mDb->getSubjectsByAcademicYear(id);

    if (!asignaturas.empty())
    {
        QString listaAsig;
        for (const auto& a : asignaturas)
            listaAsig += QString("  • %1\n").arg(a.nombre);

        QMessageBox msgBox(this);
        msgBox.setWindowTitle("No se puede eliminar el curso");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(
            QString("Este curso tiene <b>%1 asignatura(s) vinculada(s)</b>.<br>"
                    "Debes eliminarlas o reasignarlas antes de eliminar el curso.")
            .arg(asignaturas.size())
        );
        msgBox.setDetailedText(
            QString("Asignaturas que dependen de este curso:\n\n") + listaAsig
        );
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }

    if (QMessageBox::question(this, "Confirmar eliminación",
                              "¿Eliminar este curso académico?\n"
                              "Esta acción no se puede deshacer.",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    QString errorMsg;
    if (mDb->deleteAcademicYear(id, &errorMsg))
    {
        QMessageBox::information(this, "Éxito", "Curso académico eliminado correctamente");
        loadCourses();
    }
    else
    {
        QMessageBox::critical(this, "Error", "No se pudo eliminar el curso:\n" + errorMsg);
    }
}
