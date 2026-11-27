#pragma once

#include "../engine/AclTypes.hpp"
#include "../model/DiffTableModel.hpp"
#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QLabel>
#include <QGroupBox>

class QLineEdit;
class QProgressBar;
class QFrame;

namespace AclAuditor::UI {

class MultiColumnSortFilterProxyModel;

class SnapshotDiffWidget : public QWidget {
    Q_OBJECT

public:
    explicit SnapshotDiffWidget(QWidget* parent = nullptr);
    ~SnapshotDiffWidget() override = default;

    void refreshSnapshotList();

public slots:
    void onCompareClicked();
    void onExportEvidenceClicked();
    void onImportSnapshotClicked();
    void applyTheme(bool isDark);
    void retranslateUi();

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUi();
    void updateSummaryLabels();
    void onCompareFinished(bool success, Engine::SnapshotDiffResult diff, std::string baseHash, std::string currHash, qint64 elapsedMs);

    QGroupBox* m_selectGroup{nullptr};
    QLabel* m_lblBaselinePrompt{nullptr};
    QLabel* m_lblCurrentPrompt{nullptr};
    QComboBox* m_comboBaseline{nullptr};
    QComboBox* m_comboCurrent{nullptr};
    QPushButton* m_btnCompare{nullptr};
    QPushButton* m_btnExport{nullptr};
    QPushButton* m_btnImport{nullptr};

    QFrame* m_summaryCard{nullptr};
    QProgressBar* m_progressBar{nullptr};
    QLabel* m_lblDiffSummary{nullptr};
    QLabel* m_lblSha256Evidence{nullptr};

    QLineEdit* m_diffSearchEdit{nullptr};
    QTableView* m_diffTableView{nullptr};
    Model::DiffTableModel* m_diffModel{nullptr};
    MultiColumnSortFilterProxyModel* m_diffProxyModel{nullptr};

    bool m_isComparing{false};
    bool m_hasDiffResult{false};
    qint64 m_lastElapsedMs{0};
    Engine::SnapshotDiffResult m_lastDiff;
    std::string m_lastBaseHash;
    std::string m_lastCurrHash;
    std::vector<Engine::SnapshotRecord> m_availableSnapshots;
};

} // namespace AclAuditor::UI
