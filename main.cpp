#include "SFML/Graphics.hpp"
#include <iostream>

using namespace std;

class PID {
public:
    double kp, ki, kd;
    double prevError;
    double integral;

    PID(double p, double i, double d) : kp(p), ki(i), kd(d), prevError(0), integral(0) {}

    double update(double error, double dt) {
        integral += error * dt;
        double derivative = (error - prevError) / dt;
        double output = kp * error + ki * integral + kd * derivative;
        prevError = error;
        return output;
    }
};

class RotatingArm {
public:
    double angle;
    double angularVelocity;
    double momentOfInertia;

    RotatingArm(double inertia) : angle(0), angularVelocity(0), momentOfInertia(inertia) {}

    void update(double torque, double dt) {
        double angAcc = torque / momentOfInertia;
        angularVelocity += angAcc * dt;
        angle += angularVelocity * dt;
    }
};

string craftStats(double p, double i, double d, double angle, double angVel, double torque, double error, bool paused) {
    return "W/S->P\nE/D->I\nR/F->D\nENTER->RESTART\nP->PAUSE\nI->INV. TARGET\nP: " + to_string(p) + "\nI: " + to_string(i) + "\nD: " + to_string(d) + "\nAng: " + to_string(angle) +
        "\nAngVel: " + to_string(angVel) + "\nTorque: " + to_string(torque) + "\nError: " + to_string(error) + (paused ? "\nPaused" : "");
}

int main()
{
    bool paused = false;
    const int wW = 800, wH = 800;
    int radius = 250;
    int width = 10;
    double initAngle = 0;
    double p = 1.0, dp = 0.05;
    double i = 0.0, di = 0.01;
    double d = 0.1, dd = 0.05;
    double inertia = 1.0;
    double desiredAngle = -3.14159 / 2.0;
    double dt = 0.1;

    sf::RenderWindow window(sf::VideoMode(wW, wH), "PID Sim");
    window.setFramerateLimit(60);

    sf::RectangleShape rect(sf::Vector2f(radius, width));
    rect.setPosition(sf::Vector2f(wW / 2, 500));
    rect.setFillColor(sf::Color::Red);

    PID pid(p, i, d);
    RotatingArm arm(inertia);
    arm.angle = initAngle;

    sf::Font font;
    sf::Text txt;
    if (!font.loadFromFile("Minecraft.ttf")) {
        cout << "Error loading font!" << endl;
    }
    txt.setFont(font);
    txt.setCharacterSize(18);
    txt.setOutlineThickness(2);
    txt.setPosition(sf::Vector2f(20, 20));

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::W) {
                    p += dp;
                    pid.kp = p;
                    //cout << "P: " << p << endl;
                }
                else if (event.key.code == sf::Keyboard::S) {
                    p -= dp;
                    if (p < 0) p = 0;
                    pid.kp = p;
                    //cout << "P: " << p << endl;
                }
                else if (event.key.code == sf::Keyboard::E) {
                    i += di;
                    pid.ki = i;
                    //cout << "I: " << i << endl;
                }
                else if (event.key.code == sf::Keyboard::D) {
                    i -= di;
                    if (i < 0) i = 0;
                    pid.ki = i;
                    //cout << "I: " << i << endl;
                }
                else if (event.key.code == sf::Keyboard::R) {
                    d += dd;
                    pid.kd = d;
                    //cout << "D: " << d << endl;
                }
                else if (event.key.code == sf::Keyboard::F) {
                    d -= dd;
                    if (d < 0) d = 0;
                    pid.kd = d;
                    //cout << "D: " << d << endl;
                }
                else if (event.key.code == sf::Keyboard::Enter) {
                    pid.prevError = 0;
                    arm.angle = initAngle;
                    arm.angularVelocity = 0;
                }
                else if (event.key.code == sf::Keyboard::P) {
                    paused = !paused;
                }
                else if (event.key.code == sf::Keyboard::I) {
                    desiredAngle *= -1.0;
                }
            }
        }
        double error = desiredAngle - arm.angle;
        double torque = pid.update(error, dt);
        
        if (!paused) {
            arm.update(torque, dt);
            double rads = arm.angle * 180.0 / 3.14159;
            rect.setRotation(rads);
        }

        window.clear();
        window.draw(rect);
        txt.setString(craftStats(pid.kp, pid.ki, pid.kd, arm.angle, arm.angularVelocity, torque, error, paused));
        window.draw(txt);
        window.display();
    }

    return 0;
}