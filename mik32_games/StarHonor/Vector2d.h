#ifndef VECTOR2D_H
#define VECTOR2D_H

class Vector2d
{
  public:
  Vector2d();
  Vector2d(int PosX, int PosY);
  ~Vector2d();

  float Magnitude();
  float MagnitudeSquared();
  void Normalize();

  Vector2d operator*(const Vector2d &rhs);
  Vector2d operator*(const float rhs);
  Vector2d& operator*=(const Vector2d &rhs); // замена типа Vector2d на ссылку, добавляем return *this
  Vector2d operator+(const Vector2d &rhs);
  Vector2d& operator+=(const Vector2d &rhs); // замена типа Vector2d на ссылку, добавляем return *this
  Vector2d operator-(const Vector2d &rhs);
  Vector2d& operator-=(const Vector2d &rhs); // замена типа Vector2d на ссылку, добавляем return *this
  float Dot( Vector2d b );
  void Rotate( float angle ); // замена типа на void
  
  float x;
  float y;
};

static float Deg2Rad( float Deg );

#endif
