/*
* Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_TIME_STEP_H
#define B2_TIME_STEP_H

#include <Box2D/Common/Math.h>

namespace box2d {

/// Profiling data. Times are in milliseconds.
struct Profile
{
	float_t step;
	float_t collide;
	float_t solve;
	float_t solveInit;
	float_t solveVelocity;
	float_t solvePosition;
	float_t broadphase;
	float_t solveTOI;
};

/// This is an internal structure.
class TimeStep
{
public:
	float_t get_dt() const noexcept { return dt; }
	float_t get_inv_dt() const noexcept { return inv_dt; }
	
	void set_dt(float_t value) noexcept
	{
		dt = value;
		inv_dt = (value > float_t(0))? float_t(1) / value: float_t(0);
	}

	float_t dtRatio; ///< dt * inv_dt0
	int32 velocityIterations; ///< Velocity iterations.
	int32 positionIterations; ///< Position iterations.
	bool warmStarting; ///< Whether or not to perform warm starting.

private:
	float_t dt; ///< The time step - delta time.
	float_t inv_dt; ///< Inverse time step (1/dt or 0 if dt == 0). @see dt.
};

/// This is an internal structure.
struct Position
{
	Position() = default;
	constexpr Position(const Position& copy) = default;

	constexpr Position(Vec2 c_, float_t a_) noexcept: c(c_), a(a_) {}

	Vec2 c; ///< Linear position.
	float_t a; ///< Angular position.
};

inline Transform GetTransform(const Position& pos, const Vec2& local_ctr)
{
	return GetTransform(pos.c, Rot(pos.a), local_ctr);
}

/// This is an internal structure.
struct Velocity
{
	Velocity() = default;
	constexpr Velocity(Vec2 v_, float_t w_) noexcept: v(v_), w(w_) {}

	Vec2 v; ///< Linear velocity.
	float_t w; ///< Angular velocity.
};

/// Solver Data
struct SolverData
{
	TimeStep step;
	Position* positions;
	Velocity* velocities;
};

} // namespace box2d

#endif
