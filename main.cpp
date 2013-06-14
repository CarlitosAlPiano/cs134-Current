#include "Polycode.h"
#include "PolycodeView.h"
#include "Current.h"

int main(int argc, char *argv[]) {
  PolycodeView *view = new PolycodeView("Current");
	Current *app = new Current(view);
	while(app->Update()) {}
	return 0;
}
