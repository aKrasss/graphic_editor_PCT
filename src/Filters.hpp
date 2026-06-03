#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <string>

class Filter {
public:
    virtual ~Filter() = default;
    virtual void apply(sf::Image& image) = 0;
    virtual std::string getName() const = 0;
};

class GrayscaleFilter : public Filter {
public:
    void apply(sf::Image& image) override {
        sf::Vector2u size = image.getSize();
        for (unsigned y = 0; y < size.y; ++y)
            for (unsigned x = 0; x < size.x; ++x) {
                sf::Color c = image.getPixel(x, y);
                int gray = static_cast<int>(0.299f * c.r + 0.587f * c.g + 0.114f * c.b);
                image.setPixel(x, y, sf::Color(gray, gray, gray, c.a));
            }
    }
    std::string getName() const override { return "Grayscale"; }
};

class InvertFilter : public Filter {
public:
    void apply(sf::Image& image) override {
        sf::Vector2u size = image.getSize();
        for (unsigned y = 0; y < size.y; ++y)
            for (unsigned x = 0; x < size.x; ++x) {
                sf::Color c = image.getPixel(x, y);
                image.setPixel(x, y, sf::Color(255 - c.r, 255 - c.g, 255 - c.b, c.a));
            }
    }
    std::string getName() const override { return "Invert"; }
};

class BlurFilter : public Filter {
public:
    void apply(sf::Image& image) override {
        sf::Image blurred = image;
        sf::Vector2u size = image.getSize();
        int kernelSize = 3, offset = kernelSize / 2;
        for (unsigned y = offset; y < size.y - offset; ++y)
            for (unsigned x = offset; x < size.x - offset; ++x) {
                int r = 0, g = 0, b = 0, count = 0;
                for (int ky = -offset; ky <= offset; ++ky)
                    for (int kx = -offset; kx <= offset; ++kx) {
                        sf::Color p = image.getPixel(x + kx, y + ky);
                        r += p.r; g += p.g; b += p.b; count++;
                    }
                blurred.setPixel(x, y, sf::Color(r / count, g / count, b / count));
            }
        image = blurred;
    }
    std::string getName() const override { return "Blur"; }
};

class BrightnessFilter : public Filter {
private:
    int adj;
public:
    BrightnessFilter(int a) : adj(a) {}
    void apply(sf::Image& image) override {
        sf::Vector2u size = image.getSize();
        for (unsigned y = 0; y < size.y; ++y)
            for (unsigned x = 0; x < size.x; ++x) {
                sf::Color c = image.getPixel(x, y);
                image.setPixel(x, y, sf::Color(
                    std::clamp(c.r + adj, 0, 255),
                    std::clamp(c.g + adj, 0, 255),
                    std::clamp(c.b + adj, 0, 255), c.a));
            }
    }
    std::string getName() const override { return adj > 0 ? "Brightness+" : "Brightness-"; }
};

class SepiaFilter : public Filter {
public:
    void apply(sf::Image& image) override {
        sf::Vector2u size = image.getSize();
        for (unsigned y = 0; y < size.y; ++y)
            for (unsigned x = 0; x < size.x; ++x) {
                sf::Color c = image.getPixel(x, y);
                int tr = static_cast<int>(0.393f * c.r + 0.769f * c.g + 0.189f * c.b);
                int tg = static_cast<int>(0.349f * c.r + 0.686f * c.g + 0.168f * c.b);
                int tb = static_cast<int>(0.272f * c.r + 0.534f * c.g + 0.131f * c.b);
                image.setPixel(x, y, sf::Color(std::min(tr,255), std::min(tg,255), std::min(tb,255), c.a));
            }
    }
    std::string getName() const override { return "Sepia"; }
};

class SharpenFilter : public Filter {
public:
    void apply(sf::Image& image) override {
        sf::Image sharpened = image;
        sf::Vector2u size = image.getSize();
        int kernel[3][3] = {{0,-1,0},{-1,5,-1},{0,-1,0}};
        for (unsigned y = 1; y < size.y - 1; ++y)
            for (unsigned x = 1; x < size.x - 1; ++x) {
                int r = 0, g = 0, b = 0;
                for (int ky = -1; ky <= 1; ++ky)
                    for (int kx = -1; kx <= 1; ++kx) {
                        sf::Color p = image.getPixel(x + kx, y + ky);
                        int w = kernel[ky+1][kx+1];
                        r += p.r * w; g += p.g * w; b += p.b * w;
                    }
                sharpened.setPixel(x, y, sf::Color(std::clamp(r,0,255), std::clamp(g,0,255), std::clamp(b,0,255)));
            }
        image = sharpened;
    }
    std::string getName() const override { return "Sharpen"; }
};
