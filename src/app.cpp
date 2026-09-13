#include "app.hpp"

void App::run() {
    while (!window.should_close()) {
        window.poll_events();
    }
}
