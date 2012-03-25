#include <QMainWindow>

class QPushButton;
namespace Ui {
class Tool;
}

namespace QCloud {
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow (QWidget* parent = 0, Qt::WindowFlags flags = 0);
    virtual ~MainWindow();

private slots:
    void addAccountButtonClicked();
    void deleteAccountButtonClicked();
private:
    QWidget* m_widget;
    Ui::Tool* m_ui;
    QPushButton* m_addAccountButton;
    QPushButton* m_deleteAccountButton;
};
}