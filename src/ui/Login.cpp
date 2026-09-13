#include "Login.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QResizeEvent>
#include <QFrame>
#include "../model/Logger.hpp"

Login::Login(QWidget *parent)
    : QMainWindow(parent)
{
    mDb = DataBaseManager::getInstance();
    mStyleManager = new StyleManager();

    setWindowTitle("CodeIntegrity - Inicio de Sesión");
    setMinimumSize(500, 600);
    setWindowIcon(QIcon(":/icons/app_icon.png"));

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    setupUi();
    setupLayout();
    applyStyles();
    setupConnections();
    loadTestCredentials();

    Logger::log(Logger::INFO, "Ventana de Login inicializada");
}

Login::~Login()
{
    delete mStyleManager;
    Logger::log(Logger::INFO, "Ventana de Login cerrada");
}

void Login::setupUi()
{
    mTitleLabel = new QLabel("CodeIntegrity");
    mTitleLabel->setAlignment(Qt::AlignCenter);

    mSubtitleLabel = new QLabel("Garantizando la integridad del CÓDIGO");
    mSubtitleLabel->setAlignment(Qt::AlignCenter);

    QLabel* sectionLabel = new QLabel("INICIO DE SESIÓN");
    sectionLabel->setAlignment(Qt::AlignCenter);

    QLabel* emailLabel = new QLabel("Correo Electrónico:");
    QLabel* passwordLabel = new QLabel("Contraseña:");

    mEmailInput = new QLineEdit();
    mEmailInput->setPlaceholderText("tu.email@ejemplo.com");
    mEmailInput->setClearButtonEnabled(true);

    mPasswordInput = new QLineEdit();
    mPasswordInput->setPlaceholderText("Introduce tu contraseña");
    mPasswordInput->setEchoMode(QLineEdit::Password);
    mPasswordInput->setClearButtonEnabled(true);

    mLoginButton = new QPushButton("Iniciar Sesión");
    mLoginButton->setMinimumHeight(40);
    mLoginButton->setCursor(Qt::PointingHandCursor);

    QLabel* footerLabel = new QLabel("© 2026 CodeIntegrity. Todos los derechos reservados.");
    footerLabel->setAlignment(Qt::AlignCenter);

    centralWidget()->setProperty("emailLabel", QVariant::fromValue(emailLabel));
    centralWidget()->setProperty("passwordLabel", QVariant::fromValue(passwordLabel));
    centralWidget()->setProperty("sectionLabel", QVariant::fromValue(sectionLabel));
    centralWidget()->setProperty("footerLabel", QVariant::fromValue(footerLabel));
}

void Login::setupLayout()
{
    QWidget* central = centralWidget();

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(12);

    mainLayout->addSpacing(20);

    mainLayout->addWidget(mTitleLabel);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(mSubtitleLabel);
    mainLayout->addSpacing(30);

    QFrame* separator1 = new QFrame();
    separator1->setFrameShape(QFrame::HLine);
    separator1->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator1);

    mainLayout->addSpacing(20);

    QLabel* sectionLabel = qvariant_cast<QLabel*>(central->property("sectionLabel"));
    mainLayout->addWidget(sectionLabel);

    mainLayout->addSpacing(20);

    QLabel* emailLabel = qvariant_cast<QLabel*>(central->property("emailLabel"));
    mainLayout->addWidget(emailLabel);
    mainLayout->addWidget(mEmailInput);

    mainLayout->addSpacing(16);

    QLabel* passwordLabel = qvariant_cast<QLabel*>(central->property("passwordLabel"));
    mainLayout->addWidget(passwordLabel);
    mainLayout->addWidget(mPasswordInput);

    mainLayout->addSpacing(24);

    mainLayout->addWidget(mLoginButton);

    mainLayout->addStretch();

    QFrame* separator2 = new QFrame();
    separator2->setFrameShape(QFrame::HLine);
    separator2->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(separator2);

    mainLayout->addSpacing(12);

    QLabel* footerLabel = qvariant_cast<QLabel*>(central->property("footerLabel"));
    mainLayout->addWidget(footerLabel);

    mainLayout->addSpacing(10);
}

void Login::applyStyles()
{
    centralWidget()->setStyleSheet
        (
            QString("QWidget { background-color: %1; color: %2; }")
                .arg(StyleManager::getDarkBgPrimary(), StyleManager::getTextPrimary()) +
            mStyleManager->getLineEditStyle() +
            mStyleManager->getButtonStyle() +
            QString("QFrame { color: %1; }").arg(StyleManager::getDarkBorder())
            );

    mTitleLabel->setStyleSheet(mStyleManager->getLabelTitleStyle());

    mSubtitleLabel->setStyleSheet(mStyleManager->getLabelSubtitleStyle());

    QWidget* central = centralWidget();

    if (auto sectionLabel = qvariant_cast<QLabel*>(central->property("sectionLabel")))
    {
        sectionLabel->setStyleSheet(mStyleManager->getLabelSectionStyle());
    }

    if (auto emailLabel = qvariant_cast<QLabel*>(central->property("emailLabel")))
    {
        emailLabel->setStyleSheet(mStyleManager->getLabelDescriptionStyle());
    }

    if (auto passwordLabel = qvariant_cast<QLabel*>(central->property("passwordLabel")))
    {
        passwordLabel->setStyleSheet(mStyleManager->getLabelDescriptionStyle());
    }

    if (auto footerLabel = qvariant_cast<QLabel*>(central->property("footerLabel")))
    {
        footerLabel->setStyleSheet(mStyleManager->getLabelDescriptionStyle());
    }

    mEmailInput->setStyleSheet(mStyleManager->getLineEditStyle());
    mPasswordInput->setStyleSheet(mStyleManager->getLineEditStyle());

    mLoginButton->setStyleSheet(mStyleManager->getButtonStyle("#4CAF50", "#ffffff"));
}

