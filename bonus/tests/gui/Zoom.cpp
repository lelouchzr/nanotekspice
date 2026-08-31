#include "Zoom.hpp"
#include "CircuitGraph.hpp"
#include "graphics/Gui.hpp"
#include "nts/Parser.hpp"
#include <SDL.h>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace {

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

void push(SDL_Event event) {
    require(SDL_PushEvent(&event) == 1, "cannot queue zoom interaction");
}

void key(SDL_Keycode code, Uint16 modifiers = 0) {
    SDL_Event event{};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = code;
    event.key.keysym.mod = modifiers;
    push(event);
}

void click(int x, int y) {
    SDL_Event event{};
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = x;
    event.button.y = y;
    push(event);
    event.type = SDL_MOUSEBUTTONUP;
    push(event);
}

void wheel(float steps, int x, int y, bool flipped = false, bool horizontal = false) {
    SDL_WarpMouseInWindow(nullptr, x, y);
    SDL_Event event{};
    event.type = SDL_MOUSEWHEEL;
    event.wheel.x = horizontal ? 2 : 0;
    event.wheel.y = static_cast<Sint32>(steps);
    event.wheel.direction = flipped ? SDL_MOUSEWHEEL_FLIPPED : SDL_MOUSEWHEEL_NORMAL;
#if SDL_VERSION_ATLEAST(2, 0, 18)
    event.wheel.preciseY = steps;
#endif
#if SDL_VERSION_ATLEAST(2, 26, 0)
    event.wheel.mouseX = x;
    event.wheel.mouseY = y;
#endif
    push(event);
}

bool same(nts::bonus::ViewPoint a, nts::bonus::ViewPoint b) {
    return std::abs(a.x - b.x) < 1e-7 && std::abs(a.y - b.y) < 1e-7;
}

SDL_Window *currentWindow() {
    for (Uint32 id = 1; id <= 32; ++id)
        if (auto *window = SDL_GetWindowFromID(id))
            return window;
    throw std::runtime_error("cannot find native zoom test window");
}

void drag() {
    SDL_Event event{};
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = 100;
    event.button.y = 300;
    push(event);
    event = {};
    event.type = SDL_MOUSEMOTION;
    event.motion.x = 140;
    event.motion.y = 280;
    push(event);
    event = {};
    event.type = SDL_MOUSEBUTTONUP;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = 140;
    event.button.y = 280;
    push(event);
}

} // namespace

void zoomInteractions() {
    using namespace nts;
    using namespace nts::bonus;
    std::istringstream source(".chipsets:\ninput a\noutput out\n.links:\na:1 out:1\n"
                              ".graphic:\na 400 300\nout 760 300\n");
    auto circuit = Parser::parse(source);
    const auto real = circuitCallbacks(circuit);
    Gui *window = nullptr;
    ViewPoint anchor, center;
    unsigned frame = 0;
    auto callbacks = real;
    callbacks.captureGraph = [&] {
        const auto &view = window->view();
        const auto output = circuit.component("out").compute(1);
        switch (frame++) {
        case 0:
            anchor = view.world({632, 388});
            wheel(0, 644, 460, false, true);
            wheel(1, 1100, 200); // Inspector must not scroll the circuit.
            wheel(1, 200, 20);   // Toolbar must not scroll the circuit.
            break;
        case 1:
            require(view.scale() == 1, "zero/horizontal/off-canvas wheel changed zoom");
            wheel(.25F, 644, 460);
            break;
        case 2:
            require(std::abs(view.scale() - std::pow(1.15, .25)) < 1e-9,
                    "fractional trackpad wheel was lost");
            require(same(view.world({632, 388}), anchor), "wheel zoom moved the cursor anchor");
            wheel(-.25F, 644, 460);
            break;
        case 3:
            require(std::abs(view.scale() - 1) < 1e-9, "fractional wheel did not reverse");
            wheel(2, 644, 460);
            break;
        case 4:
            require(std::abs(view.scale() - 1.3225) < 1e-9, "wheel magnitude ignored");
            wheel(2, 644, 460, true);
            break;
        case 5:
            require(std::abs(view.scale() - 1) < 1e-9, "flipped wheel direction ignored");
            center = view.world(view.center());
            key(SDLK_EQUALS, KMOD_CTRL);
            break;
        case 6:
            require(std::abs(view.scale() - 1.15) < 1e-9, "Ctrl+= did not zoom in");
            require(same(center, view.world(view.center())), "keyboard zoom moved canvas center");
            key(SDLK_KP_MINUS);
            break;
        case 7:
            require(std::abs(view.scale() - 1) < 1e-9, "keypad minus did not zoom out");
            key(SDLK_PLUS);
            break;
        case 8:
            require(std::abs(view.scale() - 1.15) < 1e-9, "plus did not zoom in");
            key(SDLK_0, KMOD_GUI);
            break;
        case 9:
            require(view.scale() == 1, "Cmd+0 did not reset view");
            click(450, 400);
            key(SDLK_1);
            key(SDLK_RETURN);
            break;
        case 10:
            require(circuit.tick() == 1 && output == True, "selection failed after zoom reset");
            for (int i = 0; i < 30; ++i)
                key(SDLK_KP_PLUS);
            break;
        case 11:
            require(view.scale() == 4, "keyboard maximum zoom exceeded");
            key(SDLK_0);
            key(SDLK_RETURN);
            break;
        case 12:
            require(view.scale() == 4 && circuit.tick() == 2 && output == False,
                    "plain 0 must edit input instead of resetting zoom");
            key(SDLK_0, KMOD_CTRL);
            break;
        case 13:
            require(view.scale() == 1 && circuit.tick() == 2, "Ctrl+0 altered simulation");
            anchor = view.world({438, 328});
            for (int i = 0; i < 8; ++i)
                wheel(-8, 450, 400);
            break;
        case 14:
            require(view.scale() == .1, "wheel minimum zoom exceeded");
            require(same(anchor, view.world({438, 328})), "clamped zoom moved cursor anchor");
            click(450, 400);
            key(SDLK_1);
            key(SDLK_RETURN);
            break;
        case 15:
            require(circuit.tick() == 3 && output == True, "hit testing failed at minimum zoom");
            key(SDLK_HOME);
            break;
        case 16: {
            const auto first = view.screen({400, 300}), last = view.screen({1000, 398});
            require(first.x >= 15 && first.y >= 15 && last.x <= 941 && last.y <= 721,
                    "Home did not fit every node inside canvas");
            center = view.world(view.center());
            SDL_SetWindowSize(currentWindow(), 800, 600);
            break;
        }
        case 17:
            require(view.center().x == 238 && view.center().y == 258,
                    "native resize did not update canvas dimensions");
            require(same(center, view.world(view.center())), "native resize moved world center");
            key(SDLK_0, KMOD_CTRL);
            break;
        case 18:
            drag();
            break;
        case 19:
            require(same(view.screen({400, 300}), {440, 280}), "native panning used wrong scale");
            require(circuit.tick() == 3 && output == True, "navigation altered the circuit");
            key(SDLK_ESCAPE);
            break;
        default:
            if (frame > 22)
                throw std::runtime_error("zoom GUI did not exit");
            break;
        }
        return real.captureGraph();
    };
    {
        Gui gui(callbacks);
        window = &gui;
        gui.run();
    }
    require(frame >= 20 && circuit.tick() == 3, "zoom scenario did not complete");
}
