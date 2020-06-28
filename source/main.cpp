#include "board.hpp"

#include "ui/window.hpp"

int main() {
    arm::Board board;
    arm::ui::Window debuggerWindow(board);

    board.powerUp();

    while(debuggerWindow.update())
        board.tick();

    return 0;
}

