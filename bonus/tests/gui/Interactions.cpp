#include "CircuitGraph.hpp"
#include "GraphGui.hpp"
#include "Zoom.hpp"
#include "nts/Parser.hpp"
#include <SDL.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

void key(SDL_Keycode code) {
    SDL_Event event{};
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = code;
    require(SDL_PushEvent(&event) == 1, "cannot queue GUI key");
}

void click(int x, int y) {
    SDL_Event event{};
    event.type = SDL_MOUSEBUTTONDOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = x;
    event.button.y = y;
    require(SDL_PushEvent(&event) == 1, "cannot queue GUI click");
    event.type = SDL_MOUSEBUTTONUP;
    require(SDL_PushEvent(&event) == 1, "cannot queue GUI release");
}
} // namespace

int main() {
    try {
        using namespace nts;
        using namespace nts::bonus;
        std::istringstream source(".chipsets:\ninput a\noutput out\n.links:\na:1 out:1\n"
                                  ".graphic:\na 30 50\nout 400 50\n");
        auto circuit = Parser::parse(source);
        const auto real = circuitCallbacks(circuit);
        unsigned frame = 0;
        GraphCallbacks callbacks = real;
        callbacks.captureGraph = [&] {
            const auto graph = real.captureGraph();
            const auto output = circuit.component("out").compute(1);
            switch (frame++) {
            case 0:
                click(50, 140); // Select input and schedule U -> 0.
                break;
            case 1:
                require(output == Undefined, "GUI assignment bypassed deferred simulation");
                key(SDLK_RETURN);
                break;
            case 2:
                require(graph.tick == 1 && output == False, "GUI Step did not propagate 0");
                key(SDLK_1);
                key(SDLK_F10);
                break;
            case 3:
                require(graph.tick == 2 && output == True, "GUI explicit 1 did not reach output");
                key(SDLK_u);
                click(40, 20); // Toolbar Step.
                break;
            case 4:
                require(graph.tick == 3 && output == Undefined, "GUI U did not reach output");
                key(SDLK_0);
                key(SDLK_SPACE);
                break;
            case 5:
                require(graph.tick == 35 && output == False, "GUI Run must execute 32 real ticks");
                click(170, 20); // Toolbar Pause.
                break;
            case 6:
                require(graph.tick == 35, "GUI Pause did not stop simulation");
                key(SDLK_1);
                click(40, 20);
                break;
            case 7:
                require(graph.tick == 36 && output == True, "editing after Pause failed");
                for (int i = 0; i < 50; ++i) {
                    SDL_Event wheel{};
                    wheel.type = SDL_MOUSEWHEEL;
                    wheel.wheel.y = i < 25 ? 1 : -1;
                    require(SDL_PushEvent(&wheel) == 1, "cannot queue zoom");
                }
                click(300, 20); // Reset View, then select the same input at its initial position.
                click(50, 140);
                key(SDLK_RETURN);
                break;
            case 8:
                require(graph.tick == 37 && output == Undefined,
                        "Reset View broke input hit testing");
                key(SDLK_ESCAPE);
                break;
            default:
                break;
            }
            if (frame > 12)
                throw std::runtime_error("GUI did not consume exit event");
            return graph;
        };
        runGui(callbacks);
        require(frame >= 9 && circuit.tick() == 37, "GUI scenario did not complete");
        require(SDL_WasInit(0) == 0, "standalone GUI retained the SDL runtime");
        zoomInteractions();
        require(SDL_WasInit(0) == 0, "zoom GUI retained the SDL runtime");
        GraphCallbacks shortRun = real;
        shortRun.captureGraph = [&] {
            require(SDL_SetHint("NTS_GUI_MEMORY_MARKER", "allocated") == SDL_TRUE,
                    "cannot set owned SDL state");
            SDL_Event quit{};
            quit.type = SDL_QUIT;
            require(SDL_PushEvent(&quit) == 1, "cannot queue GUI shutdown");
            return real.captureGraph();
        };
        for (int repeat = 0; repeat < 4; ++repeat) {
            runGui(shortRun);
            require(SDL_WasInit(0) == 0, "repeated GUI retained SDL global state");
            require(SDL_GetHint("NTS_GUI_MEMORY_MARKER") == nullptr,
                    "standalone GUI retained its SDL hint allocation");
        }
        require(SDL_InitSubSystem(SDL_INIT_TIMER) == 0, "cannot initialize caller's SDL timer");
        runGui(shortRun);
        require((SDL_WasInit(0) & SDL_INIT_TIMER) != 0, "GUI destroyed caller-owned SDL state");
        require((SDL_WasInit(0) & SDL_INIT_VIDEO) == 0,
                "GUI retained its borrowed video subsystem");
        require(SDL_GetHint("NTS_GUI_MEMORY_MARKER") != nullptr,
                "GUI destroyed caller-owned global SDL allocations");
        SDL_Quit();
        const char *driver = std::getenv("SDL_VIDEODRIVER");
        const bool hadDriver = driver != nullptr;
        const std::string previousDriver = driver ? driver : "";
        require(SDL_setenv("SDL_VIDEODRIVER", "epitech-invalid-driver", 1) == 0,
                "cannot configure failed video initialization");
        bool rejected = false;
        try {
            runGui(shortRun);
        } catch (const std::runtime_error &) {
            rejected = true;
        }
        if (hadDriver)
            SDL_setenv("SDL_VIDEODRIVER", previousDriver.c_str(), 1);
        else
            unsetenv("SDL_VIDEODRIVER");
        require(rejected && SDL_WasInit(0) == 0, "failed GUI initialization retained SDL state");
        std::cout << "Native SDL events: selection, deferred 0/1/U, Step, Run x32, Pause, zoom, "
                     "Reset View, repeated lifecycle and SDL ownership passed against real circuit "
                     "output\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
