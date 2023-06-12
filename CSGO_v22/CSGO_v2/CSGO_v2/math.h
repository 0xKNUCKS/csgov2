#pragma once
#include <cmath>
#include <algorithm>

#define ABS(x) ((x < 0) ? (-x) : (x))

#define PI 3.14159265358979f

constexpr auto _normalizeDeg = [](float a) noexcept { return isfinite(a) ? remainder(a, 360.0f) : 0.0f; };

namespace math
{
	struct Vector
	{
		float x, y, z;

		constexpr auto notNull() const noexcept
		{
			return x || y || z;
		}

		constexpr auto operator==(const Vector& v) const noexcept
		{
			return x == v.x && y == v.y && z == v.z;
		}

		constexpr auto operator>=(const Vector& v) const noexcept
		{
			return x >= v.x && y >= v.y && z >= v.z;
		}

		constexpr auto operator<=(const Vector& v) const noexcept
		{
			return x <= v.x && y <= v.y && z <= v.z;
		}

		constexpr auto operator>(const Vector& v) const noexcept
		{
			return x > v.x && y > v.y && z > v.z;
		}

		constexpr auto operator<(const Vector& v) const noexcept
		{
			return x < v.x&& y < v.y&& z < v.z;
		}

		constexpr auto operator!=(const Vector& v) const noexcept
		{
			return !(*this == v);
		}

		constexpr Vector& operator=(const float array[3]) noexcept
		{
			x = array[0];
			y = array[1];
			z = array[2];
			return *this;
		}

		constexpr Vector& operator+=(const Vector& v) noexcept
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		constexpr Vector& operator+=(float f) noexcept
		{
			x += f;
			y += f;
			z += f;
			return *this;
		}

		constexpr Vector& operator-=(const Vector& v) noexcept
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}

		constexpr Vector& operator-=(float f) noexcept
		{
			x -= f;
			y -= f;
			z -= f;
			return *this;
		}

		constexpr auto operator-(const Vector& v) const noexcept
		{
			return Vector{ x - v.x, y - v.y, z - v.z };
		}

		constexpr auto operator+(const Vector& v) const noexcept
		{
			return Vector{ x + v.x, y + v.y, z + v.z };
		}

		constexpr auto operator*(const Vector& v) const noexcept
		{
			return Vector{ x * v.x, y * v.y, z * v.z };
		}

		constexpr Vector& operator/=(float div) noexcept
		{
			x /= div;
			y /= div;
			z /= div;
			return *this;
		}

		constexpr Vector& operator*=(float mul) noexcept
		{
			x *= mul;
			y *= mul;
			z *= mul;
			return *this;
		}

		constexpr auto operator*(float mul) const noexcept
		{
			return Vector{ x * mul, y * mul, z * mul };
		}

		constexpr auto operator/(float div) const noexcept
		{
			return Vector{ x / div, y / div, z / div };
		}

		constexpr auto operator-(float sub) const noexcept
		{
			return Vector{ x - sub, y - sub, z - sub };
		}

		constexpr auto operator+(float add) const noexcept
		{
			return Vector{ x + add, y + add, z + add };
		}

		constexpr auto operator-() const noexcept
		{
			return Vector{ -x, -y, -z };
		}

		constexpr Vector& normalizeDeg() noexcept
		{
			x = _normalizeDeg(x);
			y = _normalizeDeg(y);
			z = 0.0f;
			return *this;
		}

		constexpr Vector& normalize() noexcept
		{
			x = std::clamp(x, -89.0f, 89.0f);
			y = std::clamp(y, -180.0f, 180.0f);
			z = 0.0f;
			return *this;
		}

		auto length() const noexcept
		{
			return sqrt(x * x + y * y + z * z);
		}

		auto length2D() const noexcept
		{
			return sqrt(x * x + y * y);
		}

		constexpr auto lengthSquared() const noexcept
		{
			return x * x + y * y + z * z;
		}

		constexpr auto lengthSquared2D() const noexcept
		{
			return x * x + y * y;
		}

		constexpr auto dotProduct(const Vector& v) const noexcept
		{
			return x * v.x + y * v.y + z * v.z;
		}

		constexpr auto dotProduct2D(const Vector& v) const noexcept
		{
			return x * v.x + y * v.y;
		}

		constexpr auto crossProduct(const Vector& v) const noexcept
		{
			return Vector{ y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
		}

		auto distTo(const Vector& v) const noexcept
		{
			return (*this - v).length();
		}

		constexpr auto distToSquared(const Vector& v) const noexcept
		{
			return (*this - v).lengthSquared();
		}

		auto floor() const noexcept
		{
			return Vector{ (float)(int)x, (float)(int)y, (float)(int)z };
		}

		auto snapTo4() const noexcept
		{
			const float l = length2D();
			bool xp = x >= 0.0f;
			bool yp = y >= 0.0f;
			bool xy = fabsf(x) >= fabsf(y);
			if (xp && xy)
				return Vector{ l, 0.0f, 0.0f };
			if (!xp && xy)
				return Vector{ -l, 0.0f, 0.0f };
			if (yp && !xy)
				return Vector{ 0.0f, l, 0.0f };
			if (!yp && !xy)
				return Vector{ 0.0f, -l, 0.0f };

			return Vector{};
		}

		constexpr static auto up() noexcept
		{
			return Vector{ 0.0f, 0.0f, 1.0f };
		}

		constexpr static auto down() noexcept
		{
			return Vector{ 0.0f, 0.0f, -1.0f };
		}

		constexpr static auto forward() noexcept
		{
			return Vector{ 1.0f, 0.0f, 0.0f };
		}

		constexpr static auto back() noexcept
		{
			return Vector{ -1.0f, 0.0f, 0.0f };
		}

		constexpr static auto left() noexcept
		{
			return Vector{ 0.0f, 1.0f, 0.0f };
		}

		constexpr static auto right() noexcept
		{
			return Vector{ 0.0f, -1.0f, 0.0f };
		}

		static auto fromAngle(const Vector& angle) noexcept
		{
			return Vector{ std::cos((angle.x) * PI / 180) * std::cos((angle.y) * PI / 180),
						   std::cos((angle.x) * PI / 180) * std::sin((angle.y) * PI / 180),
						  -std::sin((angle.x) * PI / 180) };
		}
	};

	struct vec4
	{
		float x, y, z, w;
	};

	struct Matrix4x4
	{
		union {
			struct {
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;

			};
			float m[4][4];
		};
	};

	struct Matrix3x4
	{
	public:
		float m[3][4];

		auto GetVecOrgin()
		{
			math::Vector v;
			v.x = m[0][3];
			v.y = m[1][3];
			v.z = m[2][3];
			return v;
		}
	};

	template <typename T>
	class UtlVector
	{
	public:
		constexpr T& operator[](int i) noexcept { return memory[i]; };
		constexpr const T& operator[](int i) const noexcept { return memory[i]; };

		void destructAll() noexcept { while (size) reinterpret_cast<T*>(&memory[--size])->~T(); }

		T* begin() noexcept { return memory; }
		T* end() noexcept { return memory + size; }

		T* memory;
		int allocationCount;
		int growSize;
		int size;
		T* elements;
	};


	struct ESPprop
	{
		int w, h;
		float x, y;
	};
}
