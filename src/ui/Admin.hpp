#ifndef ADMIN_HPP
#define ADMIN_HPP

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include "../model/DataBaseManager.hpp"
#include "StyleManager.hpp"

class Admin : public QMainWindow
{
    Q_OBJECT

public:
    explicit Admin(QWidget *parent = nullptr);
    ~Admin();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onLogoutClicked();

    void onUsersFilterChanged(const QString& text);
    void onSubjectsFilterChanged(const QString& text);
    void onCoursesFilterChanged(const QString& text);

    void onUserSelected();
    void onSubjectSelected();
    void onCourseSelected();

    void onAddUserClicked();
    void onEditUserClicked();
    void onRemoveUserClicked();

    void onAddSubjectClicked();
    void onEditSubjectClicked();
    void onRemoveSubjectClicked();
    void onAssignTeachersClicked();

    void onAddCourseClicked();
    void onEditCourseClicked();
    void onRemoveCourseClicked();

private:
    void setupUI();
    void setupLayout();
    void applyStyles();
    void setupConnections();

    void loadUsers();
    void loadSubjects();
    void loadCourses();
    void refreshAllData();

    void filterListWidget(QListWidget* listWidget, const QString& text);
    void clearOtherSelections(QListWidget* current);

    QLabel*      mTitleLabel;
    QPushButton* mLogoutButton;

    QLineEdit*   mUsersSearchInput;
    QListWidget* mUsersListWidget;
    QPushButton* mAddUserButton;
    QPushButton* mEditUserButton;
    QPushButton* mRemoveUserButton;

    QLineEdit*   mSubjectsSearchInput;
    QListWidget* mSubjectsListWidget;
    QPushButton* mAddSubjectButton    = nullptr;
    QPushButton* mEditSubjectButton   = nullptr;
    QPushButton* mRemoveSubjectButton = nullptr;
    QPushButton* mAssignTeachersButton = nullptr;

    QLineEdit*   mCoursesSearchInput;
    QListWidget* mCoursesListWidget;
    QPushButton* mAddCourseButton;
    QPushButton* mEditCourseButton;
    QPushButton* mRemoveCourseButton;

    DataBaseManager* mDb;
    StyleManager*    mStyleManager;
};

#endif // ADMIN_HPP
