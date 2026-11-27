#include "SnapshotDiffWidget.hpp"
#include "ThemeManager.hpp"
#include "TableEnhancer.hpp"
#include "../engine/SnapshotEngine.hpp"
#include "../language/GerenciadorTraducao.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QEvent>
#include <QElapsedTimer>
#include <QApplication>
#include <QProgressBar>
#include <QThread>

namespace AclAuditor::UI {

SnapshotDiffWidget::SnapshotDiffWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    retranslateUi();
    refreshSnapshotList();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &SnapshotDiffWidget::applyTheme);
    applyTheme(ThemeManager::instance().isDark());

    connect(&Language::GerenciadorTraducao::instance(), &Language::GerenciadorTraducao::idioma_alterado,
            this, &SnapshotDiffWidget::retranslateUi);
}

void SnapshotDiffWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

void SnapshotDiffWidget::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // Snapshot selection group
    m_selectGroup = new QGroupBox(this);
    auto* selLayout = new QGridLayout(m_selectGroup);
    selLayout->setContentsMargins(10, 10, 10, 10);
    selLayout->setSpacing(8);

    m_lblBaselinePrompt = new QLabel(m_selectGroup);
    selLayout->addWidget(m_lblBaselinePrompt, 0, 0);
    m_comboBaseline = new QComboBox(m_selectGroup);
    selLayout->addWidget(m_comboBaseline, 0, 1);

    m_lblCurrentPrompt = new QLabel(m_selectGroup);
    selLayout->addWidget(m_lblCurrentPrompt, 1, 0);
    m_comboCurrent = new QComboBox(m_selectGroup);
    selLayout->addWidget(m_comboCurrent, 1, 1);

    m_btnCompare = new QPushButton(m_selectGroup);
    m_btnCompare->setStyleSheet(QStringLiteral("font-weight: bold; padding: 6px 14px;"));
    connect(m_btnCompare, &QPushButton::clicked, this, &SnapshotDiffWidget::onCompareClicked);
    selLayout->addWidget(m_btnCompare, 1, 2);

    auto* btnRow = new QHBoxLayout();
    m_btnExport = new QPushButton(m_selectGroup);
    connect(m_btnExport, &QPushButton::clicked, this, &SnapshotDiffWidget::onExportEvidenceClicked);
    btnRow->addWidget(m_btnExport);

    m_btnImport = new QPushButton(m_selectGroup);
    connect(m_btnImport, &QPushButton::clicked, this, &SnapshotDiffWidget::onImportSnapshotClicked);
    btnRow->addWidget(m_btnImport);
    btnRow->addStretch();

    selLayout->addLayout(btnRow, 2, 1);

    mainLayout->addWidget(m_selectGroup);

    // Summary Card
    m_summaryCard = new QFrame(this);
    m_summaryCard->setFrameShape(QFrame::StyledPanel);
    auto* sumLayout = new QVBoxLayout(m_summaryCard);
    sumLayout->setContentsMargins(8, 8, 8, 8);
    sumLayout->setSpacing(6);

    m_progressBar = new QProgressBar(m_summaryCard);
    m_progressBar->setMaximumHeight(4);
    m_progressBar->setTextVisible(false);
    m_progressBar->setVisible(false);
    sumLayout->addWidget(m_progressBar);

    m_lblDiffSummary = new QLabel(m_summaryCard);
    QFont f = m_lblDiffSummary->font();
    f.setBold(true);
    m_lblDiffSummary->setFont(f);
    sumLayout->addWidget(m_lblDiffSummary);

    m_lblSha256Evidence = new QLabel(m_summaryCard);
    sumLayout->addWidget(m_lblSha256Evidence);

    mainLayout->addWidget(m_summaryCard);

    // Diff Table Search
    m_diffSearchEdit = new QLineEdit(this);
    m_diffSearchEdit->setClearButtonEnabled(true);
    m_diffSearchEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Buscar nas diferenças do snapshot..."));
    connect(m_diffSearchEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        if (m_diffProxyModel) {
            m_diffProxyModel->setFilterPattern(text);
        }
    });
    mainLayout->addWidget(m_diffSearchEdit);

    // Diff Table
    m_diffModel = new Model::DiffTableModel(this);
    m_diffProxyModel = new MultiColumnSortFilterProxyModel(this);
    m_diffProxyModel->setSourceModel(m_diffModel);

    m_diffTableView = new QTableView(this);
    TableEnhancer::enhanceTableView(
        m_diffTableView,
        m_diffProxyModel,
        QStringLiteral("auditoria_snapshots_diff"),
        { 180, 320, 300, 200, 200 }
    );

    mainLayout->addWidget(m_diffTableView, 1);
}

