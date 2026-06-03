#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Filter {
public:
    virtual ~Filter() = default;
    virtual void apply(sf::Image& image) = 0;
    virtual std::string getName() const = 0;
};
