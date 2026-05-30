#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#include <queue>
#include <stack>

void floodFill(sf::Image& image, sf::Vector2i startPoint, sf::Color targetColor, sf::Color replacementColor);