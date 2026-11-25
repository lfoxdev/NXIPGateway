#include "NXPanelStateItem.h"
#include "NXPanel.h"

void IPanelStateItem::notifyGlobalChange() {
    panel.handlePanelStateChange(this);
}