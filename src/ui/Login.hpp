#ifndef LOGIN_HPP
#define LOGIN_HPP

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "Teacher.hpp"
#include "Admin.hpp"
#include "../model/DataBaseManager.hpp"
#include "StyleManager.hpp"

class Login : public QMainWindow
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLoginClicked();
    void onEmailChanged();
    void onPasswordChanged();

private:
    QLineEdit* mEmailInput;
    QLineEdit* mPasswordInput;
    QPushButton* mLoginButton;
    QLabel* mTitleLabel;
    QLabel* mSubtitleLabel;

    std::unique_ptr<Admin> mAdminWindow;
    std::unique_ptr<Teacher> mTeacherWindow;
    DataBaseManager* mDb;
    StyleManager* mStyleManager;

    bool mEmailHasError = false;
    bool mPasswordHasError = false;

    void setupUi();
    void applyStyles();
    void setupConnections();
    void setupLayout();
    void loadTestCredentials();
    bool validateInput(const QString& email, const QString& password);
    void clearError();
    void setInputError(QLineEdit* input, bool hasError);
};

#endif
