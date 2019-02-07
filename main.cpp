#include <SFML/Graphics.hpp>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>

int draw = 0;
int done = 0;
std::mutex mtx;
std::condition_variable cv;

void drawarray(sf::RenderWindow &w, int wi, int he, std::vector<unsigned int> &v)
{
	std::unique_lock<std::mutex> lk(mtx);
	while(1)	
	{
		cv.wait(lk, []{ return draw == 1 || done == 1; });
		if(done == 1)
			return;
		w.clear(sf::Color::Black);
		sf::RectangleShape rec;
		rec.setFillColor(sf::Color::White);
		float rec_width = (float)wi / (float)v.size();
		for(auto i=0; i<v.size(); i++)
		{
			rec.setPosition(i*rec_width, he - v[i]);
			rec.setSize(sf::Vector2<float>(rec_width, v[i]));
			w.draw(rec);
		}
		w.display();
	}
	return;
}

void BubbleSort(std::vector<unsigned int> &v)
{
	for(auto i=0; i<v.size() - 1; i++)
		for(auto j=0; j<v.size() - 1; j++)
			if(v[j] > v[j + 1])
			{
				std::swap(v[j], v[j + 1]);
				draw = 1;
				cv.notify_one();
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
	done = 1;
	return;
}

int main()
{
	srand((unsigned)time(NULL));
	int w_heigth=100, w_width=500;
	std::vector<unsigned int> v(100);
	for(auto&& x : v)
		x = rand() % w_heigth + 1;
	sf::RenderWindow window(sf::VideoMode(w_width, w_heigth), "Sorting");
	std::thread tsort(BubbleSort, std::ref(v));
	tsort.detach();
	std::thread tdraw(drawarray, std::ref(window), std::ref(w_width), std::ref(w_heigth), std::ref(v));	
	tdraw.join();
	while(window.isOpen())
	{
		sf::Event event;
		while(window.pollEvent(event))
			if(event.type == sf::Event::KeyPressed && 
					event.key.code == sf::Keyboard::Escape)
				window.close();
	}
	return 0;
}