void SnapshotDiffWidget::retranslateUi() {
    if (m_selectGroup) {
        m_selectGroup->setTitle(Language::GerenciadorTraducao::trText("Comparação Temporal de Snapshots de Segurança (Auditoria de Mudanças)"));
    }
    if (m_lblBaselinePrompt) {
        m_lblBaselinePrompt->setText(Language::GerenciadorTraducao::trText("Snapshot Base (Anterior / Referência):"));
    }
    if (m_lblCurrentPrompt) {
        m_lblCurrentPrompt->setText(Language::GerenciadorTraducao::trText("Snapshot Atual (Recente / Comparativo):"));
    }
    if (m_btnCompare) {
        m_btnCompare->setText(Language::GerenciadorTraducao::trText("Comparar Snapshots (Calcular Diff)"));
    }
    if (m_btnExport) {
        m_btnExport->setText(Language::GerenciadorTraducao::trText("Exportar Evidência JSON Assinada"));
    }
    if (m_btnImport) {
        m_btnImport->setText(Language::GerenciadorTraducao::trText("Importar Arquivo de Evidência"));
    }
    if (m_diffSearchEdit) {
        m_diffSearchEdit->setPlaceholderText(Language::GerenciadorTraducao::trText("Buscar nas diferenças do snapshot..."));
    }

    if (m_diffModel) {
        m_diffModel->refreshTranslations();
    }

    refreshSnapshotList();
    updateSummaryLabels();
    applyTheme(ThemeManager::instance().isDark());
}

void SnapshotDiffWidget::updateSummaryLabels() {
    if (m_isComparing) {
        m_lblDiffSummary->setText(Language::GerenciadorTraducao::trText("Comparando snapshots em segundo plano..."));
        if (m_lblSha256Evidence) m_lblSha256Evidence->clear();
        return;
    }

    if (m_hasDiffResult && !m_lastDiff.changes.empty()) {
        m_lblDiffSummary->setText(QStringLiteral(
            "%1: %2 %3, %4 %5 %6. (%7 ms)"
        ).arg(Language::GerenciadorTraducao::trText("Comparação concluída"))
         .arg(m_lastDiff.totalChanges)
         .arg(Language::GerenciadorTraducao::trText("alteração(ões) identificada(s)"))
         .arg(Language::GerenciadorTraducao::trText("sendo"))
         .arg(m_lastDiff.totalNewRisks)
         .arg(Language::GerenciadorTraducao::trText("novo(s) risco(s) de privilégio ou exposição"))
         .arg(m_lastElapsedMs));

        m_lblSha256Evidence->setText(QStringLiteral(
            "%1: %2 | %3: %4"
        ).arg(Language::GerenciadorTraducao::trText("Hash de Evidência Base"),
              QString::fromStdString(m_lastBaseHash),
              Language::GerenciadorTraducao::trText("Hash de Evidência Atual"),
              QString::fromStdString(m_lastCurrHash)));
    } else if (m_hasDiffResult && m_lastDiff.changes.empty()) {
        m_lblDiffSummary->setText(QStringLiteral(
            "%1: %2 %3. (%4 ms)"
        ).arg(Language::GerenciadorTraducao::trText("Comparação concluída"))
         .arg(0)
         .arg(Language::GerenciadorTraducao::trText("alteração(ões) identificada(s)"))
         .arg(m_lastElapsedMs));

        m_lblSha256Evidence->setText(QStringLiteral(
            "%1: %2 | %3: %4"
        ).arg(Language::GerenciadorTraducao::trText("Hash de Evidência Base"),
              QString::fromStdString(m_lastBaseHash),
              Language::GerenciadorTraducao::trText("Hash de Evidência Atual"),
              QString::fromStdString(m_lastCurrHash)));
    } else if (m_lblDiffSummary) {
        m_lblDiffSummary->setText(Language::GerenciadorTraducao::trText("Selecione dois snapshots acima para comparar permissões, herança e novos riscos."));
        if (m_lblSha256Evidence) m_lblSha256Evidence->clear();
    }
}

