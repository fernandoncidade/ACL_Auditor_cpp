#pragma once

#include "../engine/AclTypes.hpp"
#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTextBrowser>
#include <QLabel>
#include <QGroupBox>

namespace AclAuditor::UI {

class VisualExplanationWidget : public QWidget {
    Q_OBJECT

public:
    explicit VisualExplanationWidget(QWidget* parent = nullptr);
    ~VisualExplanationWidget() override = default;

    void setTargetObject(const QString& objectPath);
    void evaluateAccess();
    void retranslateUi();

public slots:
    void applyTheme(bool isDark);

signals:
    void auditRequested(const QString& path, const QString& user);

protected:
    void changeEvent(QEvent* event) override;

private slots:
    void onCalculateClicked();

private:
    void setupUi();
    void renderExplanationTree(const Engine::EffectiveAccessResult& result);
    void renderVerdictCards(const Engine::EffectiveAccessResult& result);
    void renderExplanatoryText(const Engine::EffectiveAccessResult& result);

    QString m_currentPath;

    QGroupBox* m_inputGroup{nullptr};
    QLabel* m_lblPathTitle{nullptr};
    QLabel* m_lblUserTitle{nullptr};
    QLineEdit* m_pathEdit{nullptr};
    QComboBox* m_userCombo{nullptr};
    QPushButton* m_btnCalculate{nullptr};

    // Verdict header
    QFrame* m_verdictCard{nullptr};
    QLabel* m_verdictLabel{nullptr};
    QLabel* m_rightsSummaryLabel{nullptr};

    // Quick checkboxes / indicators for standard operations
    QLabel* m_lblRead{nullptr};
    QLabel* m_lblWrite{nullptr};
    QLabel* m_lblExecute{nullptr};
    QLabel* m_lblDelete{nullptr};
    QLabel* m_lblChangePerms{nullptr};

    // Visual tree
    QGroupBox* m_treeGroup{nullptr};
    QTreeWidget* m_chainTree{nullptr};

    // Natural text explanation
    QGroupBox* m_textGroup{nullptr};
    QTextBrowser* m_explanationText{nullptr};

    Engine::EffectiveAccessResult m_lastResult;
};

} // namespace AclAuditor::UI
