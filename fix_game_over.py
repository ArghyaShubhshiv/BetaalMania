import os

game_h = "src/Game.h"
with open(game_h, "r") as f:
    content = f.read()

if "bool isGameOver;" not in content:
    content = content.replace("bool reloading;", "bool reloading;\n    bool isGameOver;")
    with open(game_h, "w") as f:
        f.write(content)

game_cpp = "src/Game.cpp"
with open(game_cpp, "r") as f:
    content = f.read()

content = content.replace("if(player->getHealth()<=0)exit(0);", "if(player->getHealth()<=0)isGameOver=true;")
content = content.replace("if (player->getHealth() <= 0) exit(0);", "if (player->getHealth() <= 0) isGameOver=true;")
content = content.replace("Game::Game() {", "Game::Game() {\n    isGameOver=false;")
content = content.replace("void Game::reset(){", "void Game::reset(){\n    isGameOver=false;")

with open(game_cpp, "w") as f:
    f.write(content)

main_cpp = "src/2D-Top-down-shooter.cpp"
with open(main_cpp, "r") as f:
    content = f.read()

content = content.replace("game->updateMovement(keys);", "if(game->isGameOver){ returnToMenu(); } else { game->updateMovement(keys); }")

with open(main_cpp, "w") as f:
    f.write(content)
