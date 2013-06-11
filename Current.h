#include <Polycode.h>
#include "PolycodeView.h"
#include "MainMenu.h"

class Current {
private:
    Core *core;
    MainMenu *menu;

public:
    Current(PolycodeView *view);
    ~Current();
    bool Update();
};
