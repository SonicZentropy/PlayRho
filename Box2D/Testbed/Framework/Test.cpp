/*
* Original work Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
* Modified work Copyright (c) 2016 Louis Langholtz https://github.com/louis-langholtz/Box2D
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

#include "Test.h"
#include "Drawer.h"
#include <stdio.h>

#include <Box2D/Rope/Rope.h>
#include <Box2D/Dynamics/FixtureProxy.hpp>

using namespace box2d;

static void Draw(Drawer& drawer, const CircleShape& shape, const Transformation& xf, const Color& color)
{
	const auto center = Transform(shape.GetLocation(), xf);
	const auto radius = shape.GetRadius();
	const auto axis = Rotate(Vec2{float_t{1}, float_t{0}}, xf.q);
	drawer.DrawSolidCircle(center, radius, axis, color);
}

static void Draw(Drawer& drawer, const EdgeShape& shape, const Transformation& xf, const Color& color)
{
	const auto v1 = Transform(shape.GetVertex1(), xf);
	const auto v2 = Transform(shape.GetVertex2(), xf);
	drawer.DrawSegment(v1, v2, color);
}

static void Draw(Drawer& drawer, const ChainShape& shape, const Transformation& xf, const Color& color)
{
	const auto count = shape.GetVertexCount();
	auto v1 = Transform(shape.GetVertex(0), xf);
	for (auto i = decltype(count){1}; i < count; ++i)
	{
		const auto v2 = Transform(shape.GetVertex(i), xf);
		drawer.DrawSegment(v1, v2, color);
		drawer.DrawCircle(v1, float_t(0.05), color);
		v1 = v2;
	}
}

static void Draw(Drawer& drawer, const PolygonShape& shape, const Transformation& xf, const Color& color)
{
	const auto vertexCount = shape.GetVertexCount();
	assert(vertexCount <= MaxPolygonVertices);
	Vec2 vertices[MaxPolygonVertices];
	for (auto i = decltype(vertexCount){0}; i < vertexCount; ++i)
	{
		vertices[i] = Transform(shape.GetVertex(i), xf);
	}
	drawer.DrawSolidPolygon(vertices, vertexCount, color);
}

static void Draw(Drawer& drawer, const Fixture& fixture, const Transformation& xf, const Color& color)
{
	switch (GetType(fixture))
	{
		case Shape::e_circle:
			Draw(drawer, *static_cast<const CircleShape*>(fixture.GetShape()), xf, color);
			break;
			
		case Shape::e_edge:
			Draw(drawer, *static_cast<const EdgeShape*>(fixture.GetShape()), xf, color);
			break;
			
		case Shape::e_chain:
			Draw(drawer, *static_cast<const ChainShape*>(fixture.GetShape()), xf, color);
			break;
			
		case Shape::e_polygon:
			Draw(drawer, *static_cast<const PolygonShape*>(fixture.GetShape()), xf, color);
			break;
			
		default:
			break;
	}
}

static Color GetColor(const Body& body)
{
	if (!body.IsActive())
	{
		return Color(0.5f, 0.5f, 0.3f);
	}
	if (body.GetType() == BodyType::Static)
	{
		return Color(0.5f, 0.9f, 0.5f);
	}
	if (body.GetType() == BodyType::Kinematic)
	{
		return Color(0.5f, 0.5f, 0.9f);
	}
	if (!body.IsAwake())
	{
		return Color(0.6f, 0.6f, 0.6f);
	}
	return Color(0.9f, 0.7f, 0.7f);
}

static void Draw(Drawer& drawer, const Body& body)
{
	const auto xf = body.GetTransformation();
	const auto color = GetColor(body);
	for (auto&& f: body.GetFixtures())
	{
		Draw(drawer, f, xf, color);
	}
}

static void Draw(Drawer& drawer, const Joint& joint)
{
	const auto bodyA = joint.GetBodyA();
	const auto bodyB = joint.GetBodyB();
	const auto xf1 = bodyA->GetTransformation();
	const auto xf2 = bodyB->GetTransformation();
	const auto x1 = xf1.p;
	const auto x2 = xf2.p;
	const auto p1 = joint.GetAnchorA();
	const auto p2 = joint.GetAnchorB();
	
	const Color color(float_t(0.5), float_t(0.8), float_t(0.8));
	
	switch (joint.GetType())
	{
		case JointType::Distance:
			drawer.DrawSegment(p1, p2, color);
			break;
			
		case JointType::Pulley:
		{
			const auto pulley = static_cast<const PulleyJoint&>(joint);
			const auto s1 = pulley.GetGroundAnchorA();
			const auto s2 = pulley.GetGroundAnchorB();
			drawer.DrawSegment(s1, p1, color);
			drawer.DrawSegment(s2, p2, color);
			drawer.DrawSegment(s1, s2, color);
		}
			break;
			
		case JointType::Mouse:
			// don't draw this
			break;
			
		default:
			drawer.DrawSegment(x1, p1, color);
			drawer.DrawSegment(p1, p2, color);
			drawer.DrawSegment(x2, p2, color);
	}
}

static void Draw(Drawer& drawer, const World& world, const Settings& settings)
{
	if (settings.drawShapes)
	{
		for (auto&& b: world.GetBodies())
		{
			Draw(drawer, b);
		}
	}
	
	if (settings.drawJoints)
	{
		for (auto&& j: world.GetJoints())
		{
			Draw(drawer, j);
		}
	}
	
	if (settings.drawAABBs)
	{
		const Color color(0.9f, 0.3f, 0.9f);
		const auto bp = &world.GetContactManager().m_broadPhase;
		
		for (auto&& b: world.GetBodies())
		{
			if (!b.IsActive())
			{
				continue;
			}
			
			for (auto&& f: b.GetFixtures())
			{
				const auto proxy_count = f.GetProxyCount();
				for (auto i = decltype(proxy_count){0}; i < proxy_count; ++i)
				{
					const auto proxy = f.GetProxy(i);
					const auto aabb = bp->GetFatAABB(proxy->proxyId);
					Vec2 vs[4];
					vs[0] = Vec2{aabb.GetLowerBound().x, aabb.GetLowerBound().y};
					vs[1] = Vec2{aabb.GetUpperBound().x, aabb.GetLowerBound().y};
					vs[2] = Vec2{aabb.GetUpperBound().x, aabb.GetUpperBound().y};
					vs[3] = Vec2{aabb.GetLowerBound().x, aabb.GetUpperBound().y};
					
					drawer.DrawPolygon(vs, 4, color);
				}
			}
		}
	}
	
	if (settings.drawCOMs)
	{
		for (auto&& b: world.GetBodies())
		{
			auto xf = b.GetTransformation();
			xf.p = b.GetWorldCenter();
			drawer.DrawTransform(xf);
		}
	}
}

void TestDestructionListener::SayGoodbye(Joint& joint)
{
	if (test->m_mouseJoint == &joint)
	{
		test->m_mouseJoint = nullptr;
	}
	else
	{
		test->JointDestroyed(&joint);
	}
}

Test::Test()
{
	m_world = new World(World::Def{}.UseGravity(Vec2(0.0f, -10.0f)));
	m_bomb = nullptr;
	m_textLine = 30;
	m_mouseJoint = nullptr;
	m_pointCount = 0;

	m_destructionListener.test = this;
	m_world->SetDestructionListener(&m_destructionListener);
	m_world->SetContactListener(this);
	
	m_bombSpawning = false;

	m_stepCount = 0;

	BodyDef bodyDef;
	m_groundBody = m_world->CreateBody(bodyDef);

	memset(&m_maxProfile, 0, sizeof(Profile));
	memset(&m_totalProfile, 0, sizeof(Profile));
}

Test::~Test()
{
	// By deleting the world, we delete the bomb, mouse joint, etc.
	delete m_world;
	m_world = nullptr;
}

void Test::PreSolve(Contact& contact, const Manifold& oldManifold)
{
	const auto& manifold = contact.GetManifold();

	const auto manifoldPointCount = manifold.GetPointCount();
	if (manifoldPointCount == 0)
	{
		return;
	}

	auto fixtureA = contact.GetFixtureA();
	auto fixtureB = contact.GetFixtureB();

	PointStateArray state1;
	PointStateArray state2;
	GetPointStates(state1, state2, oldManifold, manifold);

	const auto worldManifold = GetWorldManifold(contact);

	for (auto i = decltype(manifoldPointCount){0}; (i < manifoldPointCount) && (m_pointCount < k_maxContactPoints); ++i)
	{
		auto cp = m_points + m_pointCount;
		cp->fixtureA = fixtureA;
		cp->fixtureB = fixtureB;
		cp->position = worldManifold.GetPoint(i);
		cp->normal = worldManifold.GetNormal();
		cp->state = state2[i];
		cp->normalImpulse = manifold.GetPoint(i).normalImpulse;
		cp->tangentImpulse = manifold.GetPoint(i).tangentImpulse;
		cp->separation = worldManifold.GetSeparation(i);
		++m_pointCount;
	}
}

void Test::DrawTitle(Drawer& drawer, const char *string)
{
    drawer.DrawString(5, DRAW_STRING_NEW_LINE, string);
    m_textLine = 3 * DRAW_STRING_NEW_LINE;
}

class QueryCallback : public QueryFixtureReporter
{
public:
	QueryCallback(const Vec2& point)
	{
		m_point = point;
		m_fixture = nullptr;
	}

	bool ReportFixture(Fixture* fixture)
	{
		Body* body = fixture->GetBody();
		if (body->GetType() == BodyType::Dynamic)
		{
			bool inside = TestPoint(*fixture, m_point);
			if (inside)
			{
				m_fixture = fixture;

				// We are done, terminate the query.
				return false;
			}
		}

		// Continue the query.
		return true;
	}

	Vec2 m_point;
	Fixture* m_fixture;
};

void Test::MouseDown(const Vec2& p)
{
	m_mouseWorld = p;
	
	if (m_mouseJoint != nullptr)
	{
		return;
	}

	// Make a small box.
	const auto aabb = AABB{p, p} + Vec2(0.001f, 0.001f);
	
	// Query the world for overlapping shapes.
	QueryCallback callback(p);
	m_world->QueryAABB(&callback, aabb);

	if (callback.m_fixture)
	{
		Body* body = callback.m_fixture->GetBody();
		MouseJointDef md;
		md.bodyA = m_groundBody;
		md.bodyB = body;
		md.target = p;
		md.maxForce = 1000.0f * GetMass(*body);
		m_mouseJoint = (MouseJoint*)m_world->CreateJoint(md);
		body->SetAwake();
	}
}

void Test::SpawnBomb(const Vec2& worldPt)
{
	m_bombSpawnPoint = worldPt;
	m_bombSpawning = true;
}
    
void Test::CompleteBombSpawn(const Vec2& p)
{
	if (m_bombSpawning == false)
	{
		return;
	}

	const float multiplier = 30.0f;
	Vec2 vel = m_bombSpawnPoint - p;
	vel *= multiplier;
	LaunchBomb(m_bombSpawnPoint,vel);
	m_bombSpawning = false;
}

void Test::ShiftMouseDown(const Vec2& p)
{
	m_mouseWorld = p;
	
	if (m_mouseJoint != nullptr)
	{
		return;
	}

	SpawnBomb(p);
}

void Test::MouseUp(const Vec2& p)
{
	if (m_mouseJoint)
	{
		m_world->Destroy(m_mouseJoint);
		m_mouseJoint = nullptr;
	}
	
	if (m_bombSpawning)
	{
		CompleteBombSpawn(p);
	}
}

void Test::MouseMove(const Vec2& p)
{
	m_mouseWorld = p;
	
	if (m_mouseJoint)
	{
		m_mouseJoint->SetTarget(p);
	}
}

void Test::LaunchBomb()
{
	Vec2 p(RandomFloat(-15.0f, 15.0f), 30.0f);
	Vec2 v = -5.0f * p;
	LaunchBomb(p, v);
}

void Test::LaunchBomb(const Vec2& position, const Vec2& linearVelocity)
{
	if (m_bomb)
	{
		m_world->Destroy(m_bomb);
		m_bomb = nullptr;
	}

	BodyDef bd;
	bd.type = BodyType::Dynamic;
	bd.position = position;
	bd.bullet = true;
	m_bomb = m_world->CreateBody(bd);
	m_bomb->SetVelocity(Velocity{linearVelocity, 0_rad});
	
	CircleShape circle(0.3f);

	FixtureDef fd;
	fd.shape = &circle;
	fd.density = 20.0f;
	fd.restitution = 0.0f;
	
	m_bomb->CreateFixture(fd);
}

void Test::Step(Settings& settings, Drawer& drawer)
{
	float_t timeStep = settings.hz > 0.0f ? 1.0f / settings.hz : float_t(0.0f);

	if (settings.pause)
	{
		if (settings.singleStep)
		{
			settings.singleStep = false;
		}
		else
		{
			timeStep = 0.0f;
		}

		drawer.DrawString(5, m_textLine, "****PAUSED****");
		m_textLine += DRAW_STRING_NEW_LINE;
	}

	m_world->SetAllowSleeping(settings.enableSleep);
	m_world->SetWarmStarting(settings.enableWarmStarting);
	m_world->SetContinuousPhysics(settings.enableContinuous);
	m_world->SetSubStepping(settings.enableSubStepping);

	m_pointCount = 0;

	m_world->Step(timeStep, static_cast<unsigned>(settings.velocityIterations), static_cast<unsigned>(settings.positionIterations));

	Draw(drawer, *m_world, settings);

	drawer.Flush();

	if (timeStep > 0.0f)
	{
		++m_stepCount;
	}

	if (settings.drawStats)
	{
		const auto sleepCount = [&](){
			auto count = unsigned(0);
			for (auto&& body: m_world->GetBodies())
			{
				if (!body.IsAwake())
				{
					++count;
				}
			}
			return count;
		}();
		const auto bodyCount = GetBodyCount(*m_world);
		const auto contactCount = GetContactCount(*m_world);
		const auto jointCount = GetJointCount(*m_world);
		drawer.DrawString(5, m_textLine, "sleep/bodies/contacts/joints = %d/%d/%d/%d", sleepCount, bodyCount, contactCount, jointCount);
		m_textLine += DRAW_STRING_NEW_LINE;

		const auto proxyCount = m_world->GetProxyCount();
		const auto height = m_world->GetTreeHeight();
		const auto balance = m_world->GetTreeBalance();
		float_t quality = m_world->GetTreeQuality();
		drawer.DrawString(5, m_textLine, "proxies/height/balance/quality = %d/%d/%d/%g", proxyCount, height, balance, quality);
		m_textLine += DRAW_STRING_NEW_LINE;
	}

	// Track maximum profile times
	{
		const Profile& p = m_world->GetProfile();
		m_maxProfile.step = Max(m_maxProfile.step, p.step);
		m_maxProfile.collide = Max(m_maxProfile.collide, p.collide);
		m_maxProfile.solve = Max(m_maxProfile.solve, p.solve);
		m_maxProfile.solveInit = Max(m_maxProfile.solveInit, p.solveInit);
		m_maxProfile.solveVelocity = Max(m_maxProfile.solveVelocity, p.solveVelocity);
		m_maxProfile.solvePosition = Max(m_maxProfile.solvePosition, p.solvePosition);
		m_maxProfile.solveTOI = Max(m_maxProfile.solveTOI, p.solveTOI);
		m_maxProfile.broadphase = Max(m_maxProfile.broadphase, p.broadphase);

		m_totalProfile.step += p.step;
		m_totalProfile.collide += p.collide;
		m_totalProfile.solve += p.solve;
		m_totalProfile.solveInit += p.solveInit;
		m_totalProfile.solveVelocity += p.solveVelocity;
		m_totalProfile.solvePosition += p.solvePosition;
		m_totalProfile.solveTOI += p.solveTOI;
		m_totalProfile.broadphase += p.broadphase;
	}

	if (settings.drawProfile)
	{
		const Profile& p = m_world->GetProfile();

		Profile aveProfile;
		memset(&aveProfile, 0, sizeof(Profile));
		if (m_stepCount > 0)
		{
			float_t scale = 1.0f / m_stepCount;
			aveProfile.step = scale * m_totalProfile.step;
			aveProfile.collide = scale * m_totalProfile.collide;
			aveProfile.solve = scale * m_totalProfile.solve;
			aveProfile.solveInit = scale * m_totalProfile.solveInit;
			aveProfile.solveVelocity = scale * m_totalProfile.solveVelocity;
			aveProfile.solvePosition = scale * m_totalProfile.solvePosition;
			aveProfile.solveTOI = scale * m_totalProfile.solveTOI;
			aveProfile.broadphase = scale * m_totalProfile.broadphase;
		}

		drawer.DrawString(5, m_textLine, "step [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.step, aveProfile.step, m_maxProfile.step);
		m_textLine += DRAW_STRING_NEW_LINE;
		drawer.DrawString(5, m_textLine, "collide [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.collide, aveProfile.collide, m_maxProfile.collide);
		m_textLine += DRAW_STRING_NEW_LINE;
		drawer.DrawString(5, m_textLine, "solve [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solve, aveProfile.solve, m_maxProfile.solve);
		m_textLine += DRAW_STRING_NEW_LINE;
		drawer.DrawString(5, m_textLine, "solve init [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveInit, aveProfile.solveInit, m_maxProfile.solveInit);
		m_textLine += DRAW_STRING_NEW_LINE;
		drawer.DrawString(5, m_textLine, "solve velocity [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveVelocity, aveProfile.solveVelocity, m_maxProfile.solveVelocity);
		m_textLine += DRAW_STRING_NEW_LINE;
		drawer.DrawString(5, m_textLine, "solve position [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solvePosition, aveProfile.solvePosition, m_maxProfile.solvePosition);
		m_textLine += DRAW_STRING_NEW_LINE;
		drawer.DrawString(5, m_textLine, "solveTOI [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.solveTOI, aveProfile.solveTOI, m_maxProfile.solveTOI);
		m_textLine += DRAW_STRING_NEW_LINE;
		drawer.DrawString(5, m_textLine, "broad-phase [ave] (max) = %5.2f [%6.2f] (%6.2f)", p.broadphase, aveProfile.broadphase, m_maxProfile.broadphase);
		m_textLine += DRAW_STRING_NEW_LINE;
	}

	if (m_mouseJoint)
	{
		Vec2 p1 = m_mouseJoint->GetAnchorB();
		Vec2 p2 = m_mouseJoint->GetTarget();

		Color c;
		c.Set(0.0f, 1.0f, 0.0f);
		drawer.DrawPoint(p1, 4.0f, c);
		drawer.DrawPoint(p2, 4.0f, c);

		c.Set(0.8f, 0.8f, 0.8f);
		drawer.DrawSegment(p1, p2, c);
	}
	
	if (m_bombSpawning)
	{
		Color c;
		c.Set(0.0f, 0.0f, 1.0f);
		drawer.DrawPoint(m_bombSpawnPoint, 4.0f, c);

		c.Set(0.8f, 0.8f, 0.8f);
		drawer.DrawSegment(m_mouseWorld, m_bombSpawnPoint, c);
	}

	if (settings.drawContactPoints)
	{
		const float_t k_impulseScale = 0.1f;
		const float_t k_axisScale = 0.3f;

		for (int32 i = 0; i < m_pointCount; ++i)
		{
			ContactPoint* point = m_points + i;

			if (point->state == PointState::AddState)
			{
				// Add
				drawer.DrawPoint(point->position, 10.0f, Color(0.3f, 0.95f, 0.3f));
			}
			else if (point->state == PointState::PersistState)
			{
				// Persist
				drawer.DrawPoint(point->position, 5.0f, Color(0.3f, 0.3f, 0.95f));
			}

			if (settings.drawContactNormals == 1)
			{
				Vec2 p1 = point->position;
				Vec2 p2 = p1 + k_axisScale * point->normal;
				drawer.DrawSegment(p1, p2, Color(0.9f, 0.9f, 0.9f));
			}
			else if (settings.drawContactImpulse == 1)
			{
				Vec2 p1 = point->position;
				Vec2 p2 = p1 + k_impulseScale * point->normalImpulse * point->normal;
				drawer.DrawSegment(p1, p2, Color(0.9f, 0.9f, 0.3f));
			}

			if (settings.drawFrictionImpulse == 1)
			{
				Vec2 tangent = GetFwdPerpendicular(point->normal);
				Vec2 p1 = point->position;
				Vec2 p2 = p1 + k_impulseScale * point->tangentImpulse * tangent;
				drawer.DrawSegment(p1, p2, Color(0.9f, 0.9f, 0.3f));
			}
		}
	}
}

void Test::ShiftOrigin(const Vec2& newOrigin)
{
	m_world->ShiftOrigin(newOrigin);
}

