#pragma once

#include <QString>

namespace AclAuditor::Utils {

class ExplorerIntegration {
public:
    static bool isContextMenuRegistered();
    static bool registerContextMenu();
    static bool unregisterContextMenu();
};

} // namespace AclAuditor::Utils