void SnapshotDiffWidget::refreshSnapshotList() {
    QString prevBase = m_comboBaseline ? m_comboBaseline->currentData().toString() : QString();
    QString prevCurr = m_comboCurrent ? m_comboCurrent->currentData().toString() : QString();

    m_comboBaseline->clear();
    m_comboCurrent->clear();

    m_availableSnapshots = Engine::SnapshotEngine::instance().listSnapshots();

    for (const auto& s : m_availableSnapshots) {
        QString text = QStringLiteral("[%1] %2 (%3 %4)").arg(
            QString::fromStdWString(s.timestampIso),
            QString::fromStdWString(s.title),
            QString::number(s.totalFiles + s.totalDirectories),
            Language::GerenciadorTraducao::trText("itens")
        );
        m_comboBaseline->addItem(text, QString::fromStdWString(s.id));
        m_comboCurrent->addItem(text, QString::fromStdWString(s.id));
    }

    if (!prevBase.isEmpty()) {
        int idx = m_comboBaseline->findData(prevBase);
        if (idx >= 0) m_comboBaseline->setCurrentIndex(idx);
    }
    if (!prevCurr.isEmpty()) {
        int idx = m_comboCurrent->findData(prevCurr);
        if (idx >= 0) m_comboCurrent->setCurrentIndex(idx);
    }

    if (prevBase.isEmpty() && prevCurr.isEmpty() && m_comboCurrent->count() > 1) {
        m_comboCurrent->setCurrentIndex(0);
        m_comboBaseline->setCurrentIndex(1);
    }
}

void SnapshotDiffWidget::onCompareClicked() {
    if (m_comboBaseline->currentIndex() < 0 || m_comboCurrent->currentIndex() < 0) {
        QMessageBox::information(this,
            Language::GerenciadorTraducao::trText("Snapshots"),
            Language::GerenciadorTraducao::trText("É necessário ter pelo menos dois snapshots para comparar."));
        return;
    }

    QString idBase = m_comboBaseline->currentData().toString();
    QString idCurr = m_comboCurrent->currentData().toString();

    m_isComparing = true;
    m_btnCompare->setEnabled(false);
    m_comboBaseline->setEnabled(false);
    m_comboCurrent->setEnabled(false);
    m_btnExport->setEnabled(false);
    m_btnImport->setEnabled(false);

    m_progressBar->setRange(0, 0); // Smooth indeterminate busy animation
    m_progressBar->setVisible(true);

    updateSummaryLabels();

    auto timer = std::make_shared<QElapsedTimer>();
    timer->start();

    const std::wstring baseIdW = idBase.toStdWString();
    const std::wstring currIdW = idCurr.toStdWString();

    QThread* worker = QThread::create([this, baseIdW, currIdW, timer]() {
        auto snapBase = Engine::SnapshotEngine::instance().loadSnapshot(baseIdW);
        auto snapCurr = Engine::SnapshotEngine::instance().loadSnapshot(currIdW);

        bool success = snapBase.has_value() && snapCurr.has_value();
        Engine::SnapshotDiffResult diffResult;
        std::string baseHash;
        std::string currHash;

        if (success) {
            baseHash = snapBase->sha256Hash;
            currHash = snapCurr->sha256Hash;
            diffResult = Engine::SnapshotEngine::instance().compareSnapshots(*snapBase, *snapCurr);
        }

        const qint64 elapsedMs = timer->elapsed();

        Engine::SnapshotEngine::instance().cleanupThreadConnection();

        QMetaObject::invokeMethod(this, [this, success, diff = std::move(diffResult), baseHash = std::move(baseHash), currHash = std::move(currHash), elapsedMs]() mutable {
            onCompareFinished(success, std::move(diff), std::move(baseHash), std::move(currHash), elapsedMs);
        }, Qt::QueuedConnection);
    });

    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    worker->start();
}

