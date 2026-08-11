#include "App.hpp"

// Thin entry point. Everything that used to live here now belongs to a named
// module: topology/state in Lattice, flat drawing in FlatRenderer, torus
// drawing in TorusRenderer, and the wiring between them in App.
int main(int argc, const char* argv[]) {
    (void)argc; (void)argv;

    App app;
    if (!app.initialize()) {
        return -1;
    }

    app.run();
    app.shutdown();
    return 0;
}
