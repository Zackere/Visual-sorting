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

typedef struct elem
{
    unsigned int value;
    sf::Color color;
}elem;

sf::Color getRainbowColor(double x)
{
    if(x < 0) return sf::Color::Red;
    if(x > 1) return sf::Color::Yellow;
    x *= 5.f;
    double frac = x;
    while(frac > 1.f)
        frac--;
    if(x > 4) return sf::Color(255.f * frac, 255.f, 0);
    if(x > 3) return sf::Color(0, 255.f, 255.f * (1.f - frac));
    if(x > 2) return sf::Color(0, 255.f * frac, 255.f);
    if(x > 1) return sf::Color(255.f * (1.f - frac), 0, 255.f);
    return sf::Color(255.f, 0, 255.f * frac);
}

void drawarray(sf::RenderWindow &w, int wi, int he, std::vector<elem> &v)
{
	w.setActive(true);
	std::unique_lock<std::mutex> lk(mtx);
	while(1)
	{
		cv.wait(lk, []{ return draw == 1 || done == 1; });
		if(done == 1)
			return;
		w.clear(sf::Color::Black);
		sf::RectangleShape rec;
		float rec_width = (float)wi / (float)v.size();
		for(auto i=0; i<v.size(); i++)
		{
		    rec.setFillColor(v[i].color);
			rec.setPosition(i*rec_width, he - v[i].value);
			rec.setSize(sf::Vector2<float>(rec_width, v[i].value));
			w.draw(rec);
		}
		w.display();
		draw = 0;
		cv.notify_one();
	}
	return;
}

void BubbleSort(std::vector<elem> &v)
{
	std::unique_lock<std::mutex> lk(mtx);
	for(auto i=0; i<v.size() - 1; i++)
		for(auto j=0; j<v.size() - 1; j++)
			if(v[j].value > v[j + 1].value)
			{
				std::swap(v[j], v[j + 1]);
				draw = 1;
				cv.notify_one();
				cv.wait(lk, []{ return draw == 0; });
			}
	done = 1;
	cv.notify_one();
	return;
}

void InsertionSort(std::vector<elem> &v)
{
	std::unique_lock<std::mutex> lk(mtx);
	for(auto i=1; i < v.size(); i++)
		for(auto j = i; j > 0 && v[j - 1].value > v[j].value; j--)
		{
			std::swap(v[j], v[j-1]);
			draw = 1;
			cv.notify_one();
			cv.wait(lk, []{ return draw == 0; });
		}
	done = 1;
	cv.notify_one();
	return;
}

void Merge(std::vector<elem> &v, const unsigned int &l, const unsigned int &m, const unsigned int &r)
{
	auto pom = new elem[r - l];
	for(auto i = 0; i < r - l; i++)
		pom[i] = std::move(v[l + i]);
	unsigned int x = 0, y = m - l, i = l;
	while(x < m - l && y < r - l)
	{
		if(pom[x].value == pom[y].value)
		{
			v[i++] = std::move(pom[x++]);
			v[i++] = std::move(pom[y++]);
		}
		else
			if(pom[y].value < pom[x].value)
				v[i++] = std::move(pom[y++]);
			else
				v[i++] = std::move(pom[x++]);
	}
	if(x == m - l)
		while(y < r - l)
			v[i++] = std::move(pom[y++]);
	else if(y == r - l)
		while(x < m - l)
			v[i++] = std::move(pom[x++]);
	delete[] pom;
}

void MergeSort(std::vector<elem> &v)
{
	std::unique_lock<std::mutex> lk(mtx);
	int curr_size;
	int l,m,r;
	for(curr_size = 1; curr_size < v.size(); curr_size *= 2)
		for(l = 0; l < v.size(); l += 2 * curr_size)
		{
			m = l + curr_size;
			if(m > v.size())
				m = v.size();
			r = l + 2 * curr_size;
			if(r > v.size())
				r = v.size();
			Merge(v, l, m, r);
			draw = 1;
			cv.notify_one();
			cv.wait(lk, []{ return draw == 0; });
		}
	done = 1;
	cv.notify_one();
	return;
}


int main()
{
	srand((unsigned)time(NULL));
	int w_heigth=700, w_width=1000;
	std::vector<elem> v(500);
	v.shrink_to_fit();
	for(auto&& x : v)
        	x.value = rand() % w_heigth + 1;
	auto maxv = (*std::max_element(v.begin(), v.end(), [](elem &x, elem &y){ return x.value < y.value; })).value;
    	for(auto&& x : v)
        	x.color = getRainbowColor((double)x.value / (double)maxv);
	sf::RenderWindow window(sf::VideoMode(w_width, w_heigth), "Sorting");
	window.setActive(false);
	std::thread tsort(InsertionSort, std::ref(v));
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
