#include "leg.h"


Leg::Leg(Leg::Side side,
        int pin_base,
        int pin_femur,
        int pin_tibia,
        const Vector4 & base_point,
        double delta_base, 
        double delta_femur, 
        double delta_tibia):
    FSide(side),
    FBasePoint(base_point),
    FDeltaBase(delta_base), FDeltaFemur(delta_femur), FDeltaTibia(delta_tibia)
{
    FServoBase.attach(pin_base);
    FServoFemur.attach(pin_femur);
    FServoTibia.attach(pin_tibia);
}

static bool intersectCircleWithLine(
    double & x, double & y,      // Искомые координаты
    double r,                   // Радиус окружности (с центром в начале координат)
    double a, double b, double c) // Коэффициенты уравнения прямой a*x + b*y + c = 0
{
    double x0 = -a*c / (a*a + b*b);
    double y0 = -b*c / (a*a + b*b);
    const double EPS = 0.001;

    double param = c*c - r*r*(a*a+b*b);

    if (param > EPS)                // 0 points
        return false;
    else if (fabs(param) < EPS)    // 1 point
    {
        x = x0;
        y = y0;
    }
    else                            // 2 points
    {
        double d = r*r - c*c/(a*a+b*b);
        double mult = sqrt (d / (a*a+b*b));

        double ax = x0 + b * mult;
        double bx = x0 - b * mult;
        double ay = y0 - a * mult;
        double by = y0 + a * mult;
        // Выбираем верхнюю точку, чтобы коленки были вверху
        x = (ay > by) ? ax : bx;
        y = (ay > by) ? ay : by;
    }
    return true;
}

/// Пересечение двух окружностей (одна из двух точек, которая выше)
/**
 Возвращает true, если имеется хотя бы одно пересечение, иначе false

 Если центры окружностей совпадают, возвращает false
*/
static bool intersectCircles(
    double & x, double & y,     // Искомые координаты
    double r1,                 // Радиус первой окружности (с центром в начале координат)
    double x2, double y2,       // Координаты центра второй окружности
    double r2)                 // Радиус второй окружности
{
    if (x2 == 0 && y2 == 0) return false;
 
    // Сведем нашу задачу к пересечению окружности и прямой
    // Коэффициенты уравнения прямой a*x + b*y + c = 0 
    double a = -2*x2;
    double b = -2*y2;
    double c = x2*x2 + y2*y2 + r1*r1 - r2*r2;
    
    return intersectCircleWithLine(x, y, r1, a, b, c);
}

void Leg::moveTo(const Vector4 & point)
{
    double x = point.x() - FBasePoint.x();
    double y = point.y() - FBasePoint.y();
    double z = point.z() - FBasePoint.z();

    if (right()) x *= -1;
    
    // Поворот плоскости ноги до целевой точки
    double base_angle = atan2(y, x) * 180.0 / PI;

    if (right()) base_angle *= -1;
        
    // Координаты коленки в плоскости бедра и голени (относительно бедренного сустава)
    double knee_x;
    double knee_y;
    // Координаты кончика ноги в той же системе
    double x2 = sqrt(x*x + y*y) - FEMUR_LEDGE;
    double y2 = -(point.z() - FEMUR_H);
    if (intersectCircles(knee_x, knee_y, FEMUR_LENGTH, x2, y2, TIBIA_LENGTH))
    {
        double femur_angle = atan2(knee_y, knee_x) * 180.0 / PI;
        double tibia_angle = femur_angle + atan2(-(y2 - knee_y), x2 - knee_x) * 180 / PI;

        if (right())
        {
            femur_angle *= -1;
            tibia_angle = 180 - tibia_angle;
        }
        
        setServoAngles(90 + base_angle, 90 + femur_angle, tibia_angle);
    }
}

Vector4 Leg::basePoint() const
{
    return FBasePoint;
}

Vector4 Leg::tipPoint() const
{
    return FTipPoint;
}

void Leg::setTipPoint(const Vector4 & value)
{
    FTipPoint = value;
}

void Leg::setServoAngles(double base, double femur, double tibia)
{
    FServoBase.write(base + FDeltaBase);
    FServoFemur.write(femur + FDeltaFemur);
    FServoTibia.write(tibia + FDeltaTibia);
}

bool Leg::right() const
{
    return (FSide == Right);
}