void Login::setupConnections()
{
    connect(mLoginButton, &QPushButton::clicked, this, &Login::onLoginClicked);
    connect(mEmailInput, &QLineEdit::textChanged, this, &Login::onEmailChanged);
    connect(mPasswordInput, &QLineEdit::textChanged, this, &Login::onPasswordChanged);
    connect(mPasswordInput, &QLineEdit::returnPressed, this, &Login::onLoginClicked);
}

void Login::loadTestCredentials()
{
#ifdef QT_DEBUG
    mEmailInput->setText("carmen.molina@admin.edu");
    mPasswordInput->setText("admin123456");

    // mEmailInput->setText("alejandro.dominguez1@profesor.edu");
    // mPasswordInput->setText("profesor123");

    Logger::log(Logger::WARNING, "Credenciales de prueba cargadas (MODO DEBUG)");
#endif
}

bool Login::validateInput(const QString& email, const QString& password)
{
    clearError();
    setInputError(mEmailInput, false);
    setInputError(mPasswordInput, false);

    mEmailHasError = false;
    mPasswordHasError = false;

    if (email.isEmpty())
    {
        setInputError(mEmailInput, true);
        mEmailHasError = true;
        mEmailInput->setFocus();
        return false;
    }

    if (password.isEmpty())
    {
        setInputError(mPasswordInput, true);
        mPasswordHasError = true;
        mPasswordInput->setFocus();
        return false;
    }

    if (!email.contains("@") || !email.contains("."))
    {
        setInputError(mEmailInput, true);
        mEmailHasError = true;
        mEmailInput->setFocus();
        Logger::log(Logger::WARNING, QString("Email inválido: %1").arg(email));
        return false;
    }

    return true;
}

void Login::onLoginClicked()
{
    QString email = mEmailInput->text().trimmed();
    QString password = mPasswordInput->text();

    if (!validateInput(email, password)) return;

    Logger::log(Logger::INFO, QString("Intento de login: %1").arg(email));

    mLoginButton->setEnabled(false);
    mLoginButton->setText("Autenticando...");

    QString roleStr = mDb->authenticateUser(email, password);

    if (roleStr != "error")
    {
        UserRole role = stringToRole(roleStr);
        int userId = mDb->getCurrentUserId(email);

        Logger::log(Logger::INFO, QString("Usuario autenticado: %1 (rol: %2)").arg(email, roleStr));

        clearError();

        if (role == UserRole::Administrator)
        {
            mAdminWindow = std::make_unique<Admin>(this);
            mAdminWindow->show();
            Logger::log(Logger::INFO, "Ventana Admin abierta");
        }
        else if (role == UserRole::Teacher)
        {
            mTeacherWindow = std::make_unique<Teacher>(userId, email, this);
            mTeacherWindow->show();
            Logger::log(Logger::INFO, QString("Ventana Teacher abierta para usuario %1").arg(userId));
        }

        mPasswordInput->clear();
        hide();
    }
    else
    {
        Logger::log(Logger::WARNING, QString("Autenticación fallida para: %1").arg(email));

        setInputError(mEmailInput, true);
        setInputError(mPasswordInput, true);

        mEmailHasError = true;
        mPasswordHasError = true;

        mPasswordInput->clear();
        mPasswordInput->setFocus();
    }

    mLoginButton->setEnabled(true);
    mLoginButton->setText("Iniciar Sesión");
}

void Login::clearError()
{
    setInputError(mEmailInput, false);
    setInputError(mPasswordInput, false);

    mEmailHasError = false;
    mPasswordHasError = false;
}

void Login::setInputError(QLineEdit* input, bool hasError)
{
    if (!input) return;

    if (hasError)
    {
        input->setStyleSheet(mStyleManager->getLineEditErrorStyle());
    }
    else
    {
        input->setStyleSheet(mStyleManager->getLineEditStyle());
    }
}

void Login::onEmailChanged()
{
    if (!mEmailInput->text().isEmpty() && mEmailHasError)
    {
        setInputError(mEmailInput, false);
        mEmailHasError = false;
    }
}

void Login::onPasswordChanged()
{
    if (!mPasswordInput->text().isEmpty() && mPasswordHasError)
    {
        setInputError(mPasswordInput, false);
        mPasswordHasError = false;
    }
}

void Login::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
}