void SnapshotDiffWidget::onCompareFinished(bool success, Engine::SnapshotDiffResult diff, std::string baseHash, std::string currHash, qint64 elapsedMs) {
    m_isComparing = false;
    m_progressBar->setVisible(false);
    m_btnCompare->setEnabled(true);
    m_comboBaseline->setEnabled(true);
    m_comboCurrent->setEnabled(true);
    m_btnExport->setEnabled(true);
    m_btnImport->setEnabled(true);

    if (!success) {
        m_hasDiffResult = false;
        QMessageBox::warning(this,
            Language::GerenciadorTraducao::trText("Erro"),
            Language::GerenciadorTraducao::trText("Falha ao carregar snapshots do banco local."));
        updateSummaryLabels();
        return;
    }

    m_hasDiffResult = true;
    m_lastBaseHash = std::move(baseHash);
    m_lastCurrHash = std::move(currHash);
    m_lastDiff = std::move(diff);
    m_lastElapsedMs = elapsedMs;

    m_diffModel->setDiff(m_lastDiff);
    updateSummaryLabels();
    applyTheme(ThemeManager::instance().isDark());
}

void SnapshotDiffWidget::onExportEvidenceClicked() {
    if (m_comboCurrent->currentIndex() < 0) return;
    QString id = m_comboCurrent->currentData().toString();
    auto snap = Engine::SnapshotEngine::instance().loadSnapshot(id.toStdWString());
    if (!snap.has_value()) return;

    QString savePath = QFileDialog::getSaveFileName(
        this,
        Language::GerenciadorTraducao::trText("Exportar Evidência de Snapshot"),
        QStringLiteral("snapshot_evidencia_%1.json").arg(QString::fromStdWString(snap->id).left(8)),
        QStringLiteral("Arquivos JSON (*.json)")
    );

    if (!savePath.isEmpty()) {
        if (Engine::SnapshotEngine::instance().exportSnapshotToJson(*snap, savePath.toStdWString())) {
            QMessageBox::information(this,
                Language::GerenciadorTraducao::trText("Sucesso"),
                Language::GerenciadorTraducao::trText("Arquivo de evidência exportado com assinatura de integridade SHA-256."));
        }
    }
}

void SnapshotDiffWidget::onImportSnapshotClicked() {
    QString openPath = QFileDialog::getOpenFileName(
        this,
        Language::GerenciadorTraducao::trText("Importar Arquivo de Evidência de Snapshot"),
        QString(),
        QStringLiteral("Arquivos JSON (*.json)")
    );

    if (!openPath.isEmpty()) {
        auto imported = Engine::SnapshotEngine::instance().importSnapshotFromJson(openPath.toStdWString());
        if (imported.has_value()) {
            Engine::SnapshotEngine::instance().saveSnapshot(*imported);
            refreshSnapshotList();
            QMessageBox::information(this,
                Language::GerenciadorTraducao::trText("Sucesso"),
                Language::GerenciadorTraducao::trText("Snapshot importado com sucesso para a base local."));
        } else {
            QMessageBox::warning(this,
                Language::GerenciadorTraducao::trText("Erro"),
                Language::GerenciadorTraducao::trText("O arquivo selecionado não é um snapshot válido do ACL Auditor."));
        }
    }
}

