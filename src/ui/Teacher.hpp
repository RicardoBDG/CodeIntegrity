#ifndef TEACHER_HPP
#define TEACHER_HPP

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QButtonGroup>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QDateTimeEdit>
#include <memory>
#include <vector>

#include "../model/DataBaseManager.hpp"
#include "StyleManager.hpp"
#include "../model/DatabaseTypes.hpp"

class SubmissionManager;
class AnalysisCoordinator;
class UIComponentFactory;

class Teacher : public QMainWindow
{
    Q_OBJECT

public:
    explicit Teacher(int userId, const QString& email, QWidget *parent = nullptr);
    ~Teacher();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void logoutButtonClicked();
    void filtrarAsignaturas(const QString& texto);
    void filtrarTareas(const QString& texto);
    void actualizarTareaSeleccionada(const QString& nombreTarea);
    void anadirTarea();
    void checkRunAnalysis();

    void onSubmissionExtractionFinished(bool success, const QString& message);
    void onAnalysisFinished(bool success, const QString& reportUrl);
    void onAnalysisError(const QString& errorMessage);

private:
    void setupUI();
    void setupLayout();
    void setupConnections();
    void setupAnalysisTools();
    void applyStyles();

    void cargarAsignaturasProfesor();
    void cargarTareasAsignatura(int idAsignatura);
    void crearWidgetAsignatura(const SubjectInfo& asignatura);
    void crearWidgetTarea(const TaskInfo& tarea);
    void eliminarTarea(int idTarea, const QString& nombreTarea);
    void editarTarea(int idTarea, const QString& nombreTarea, const QString& descripcionTarea);
    void seleccionarTarea(const QString& nombreTarea, int idTarea, QPushButton* subirBtnActivo);
    void subirArchivoTarea(int idTarea, const QString& nombreTarea);
    void handleToolSelection(bool checked);
    void actualizarEstadoBotonAnalisis();

    QLabel*      mTitleLabel      = nullptr;
    QPushButton* mLogoutButton    = nullptr;

    QLineEdit*   mSignatureSearchBar     = nullptr;
    QListWidget* mAsignaturasListWidget  = nullptr;

    QLineEdit*   mTaskSearchBar         = nullptr;
    QListWidget* mTareasListWidget      = nullptr;
    QPushButton* mAddTaskButton         = nullptr;

    QTextEdit*    mTaskSelectedTextEdit   = nullptr;
    QCheckBox*    mMossCheckBox           = nullptr;
    QCheckBox*    mJplagCheckBox          = nullptr;
    QButtonGroup* mAnalysisToolGroup      = nullptr;
    QHBoxLayout*  mCheckBoxLayout         = nullptr;
    QPushButton*  mRunAnalysisButton      = nullptr;

    DataBaseManager* mDb           = nullptr;
    StyleManager*    mStyleManager = nullptr;

    std::unique_ptr<SubmissionManager>    mSubmissionManager;
    std::unique_ptr<AnalysisCoordinator>  mAnalysisCoordinator;
    std::unique_ptr<UIComponentFactory>   mUIFactory;

    int     mUserId;
    QString mEmailProfesor;
    QString mCurrentSubmissionPath;
    QString mTareaSeleccionada;
    int     mAsignaturaSeleccionada = -1;

    // Identifican de forma inequívoca a qué tarea pertenece el archivo actualmente
    // extraído en mCurrentSubmissionPath, para no poder lanzar nunca un análisis
    // con los archivos de una tarea distinta a la que aparece como seleccionada.
    int mTareaSeleccionadaId     = -1;  // tarea marcada con "Seleccionar" ahora mismo
    int mCurrentSubmissionTaskId = -1;  // tarea a la que pertenece mCurrentSubmissionPath
    int mUploadingTaskId         = -1;  // tarea de la subida en curso (mientras se extrae)

    // Botón "Subir" de cada fila de tarea visible, para poder deshabilitar todos
    // menos el de la tarea seleccionada (ver seleccionarTarea).
    std::vector<QPushButton*> mBotonesSubir;

    std::vector<SubjectInfo> mAsignaturas;
    std::vector<TaskInfo>    mTareasActuales;
};

#endif // TEACHER_HPP
