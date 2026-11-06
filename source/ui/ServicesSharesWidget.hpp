#pragma once

#include "../engine/AclTypes.hpp"
#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

class QLineEdit;

namespace AclAuditor::UI {

class ServicesSharesWidget : public QWidget {
    Q_OBJECT

public:
    explicit ServicesSharesWidget(QWidget* parent = nullptr);
    ~ServicesSharesWidget() override = default;

public slots:
    void scanServices();
    void scanShares();
    void applyTheme(bool isDark);
    void retranslateUi();

protected:
    void changeEvent(QEvent* event) override;

private:
    void setupUi();

    void renderServicesTable();
    void renderSharesTable();

    QTabWidget* m_tabWidget{nullptr};

    // Services UI
    QPushButton* m_btnScanServices{nullptr};
    QLabel* m_lblServicesSummary{nullptr};
    QLineEdit* m_servicesSearchEdit{nullptr};
    QTableWidget* m_servicesTable{nullptr};

    // Shares UI
    QPushButton* m_btnScanShares{nullptr};
    QLabel* m_lblSharesSummary{nullptr};
    QLineEdit* m_sharesSearchEdit{nullptr};
    QTableWidget* m_sharesTable{nullptr};

    std::vector<Engine::ServiceSecurityFinding> m_lastServices;
    std::vector<Engine::SmbShareFinding> m_lastShares;
};

} // namespace AclAuditor::UI
