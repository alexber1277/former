#include "src/app.h"

int main() {

    App* app = new App("RTNV");
    app->LoadMeshes();
    return app->StartedRT();

}
