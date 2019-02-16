#include <SFML/Graphics.hpp>
#include <list>
#include <utility>
#include <math.h>
#include <iostream>
#include "wave_source.h"

void draw(sf::RenderWindow &w, int wi, int he, std::list<wave_source> &fields, double res, double dt)
{
    if(fields.empty()) return;
    sf::RectangleShape c(sf::Vector2f(res,res));
    double col;
    for_each(fields.begin(), fields.end(), [&](wave_source &s){ s.gettime() += dt; });
    for(int i=0; i<wi; i+=res)
    {
        for(int j=0;j<he;j+=res)
        {
            col = 0.f;
            for_each(fields.begin(), fields.end(), [&](wave_source &s){
                     double r = sqrt((s.getxpos() - i) * (s.getxpos() - i) + (s.getypos() - j) * (s.getypos() - j));
                     col += sin(s.getomega() * s.gettime() - r * s.getk())/(double)fields.size();
                     });
            col *= 255.f;
            if(col > 0)
                c.setFillColor(sf::Color(255.f, col, 0));
            else
                c.setFillColor(sf::Color(255.f + col, 0, -col));
            c.setPosition(i,j);
            w.draw(c);
        }
    }
}

int main()
{
    int w_width = 600, w_heigth = 600;
    double res = 3.f;
    sf::Clock clock;
    clock.restart();
    std::list<wave_source> fields;
    sf::RenderWindow window(sf::VideoMode(w_width, w_heigth), "SFML works!");

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R)
                fields.clear();
            else if(event.type == sf::Event::MouseButtonPressed)
                if(sf::Mouse::getPosition(window).x > 0 && sf::Mouse::getPosition(window).x < w_width && sf::Mouse::getPosition(window).y > 0 && sf::Mouse::getPosition(window).y < w_heigth)
                fields.push_front(wave_source(floor((double)sf::Mouse::getPosition(window).x/res)*res, floor((double)sf::Mouse::getPosition(window).y/res)*res, 0.1f, 10.f));
        }
        window.clear();
        sf::sleep(sf::seconds(0.02) - clock.getElapsedTime());
        draw(window, w_width, w_heigth, fields, res, clock.restart().asSeconds());
        window.display();
    }

    return 0;
}
