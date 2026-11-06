#pragma once

#include "../engine/AclTypes.hpp"
#include "../model/AclTreeModel.hpp"
#include "../model/AceTableModel.hpp"

#include <QMainWindow>
#include <QTabWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeView>
#include <QTableView>
#include <QLabel>
#include <QProgressBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QPointer>
#include <QList>

class QCloseEvent;

namespace AclAuditor::UI {

class VisualExplanationWidget;
class RiskDashboardWidget;
class ServicesSharesWidget;
class SnapshotDiffWidget;
class ReportExportDialog;
class RemediationDialog;
class AboutDialog;
class ManualDialog;
class MultiColumnSortFilterProxyModel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    void openPathForAudit(const QString& path);
    void retranslateUi();

public slots:
    void onScanClicked();
    void onBrowseFolderClicked();
    void onTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
    void onExplainCurrentObject();
    void onTakeSnapshot();
    void onOpenReportDialog();
    void onOpenRemediationDialog();
    void onToggleExplorerIntegration();
    void onOpenAbout();
    void onOpenManual();
    void applyTheme(bool isDark);

protected:
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void registerSecondaryWindow(QWidget* window);
    void setupUi();
    void setupMenus();
    void setupStatusBar();
    void updateStatusBarInfo();
    void updateStatusMessage();

    // Navigation & Inspection Tab UI
    QLabel* m_lblPathPrefix{nullptr};
    QLineEdit* m_pathEdit{nullptr};
    QPushButton* m_btnBrowse{nullptr};
    QPushButton* m_btnScan{nullptr};
    QPushButton* m_btnExplain{nullptr};
    QPushButton* m_btnSnapshot{nullptr};
    QPushButton* m_btnReport{nullptr};

    QTreeView* m_treeView{nullptr};
    QLineEdit* m_aceSearchEdit{nullptr};
    QTableView* m_aceTableView{nullptr};
    Model::AclTreeModel* m_treeModel{nullptr};
    Model::AceTableModel* m_aceModel{nullptr};
    MultiColumnSortFilterProxyModel* m_aceProxyModel{nullptr};

    QLabel* m_lblCurrentObjectInfo{nullptr};

    // Sub-widgets
    QTabWidget* m_mainTabWidget{nullptr};
    VisualExplanationWidget* m_visualExplanationWidget{nullptr};
    RiskDashboardWidget* m_riskDashboardWidget{nullptr};
    ServicesSharesWidget* m_servicesSharesWidget{nullptr};
    SnapshotDiffWidget* m_snapshotDiffWidget{nullptr};

    // Menus and Actions for dynamic translation
    QMenu* m_menuArquivo{nullptr};
    QMenu* m_menuAuditoria{nullptr};
    QMenu* m_menuConfig{nullptr};
    QMenu* m_menuIdiomas{nullptr};
    QMenu* m_menuFerramentas{nullptr};
    QMenu* m_menuAjuda{nullptr};

    QAction* m_actionBrowse{nullptr};
    QAction* m_actionScan{nullptr};
    QAction* m_actionSnapshot{nullptr};
    QAction* m_actionReport{nullptr};
    QAction* m_actionExit{nullptr};

    QAction* m_actionExplain{nullptr};
    QAction* m_actionRemediation{nullptr};
    QAction* m_actionAuditServices{nullptr};
    QAction* m_actionAuditShares{nullptr};

    QActionGroup* m_langActionGroup{nullptr};
    QAction* m_actionPtBr{nullptr};
    QAction* m_actionEnUs{nullptr};

    QAction* m_actionExplorerIntegration{nullptr};
    QAction* m_actionTheme{nullptr};

    QAction* m_actionManual{nullptr};
    QAction* m_actionAbout{nullptr};

    // Status bar labels
    QLabel* m_statusElevated{nullptr};
    QLabel* m_statusMemory{nullptr};
    QLabel* m_statusObjectsCount{nullptr};
    QProgressBar* m_progressBar{nullptr};

    enum class StatusState {
        Ready,
        DirectorySelected,
        Scanning,
        ScanCompleted
    };
    StatusState m_statusState{StatusState::Ready};
    QString m_lastStatusPath;
    size_t m_lastAuditedCount{0};

    Engine::FileSecurityNode m_currentRootNode;
    std::vector<Engine::RiskFinding> m_currentFindings;
    QString m_selectedObjectPath;

    // Gerenciamento de Janelas Modeless / Top-Level
    QPointer<ReportExportDialog> m_reportDialog;
    QPointer<RemediationDialog> m_remediationDialog;
    QPointer<AboutDialog> m_aboutDialog;
    QPointer<ManualDialog> m_manualDialog;
    QList<QPointer<QWidget>> m_secondaryWindows;
};

} // namespace AclAuditor::UI
