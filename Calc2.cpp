
/*
    This program draws the graph of an arbitrary 
    parametric function, then animate the tangent
    and normal line by the aimation of the parameter
*/

#include <iostream>
#include <random>
#include <functional>
#include <string>
#include <tuple>

#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

/*
    Define methods to calculate the tangent 
    and normal line by derivatives
*/   
namespace Calculus
{
    const double EPSILON = 1e-3;
    using Func = function<double(double)>;
    using Func2 = function<double(double, double)>;
    // A line is in the form Line.0 * x + Line.1 * y = Line.2
    using Line = tuple<double, double, double>;

    // Calculate the derivative of f(x)
    // May break at some non-differentiable points and return 0
    Func derivative(const Func& f)
    {
        return [&](double x)
        {
            auto r = (f(x + EPSILON) - f(x)) / EPSILON;
            auto l = (f(x) - f(x - EPSILON)) / EPSILON;
            if (!isnan(r))
                return r;
            else if (!isnan(l))
                return l;
            else
            {
                cout << "Non-existent derivative at " << x;
                return 0.0;
            }
        };
    }

    // Calculate the equation of tangent line at t0
    Line tangent(const Func& x, const Func& y, double t0)
    {
        double dry = derivative(y)(t0), drx = derivative(x)(t0);
        double vy = y(t0), vx = x(t0);
        return Line(dry, -drx, dry * vx - drx * vy);
    }

    // Calculate the equation of normal line at t0
    Line normal(const Func& x, const Func& y, double t0)
    {
        double dry = derivative(y)(t0), drx = derivative(x)(t0);
        double vy = y(t0), vx = x(t0);
        return Line(drx, dry, drx * vx + dry * vy);
    }
}

/*
    Define two methods used to render the 
    graph, its tangent and normal
*/
namespace Graph
{
    // Plot the graph from t = start to t = end with fixed step count
    VertexArray plot(const Calculus::Func& x, const Calculus::Func& y, double start, double end, int step = 100)
    {
        VertexArray res(LinesStrip);
        Vertex ver;
        ver.color = sf::Color::Red;
        for (double t = start; t <= end; t += (end - start) / step)
        {
            ver.position = sf::Vector2f(x(t), y(t));
            res.append(ver);
        }
        return res;
    }

    // Calculate two endpoints of each line, one of which is
    // length / 2 far from (x(t0), y(t0))
    VertexArray atPoint(const Calculus::Func& x, const Calculus::Func& y, double t0, double length)
    {
        VertexArray res(Lines);
        Vertex ver;
        ver.color = Color::Blue;
        Calculus::Line tg = Calculus::tangent(x, y, t0);
        Calculus::Line nm = Calculus::normal(x, y, t0);
        Vector2f center(x(t0), y(t0));
        double a = get<0>(tg), b = get<1>(tg), c = get<2>(tg);
        auto tmp = [&]()
        {
            if (a)
            {
                double dy = length * a / 2 / sqrtl(a * a + b * b);
                for (int i = 0; i <= 1; ++i)
                {
                    dy *= -1;
                    double dx = -b * dy / a;
                    ver.position = center + Vector2f(dx, dy);
                    res.append(ver);
                }
            }
            else if (b)
            {
                double dx = length / 2 * b / sqrtl(a * a + b * b);
                for (int i = 0; i <= 1; ++i)
                {
                    dx *= -1;
                    double dy = -a * dx / b;
                    ver.position = center + Vector2f(dx, dy);
                    res.append(ver);
                }
            }
        };
        tmp();
        ver.color = Color::Green;
        a = get<0>(nm), b = get<1>(nm), c = get<2>(nm);
        tmp();

        return res;
    }
}

// Define variable for ploting
double START = -sqrt(300);
double END = sqrt(300);
int STEP = 1000;
float CYCLE = 10.f;
constexpr int WIN_SIZE = 800;
Calculus::Func F1 = [](double x)
{
    return 100 * 3 * x / (1 + x) / (1 + x) / (1 + x);
};
Calculus::Func F2 = [](double x)
{
    return 100 * 3 * x * x / (1 + x) / (1 + x) / (1 + x);
};

// Main program
int main()
{

    RenderWindow win(VideoMode(WIN_SIZE, WIN_SIZE), "Graph");
    win.setFramerateLimit(60);

    // Use RenderTexture to flip the graph
    RenderTexture texture;
    texture.create(WIN_SIZE, WIN_SIZE);
    VertexArray quad(Quads);
    quad.append(Vertex(sf::Vector2f(0, WIN_SIZE), Color::White, sf::Vector2f(0, 0)));
    quad.append(Vertex(sf::Vector2f(WIN_SIZE, WIN_SIZE), Color::White, sf::Vector2f(WIN_SIZE, 0)));
    quad.append(Vertex(sf::Vector2f(WIN_SIZE, 0), Color::White, sf::Vector2f(WIN_SIZE, WIN_SIZE)));
    quad.append(Vertex(sf::Vector2f(0, 0), Color::White, sf::Vector2f(0, WIN_SIZE)));
    sf::RenderStates states;
    View view(sf::Vector2f(), win.getView().getSize());
    texture.setView(view);

    VertexArray graph = Graph::plot(F1, F2, START, END, STEP);
    VertexArray tangent;

    Clock clock;
    Time FPS = sf::seconds(1.f / 60);
    Time elapsed = clock.restart();
    float dt = START;
    while (win.isOpen())
    {
        elapsed += clock.restart();
        sf::Event e;
        while (win.pollEvent(e))
        {
            if (e.type == Event::Closed)
                win.close();
        }
        while (elapsed > FPS)
        {
            elapsed -= FPS;
            dt += (END - START) / CYCLE * FPS.asSeconds();
            while (dt > END)
                dt -= END - START;
            tangent = Graph::atPoint(F1, F2, dt, 100);
        }

        win.clear();
        texture.clear(Color::White);
        texture.draw(graph);
        texture.draw(tangent);
        texture.display();
        states.texture = &texture.getTexture();
        win.draw(quad, states);
        win.display();
    }
}