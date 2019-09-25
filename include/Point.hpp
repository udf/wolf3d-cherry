/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Point.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhoosen <mhoosen@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/06/10 13:14:19 by mhoosen           #+#    #+#             */
/*   Updated: 2019/06/10 18:46:45 by mhoosen          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <cmath>

// Generic x,y pair
template<typename T>
class Point {
  public:
    Point() {
    }
    Point(T x, T y) : x(x), y(y) {
    }
    Point(const Point &other) {
        *this = other;
    }
    Point &operator=(const Point &other) {
        this->x = other.x;
        this->y = other.y;
        return *this;
    }
    ~Point() {
    }

    Point(T v) : x(v), y(v) {
    }

    bool operator==(const Point &other) const {
        return this->x == other.x && this->y == other.y;
    }

    Point operator+(const Point &other) const {
        return Point(this->x + other.x, this->y + other.y);
    }
    Point operator+(const T &val) const {
        return Point(this->x + val, this->y + val);
    }

    Point operator-(const Point &other) const {
        return Point(this->x - other.x, this->y - other.y);
    }
    Point operator-(const T &val) const {
        return Point(this->x - val, this->y - val);
    }

    Point &operator+=(const Point &other) {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }
    Point &operator+=(const T &val) {
        this->x += val;
        this->y += val;
        return *this;
    }

    Point &operator-=(const Point &other) {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }
    Point &operator-=(const T &val) {
        this->x -= val;
        this->y -= val;
        return *this;
    }

    Point &operator*=(const T &val) {
        this->x *= val;
        this->y *= val;
        return *this;
    }
    Point &operator/=(const T &val) {
        this->x /= val;
        this->y /= val;
        return *this;
    }

    Point operator*(const T &val) const {
        return Point(this->x * val, this->y * val);
    }
    Point operator/(const T &val) const {
        return Point(this->x / val, this->y / val);
    }

    template<typename U>
    Point<U> cast_to() const {
        return Point<U>(
            static_cast<U>(x),
            static_cast<U>(y)
        );
    }

    T magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    Point normalize() {
        return *this / magnitude();
    }

    T x, y;

    using type = T;
};