#include "src/app.h"

int main() {

    App* app = new App("RTNV");
    if (!app->LoadShaderItem("base",app->baseShader)) {
        return 1;
    }
    if (!app->LoadShaderItem("geom", app->geomShader)) {
        return 1;
    }
    app->LoadMeshes();
    return app->StartedRT();
}