void SnapshotDiffWidget::applyTheme(bool isDark) {
    if (m_btnCompare) {
        m_btnCompare->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #0078d7; color: #ffffff; border: 1px solid #005a9e; border-radius: 4px; font-weight: bold; padding: 6px 14px; } QPushButton:hover { background-color: #1a88e1; }") :
            QStringLiteral("QPushButton { background-color: #0066cc; color: #ffffff; border: 1px solid #0052a3; border-radius: 4px; font-weight: bold; padding: 6px 14px; } QPushButton:hover { background-color: #0073e6; }"));
    }

    auto styleSecondaryBtn = [isDark](QPushButton* btn) {
        if (!btn) return;
        btn->setStyleSheet(isDark ?
            QStringLiteral("QPushButton { background-color: #2d3748; color: #e2e8f0; border: 1px solid #4a5568; border-radius: 4px; padding: 5px 12px; } QPushButton:hover { background-color: #3e4c63; }") :
            QStringLiteral("QPushButton { background-color: #f1f5f9; color: #0f172a; border: 1px solid #cbd5e1; border-radius: 4px; padding: 5px 12px; } QPushButton:hover { background-color: #e2e8f0; }"));
    };
    styleSecondaryBtn(m_btnExport);
    styleSecondaryBtn(m_btnImport);

    if (m_summaryCard) {
        m_summaryCard->setStyleSheet(isDark ?
            QStringLiteral("QFrame { background-color: #1a2234; border: 1px solid #2d3748; border-radius: 6px; }") :
            QStringLiteral("QFrame { background-color: #f8fafc; border: 1px solid #e2e8f0; border-radius: 6px; }"));
    }

    if (m_progressBar) {
        m_progressBar->setStyleSheet(isDark ?
            QStringLiteral("QProgressBar { border: none; background-color: #2d3748; height: 4px; border-radius: 2px; } QProgressBar::chunk { background-color: #3b82f6; border-radius: 2px; }") :
            QStringLiteral("QProgressBar { border: none; background-color: #e2e8f0; height: 4px; border-radius: 2px; } QProgressBar::chunk { background-color: #2563eb; border-radius: 2px; }"));
    }

    if (m_lblDiffSummary) {
        if (m_lastDiff.changes.empty()) {
            m_lblDiffSummary->setStyleSheet(isDark ?
                QStringLiteral("color: #94a3b8; font-weight: bold;") :
                QStringLiteral("color: #64748b; font-weight: bold;"));
        } else if (m_lastDiff.totalNewRisks > 0) {
            m_lblDiffSummary->setStyleSheet(isDark ?
                QStringLiteral("color: #f87171; font-weight: bold;") :
                QStringLiteral("color: #dc2626; font-weight: bold;"));
        } else {
            m_lblDiffSummary->setStyleSheet(isDark ?
                QStringLiteral("color: #4ade80; font-weight: bold;") :
                QStringLiteral("color: #16a34a; font-weight: bold;"));
        }
    }

    if (m_lblSha256Evidence) {
        m_lblSha256Evidence->setStyleSheet(isDark ?
            QStringLiteral("color: #94a3b8; font-family: monospace; font-size: 11px;") :
            QStringLiteral("color: #64748b; font-family: monospace; font-size: 11px;"));
    }

    const QString promptColor = isDark ? QStringLiteral("color: #cbd5e1;") : QStringLiteral("color: #334155;");
    if (m_lblBaselinePrompt) m_lblBaselinePrompt->setStyleSheet(promptColor);
    if (m_lblCurrentPrompt) m_lblCurrentPrompt->setStyleSheet(promptColor);

    if (m_diffSearchEdit) {
        m_diffSearchEdit->setStyleSheet(isDark ?
            QStringLiteral("QLineEdit { background-color: #18191b; color: #f0f2f5; border: 1px solid #3c4046; border-radius: 4px; padding: 4px 8px; }") :
            QStringLiteral("QLineEdit { background-color: #ffffff; color: #212529; border: 1px solid #ced4da; border-radius: 4px; padding: 4px 8px; }"));
    }
}

} // namespace AclAuditor::UI
