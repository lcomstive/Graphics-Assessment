#include <algorithm>
#include <Engine/Utilities.hpp>
#include <Engine/Physics/Shapes.hpp>

using namespace std;
using namespace glm;
using namespace Engine::Physics;

vec3 Engine::Physics::project(vec3& length, vec3& direction)
{
	float _dot = dot(length, direction);
	float magSq = MagnitudeSqr(direction);
	return direction * (_dot / magSq);
}

#pragma region AABB
vec3 AABB::Min() { return Position - Extents / 2.0f; }
vec3 AABB::Max() { return Position + Extents / 2.0f; }

AABB AABB::FromMinMax(vec3 min, vec3 max)
{
	return AABB
	{
		(max + min) / 2.0f, // Position
		(max - min) / 2.0f	// Half Size
	};
}

bool AABB::Intersects(AABB& other)
{
	vec3 min = Min(), max = Max();
	vec3 otherMin = other.Min(), otherMax = other.Max();

	if (max.x < otherMin.x || min.x > otherMax.x) return false;
	if (max.y < otherMin.y || min.y > otherMax.y) return false;
	if (max.z < otherMin.z || min.z > otherMax.z) return false;

	return true;
}

bool AABB::IsPointInside(vec3& point)
{
	vec3 min = Min(), max = Max();
	return
		point.x >= min.x && point.x <= max.x &&
		point.y >= min.y && point.y <= max.y &&
		point.z >= min.z && point.z <= max.z;
}

Interval AABB::GetInterval(const glm::vec3& axis)
{
	vec3 i = Min();
	vec3 a = Max();
	vec3 vertex[8] =
	{
		{ i.x, a.y, a.z },
		{ i.x, a.y, i.z },
		{ i.x, i.y, a.z },
		{ i.x, i.y, i.z },
		{ a.x, a.y, a.z },
		{ a.x, a.y, i.z },
		{ a.x, i.y, a.z },
		{ a.x, i.y, i.z }
	};

	Interval result;
	result.Min = result.Max = dot(axis, vertex[0]);
	for (int i = 1; i < 8; ++i) {
		float projection = dot(axis, vertex[i]);
		result.Min = (projection < result.Min) ?
			projection : result.Min;
		result.Max = (projection > result.Max) ?
			projection : result.Max;
	}

	return result;
}

vec3 AABB::GetClosestPoint(vec3& point)
{
	vec3 result = point;
	vec3 min = Min(), max = Max();
	result.x = (result.x < min.x) ? min.x : result.x;
	result.y = (result.y < min.y) ? min.y : result.y;
	result.z = (result.z < min.z) ? min.z : result.z;

	result.x = (result.x > max.x) ? max.x : result.x;
	result.y = (result.y > max.y) ? max.y : result.y;
	result.z = (result.z > max.z) ? max.z : result.z;

	return result;
}

bool AABB::Raycast(Ray& ray, RaycastHit* outResult)
{
	if (outResult)
		*outResult = RaycastHit{}; // Reset to initial values
	vec3 min = Min(), max = Max();

	// Check each axis
	float t[6] = { 0, 0, 0, 0, 0, 0 };
	for (int i = 0; i < 3; i++)
	{
		if (ray.Direction[i] == 0.0f)
			continue; // Avoid dividing by 0
		t[i * 2 + 0] = (min[i] - ray.Origin[i]) / ray.Direction[i];
		t[i * 2 + 1] = (max[i] - ray.Origin[i]) / ray.Direction[i];
	}

	// Find the largest minimum value
	float tmin = fmaxf(
		fmaxf(
			fminf(t[0], t[1]),
			fminf(t[2], t[3])
		),
		fminf(t[4], t[5])
	);

	// Find the smallest maximum value
	float tmax = fminf(
		fminf(
			fmaxf(t[0], t[1]),
			fmaxf(t[2], t[3])
		),
		fmaxf(t[4], t[5])
	);

	if (tmax < 0 ||  // Bounding volume behind ray origin, no intersection occurred
		tmin > tmax) // No intersection occurred
		return false;

	if (outResult)
	{
		outResult->Hit = true;
		outResult->Distance = tmin < 0 ? tmax : tmin;
		outResult->Point = ray.Origin + ray.Direction * outResult->Distance;

		// Calculate normal of hit
		const vec3 normals[] =
		{
			{ -1,  0,  0 }, { 1, 0, 0 },
			{  0, -1,  0 }, { 0, 1, 0 },
			{  0,  0, -1 }, { 0, 0, 0 },
		};
		for (int i = 0; i < 6; i++)
			if (BasicallyZero(outResult->Distance - t[i]))
				outResult->Normal = normals[i];
	}
	return true;
}

bool AABB::LineTest(Line& line)
{
	Ray ray =
	{
		line.Start,						 // Origin
		normalize(line.End - line.Start) // Direction
	};
	RaycastHit hit;
	if (!Raycast(ray, &hit))
		return false;

	return hit.Distance >= 0 && hit.Distance <= line.Length();
}
#pragma endregion

#pragma region OBB
OBB::OBB(vec3 position, vec3 extents, mat3 orientation) :
	Extents(extents),
	m_VertexExtents(),
	m_VertexPosition(),
	Position(position),
	Orientation(orientation)
{
	m_Planes.resize(6);
	m_Lines.resize(12);
	m_Vertices.resize(8);
}

bool OBB::IsPointInside(vec3& point) const
{
	vec3 dir = point - Position;

	// Iterate & test over each axis
	for (int i = 0; i < 3; i++)
	{
		vec3 axis = Orientation[i];
		float distance = dot(dir, axis);

		if (distance > Extents[i])
			return false;
		if (distance < -Extents[i])
			return false;
	}

	return true;
}

vec3 OBB::GetClosestPoint(vec3& point) const
{
	vec3 result = Position;
	vec3 dir = point - Position;

	// Iterate & test over each axis
	for (int i = 0; i < 3; i++)
	{
		vec3 axis = Orientation[i];
		float distance = std::clamp(dot(dir, axis), -Extents[i], Extents[i]);

		result += axis * distance;
	}

	return result;
}

Interval OBB::GetInterval(const vec3& axis)
{
	vector<vec3>& vertices = GetVertices();

	Interval result;
	result.Min = result.Max = dot(axis, vertices[0]);

	for (int i = 1; i < 8; i++)
	{
		float projection = dot(axis, vertices[i]);
		result.Min = projection < result.Min ? projection : result.Min;
		result.Max = projection > result.Max ? projection : result.Max;
	}
	return result;
}

bool OBB::OverlapOnAxis(OBB& other, vec3& axis)
{
	Interval a = GetInterval(axis);
	Interval b = other.GetInterval(axis);
	return (b.Min <= a.Max) && (a.Min <= b.Max);
}

bool OBB::Raycast(Ray& ray, RaycastHit* outResult)
{
	if (outResult)
		*outResult = RaycastHit{};

	vec3 x = Orientation[0];
	vec3 y = Orientation[1];
	vec3 z = Orientation[2];

	vec3 direction = Position - ray.Origin;
	vec3 f =
	{
		dot(x, ray.Direction),
		dot(y, ray.Direction),
		dot(z, ray.Direction)
	};

	vec3 e =
	{
		dot(x, direction),
		dot(y, direction),
		dot(z, direction)
	};

	// Compare f against ray and store min,max results in t
	float t[6] = { 0, 0, 0, 0, 0, 0 };
	for (int i = 0; i < 3; i++)
	{
		if (BasicallyZero(f[i]))
		{
			if (-e[i] - Extents[i] > 0 ||
				-e[i] + Extents[i] < 0)
				return false; // No collision occurs, exit
			f[i] = 0.00001f; // Avoid division by 0
		}
		t[i * 2 + 0] = (e[i] + Extents[i]) / f[i]; // Min
		t[i * 2 + 1] = (e[i] - Extents[i]) / f[i]; // Max
	}

	float tmin = fmaxf(
		fmaxf(
			fminf(t[0], t[1]),
			fminf(t[2], t[3])),
		fminf(t[4], t[5])
	);
	float tmax = fminf(
		fminf(
			fmaxf(t[0], t[1]),
			fmaxf(t[2], t[3])),
		fmaxf(t[4], t[5])
	);

	if (tmax < 0 ||	 // Bounding volume is behind ray
		tmin > tmax) // No intersection occurred
		return false;

	if (outResult)
	{
		outResult->Hit = true;
		outResult->Distance = tmin < 0.0f ? tmax : tmin;
		outResult->Point = ray.Origin + ray.Direction * outResult->Distance;

		// Find normal of face
		vec3 normals[] =
		{
			x, -x,
			y, -y,
			z, -z
		};
		for (int i = 0; i < 6; i++)
			if (BasicallyZero(outResult->Distance - t[i]))
				outResult->Normal = normals[i];
	}

	return true;
}

bool OBB::LineTest(Line& line)
{
	Ray ray =
	{
		line.Start,						 // Origin
		normalize(line.End - line.Start) // Direction
	};
	RaycastHit hit;
	if (!Raycast(ray, &hit))
		return false;

	return hit.Distance >= 0 && (hit.Distance * hit.Distance) <= line.LengthSqr();
}

vector<Line>& OBB::GetEdges()
{
	vector<vec3>& verts = GetVertices();
	const static int indices[][2] =
	{
		{ 6, 1 }, { 6, 3 }, { 6, 4 }, { 2, 7 }, { 2, 5 }, { 2, 0 },
		{ 0, 1 }, { 0, 3 }, { 7, 1 }, { 7, 4 }, { 4, 5 }, { 5, 3 },
	};
	for (int j = 0; j < 12; j++)
	{
		m_Lines[j].Start = verts[indices[j][0]];
		m_Lines[j].End = verts[indices[j][1]];
	}

	return m_Lines;
}

vector<Plane>& OBB::GetPlanes()
{
	vec3 c = Position;
	vec3 e = Extents;
	
	// Axis
	vec3 a[] =
	{
		Orientation[0],
		Orientation[1],
		Orientation[2]
	};

	m_Planes[0] = Plane( a[0],  dot(a[0], (c + a[0] * e.x)));
	m_Planes[1] = Plane(-a[0], -dot(a[0], (c - a[0] * e.x)));
	m_Planes[2] = Plane( a[1],  dot(a[1], (c + a[1] * e.y)));
	m_Planes[3] = Plane(-a[1], -dot(a[1], (c - a[1] * e.y)));
	m_Planes[4] = Plane( a[2],  dot(a[2], (c + a[2] * e.z)));
	m_Planes[5] = Plane(-a[2], -dot(a[2], (c - a[2] * e.z)));

	return m_Planes;
}

vector<vec3>& OBB::GetVertices()
{
	if (Position == m_VertexPosition && Extents == m_VertexExtents)
		return m_Vertices;

	m_VertexPosition = Position;
	m_VertexExtents = Extents;

	// Axis
	vec3 a[] =
	{
		Orientation[0],
		Orientation[1],
		Orientation[2]
	};

	vec3 c = Position;
	vec3 e = Extents;
	m_Vertices[0] = c + a[0] * e[0] + a[1] * e[1] + a[2] * e[2];
	m_Vertices[1] = c - a[0] * e[0] + a[1] * e[1] + a[2] * e[2];
	m_Vertices[2] = c + a[0] * e[0] - a[1] * e[1] + a[2] * e[2];
	m_Vertices[3] = c + a[0] * e[0] + a[1] * e[1] - a[2] * e[2];
	m_Vertices[4] = c - a[0] * e[0] - a[1] * e[1] - a[2] * e[2];
	m_Vertices[5] = c + a[0] * e[0] - a[1] * e[1] - a[2] * e[2];
	m_Vertices[6] = c - a[0] * e[0] + a[1] * e[1] - a[2] * e[2];
	m_Vertices[7] = c - a[0] * e[0] - a[1] * e[1] + a[2] * e[2];

	return m_Vertices;
}

vector<vec3> OBB::ClipEdges(vector<Line>& edges)
{
	vector<vec3> results;
	results.reserve(edges.size() * 3);

	vec3 intersection;
	vector<Plane>& planes = GetPlanes();

	for (uint32_t i = 0; i < (uint32_t)planes.size(); i++)
	{
		for (uint32_t j = 0; j < (uint32_t)edges.size(); j++)
		{
			if (ClipToPlane(planes[i], edges[j], &intersection))
				if(IsPointInside(intersection))
					results.emplace_back(intersection);
		}
	}

	return results;
}

bool OBB::ClipToPlane(Plane& plane, Line& line, vec3* result)
{
	vec3 ab = line.End - line.Start;
	float nAB = dot(plane.Normal, ab);
	if (BasicallyZero(nAB))
		return false; // No intersection occurs

	float nA = dot(plane.Normal, line.Start);
	float t = (plane.Distance - nA) / nAB;
	if (t >= 0.0f && t <= 1.0f)
	{
		if (result)
			*result = line.Start + ab * t;
		return true;
	}
	return false;
}

float OBB::PenetrationDepth(OBB& other, vec3& axis, bool* outShouldFlip)
{
	axis = normalize(axis);
	Interval i1 = GetInterval(axis);
	Interval i2 = other.GetInterval(axis);

	if (!((i2.Min <= i1.Max) && (i1.Min <= i2.Max)))
		return 0; // No penetration

	float len1 = i1.Max - i1.Min;
	float len2 = i2.Max - i2.Min;

	float min = fminf(i1.Min, i2.Min);
	float max = fmaxf(i1.Max, i2.Max);

	float length = max - min;

	if (outShouldFlip)
		*outShouldFlip = i2.Min < i1.Min;

	return (len1 + len2) - length;
}
#pragma endregion

#pragma region Sphere
bool Sphere::IsPointInside(vec3& point) const { return MagnitudeSqr(point - Position) < (Radius * Radius); }

glm::vec3 Sphere::GetClosestPoint(glm::vec3& point)
{
	vec3 direction = normalize(point - Position);
	return Position + direction * Radius;
}

bool Sphere::Raycast(Ray& ray, RaycastHit* outResult)
{
	if (outResult)
		*outResult = RaycastHit{}; // Set to initial values

	vec3 direction = Position - ray.Origin;
	float magnitudeSqr = MagnitudeSqr(direction);
	float rSqr = Radius * Radius;
	float a = dot(direction, normalize(ray.Direction));
	float bSqr = magnitudeSqr - (a * a);
	float f = sqrt(rSqr - bSqr);
	float t = a - f;

	if (a < 0 || rSqr - (magnitudeSqr - a * a) < 0.0f)
		return false; // No collision
	else if (magnitudeSqr < rSqr)
		t = a + f; // Ray starts inside sphere, reverse

	if (outResult)
	{
		outResult->Hit = true;
		outResult->Distance = t;
		outResult->Point = ray.Origin + ray.Direction * t;
		outResult->Normal = normalize(outResult->Point - Position);
	}
	return true;
}

bool Sphere::LineTest(Line& line)
{
	vec3 closestPoint = line.GetClosestPoint(Position);
	return distance(Position, closestPoint) <= Radius;
}
#pragma endregion

#pragma region Line
Line::Line(vec3 start, vec3 end) : Start(start), End(end) { }

float Line::Length() { return Magnitude(Start - End); }
float Line::LengthSqr() { return MagnitudeSqr(Start - End); }
bool Line::IsPointOn(vec3& point) { return BasicallyZero(MagnitudeSqr(GetClosestPoint(point))); }

vec3 Line::GetClosestPoint(vec3& point)
{
	vec3 line = End - Start;
	float t = dot(point - Start, line) / dot(line, line);
	t = std::clamp(t, 0.0f, 1.0f);
	return Start + line * t;
}
#pragma endregion

#pragma region Plane
Plane::Plane(vec3 normal, float distance) : Normal(normal), Distance(distance) { }
Plane::Plane(Triangle& tri)
{
	Normal = cross(tri.B - tri.A, tri.C - tri.A);
	Distance = dot(Normal, tri.A);
}

PlaneIntersection Plane::CheckPoint(vec3& point) const
{
	float direction = dot(point, Normal) - Distance;
	if (BasicallyZero(direction)) return PlaneIntersection::Intersecting;
	if (direction < 0) return PlaneIntersection::Behind;
	if (direction > 0) return PlaneIntersection::InFront;
	return PlaneIntersection::None;
}

vec3 Plane::GetClosestPoint(vec3& point)
{
	float dot = glm::dot(point, Normal);
	return point - Normal * (dot - Distance);
}

bool Plane::Raycast(Ray& ray, RaycastHit* outResult)
{
	if (outResult)
		*outResult = RaycastHit{}; // Set to initial values

	float nd = dot(ray.Direction, Normal);
	float pn = dot(ray.Origin, Normal);

	if (nd >= 0.0f)
		return false; // No intersection occurs
	float t = (Distance - pn) / nd;

	if (outResult && t >= 0.0f)
	{
		outResult->Distance = t;
		outResult->Hit = true;
		outResult->Normal = normalize(Normal);
		outResult->Point = ray.Origin + ray.Direction * t;
	}

	return t >= 0.0f;
}

bool Plane::LineTest(Line& line)
{
	vec3 direction = line.End - line.Start;

	float nA = dot(Normal, line.Start);
	float nDirection = dot(Normal, direction);

	if (BasicallyZero(nDirection))
		return false; // Line runs parrallel to plane, exit

	float t = (Distance - nA) / nDirection;
	return t >= 0.0f && t <= 1.0f;
}

float Plane::PlaneEquation(glm::vec3 point) { return dot(point, Normal) - Distance; }
#pragma endregion

#pragma region Ray
bool Ray::IsPointOn(vec3& point)
{
	if (point == Origin)
		return true;
	vec3 normal = normalize(point - Origin);
	return BasicallyZero(dot(normal, Direction));
}

vec3 Ray::GetClosestPoint(vec3& point)
{
	float t = dot(point - Origin, normalize(Direction));
	t = std::max(t, 0.0f);
	return Origin + Direction * t;
}

Ray Ray::FromLine(Line& line) { return Ray{ line.Start, normalize(line.End - line.Start) }; }
Ray Ray::FromPoints(vec3 start, vec3 end) { return Ray{ start, normalize(end - start) }; }
#pragma endregion

#pragma region Triangle
Triangle::Triangle() : A(), B(), C() { }
Triangle::Triangle(vec3 a, vec3 b, vec3 c) : A(a), B(b), C(c) { }

bool Triangle::IsPointInside(vec3& point)
{
	vec3 a = A - point;
	vec3 b = B - point;
	vec3 c = C - point;

	// Convert triangle points into local space of the test point
	vec3 normPBC = cross(b, c);
	vec3 normPCA = cross(c, a);
	vec3 normPAB = cross(a, b);

	if (dot(normPBC, normPCA) < 0.0f ||
		dot(normPBC, normPAB) < 0.0f)
		return false; // Points do not have the same normal, point is outside
	return true;
}

vec3 Triangle::GetClosestPoint(vec3& point)
{
	Plane plane(*this);
	vec3 closest = plane.GetClosestPoint(point);
	if (IsPointInside(closest))
		return closest;

	vec3 c1 = Line(A, B).GetClosestPoint(point);
	vec3 c2 = Line(B, C).GetClosestPoint(point);
	vec3 c3 = Line(C, A).GetClosestPoint(point);

	float magSq1 = MagnitudeSqr(point - c1);
	float magSq2 = MagnitudeSqr(point - c2);
	float magSq3 = MagnitudeSqr(point - c3);

	if (magSq1 < magSq2 && magSq1 < magSq3)
		return c1;
	if (magSq2 < magSq1 && magSq2 < magSq3)
		return c2;
	return c3;
}

bool Triangle::Intersects(Sphere& sphere)
{
	vec3 closest = GetClosestPoint(sphere.Position);
	float magSq = MagnitudeSqr(closest - sphere.Position);
	return magSq <= (sphere.Radius * sphere.Radius);
}

bool Triangle::Intersects(OBB& obb)
{
	// Edge vectors
	vec3 f0 = B - A;
	vec3 f1 = C - B;
	vec3 f2 = A - C;

	// OBB face normals
	vec3 u0 = obb.Orientation[0];
	vec3 u1 = obb.Orientation[1];
	vec3 u2 = obb.Orientation[2];

	// Different axes to test
	vec3 test[13] =
	{
		u0, u1, u2,
		cross(f0, f1),
		cross(u0, f0), cross(u0, f1), cross(u0, f2),
		cross(u1, f0), cross(u1, f1), cross(u1, f2),
		cross(u2, f0), cross(u2, f1), cross(u2, f2)
	};

	for (int i = 0; i < 13; i++)
		if (!OverlapOnAxis(obb, test[i]))
			return false; // Separating axis found
	return true;
}

bool Triangle::Intersects(AABB& aabb)
{
	// Edge vectors
	vec3 f0 = B - A;
	vec3 f1 = C - B;
	vec3 f2 = A - C;

	// Face normals
	vec3 u0 = { 1, 0, 0 };
	vec3 u1 = { 0, 1, 0 };
	vec3 u2 = { 0, 0, 1 };

	// Different axes to test
	vec3 test[13] =
	{
		u0, u1, u2,
		cross(f0, f1),
		cross(u0, f0), cross(u0, f1), cross(u0, f2),
		cross(u1, f0), cross(u1, f1), cross(u1, f2),
		cross(u2, f0), cross(u2, f1), cross(u2, f2)
	};

	for (int i = 0; i < 13; i++)
		if (!OverlapOnAxis(aabb, test[i]))
			return false; // Separating axis found
	return true;
}

bool Triangle::Intersects(Plane& plane)
{
	float side1 = plane.PlaneEquation(A);
	float side2 = plane.PlaneEquation(B);
	float side3 = plane.PlaneEquation(C);

	if (BasicallyZero(side1) &&
		BasicallyZero(side2) &&
		BasicallyZero(side3))
		return true; // All points are on the plane

	if ((side1 > 0 && side2 > 0 && side3 > 0) ||
		(side1 < 0 && side2 < 0 && side3 < 0))
		return false; // All points are on the same side of the plane, away from it

	return true; // At least one point was on one side, with another on another side
}

bool Triangle::Intersects(Triangle& tri)
{
	vec3 axisToTest[] =
	{
		SatCrossEdge(A, B, B, C),				  // Triangle 1 normal
		SatCrossEdge(tri.A, tri.B, tri.B, tri.C), // Triangle 2 normal

		SatCrossEdge(tri.A, tri.B, A, B),
		SatCrossEdge(tri.A, tri.B, B, C),
		SatCrossEdge(tri.A, tri.B, C, A),

		SatCrossEdge(tri.B, tri.C, A, B),
		SatCrossEdge(tri.B, tri.C, B, C),
		SatCrossEdge(tri.B, tri.C, C, A),

		SatCrossEdge(tri.C, tri.A, A, B),
		SatCrossEdge(tri.C, tri.A, B, C),
		SatCrossEdge(tri.C, tri.A, C, A)
	};

	for (int i = 0; i < 11; i++)
		if (!OverlapOnAxis(tri, axisToTest[i]))
			return false; // Separating axis found
	return true;
}

Interval Triangle::GetInterval(vec3& axis)
{
	Interval result;
	result.Min = dot(axis, Points[0]);
	result.Max = result.Min;

	for (int i = 1; i < 3; i++)
	{
		float value = dot(axis, Points[i]);
		result.Min = fminf(result.Min, value);
		result.Max = fmaxf(result.Max, value);
	}

	return result;
}

bool Triangle::OverlapOnAxis(Triangle& tri, vec3& axis)
{
	Interval a = GetInterval(axis);
	Interval b = tri.GetInterval(axis);
	return (b.Min <= a.Max) && (a.Min <= b.Max);
}

bool Triangle::OverlapOnAxis(AABB& aabb, vec3& axis)
{
	Interval a = aabb.GetInterval(axis);
	Interval b = GetInterval(axis);
	return (b.Min <= a.Max) && (a.Min <= b.Max);
}

bool Triangle::OverlapOnAxis(OBB& obb, vec3& axis)
{
	Interval a = obb.GetInterval(axis);
	Interval b = GetInterval(axis);
	return (b.Min <= a.Max) && (a.Min <= b.Max);
}

vec3 Triangle::SatCrossEdge(vec3& a, vec3& b, vec3& c, vec3& d)
{
	vec3 ab = a - b;
	vec3 cd = c - d;
	vec3 result = cross(ab, cd);
	if (BasicallyZero(MagnitudeSqr(result)))
		return result; // Not parrallel
	vec3 axis = cross(ab, c - a);
	result = cross(ab, axis);
	if (BasicallyZero(MagnitudeSqr(result)))
		return result; // Now not parrallel
	return { 0, 0, 0 }; // No way to get proper cross product
}

vec3 Triangle::Barycentric(vec3& point)
{
	vec3 ap = point - A;
	vec3 bp = point - B;
	vec3 cp = point - C;

	vec3 ab = B - A;
	vec3 ac = C - A;
	vec3 bc = C - B;
	vec3 cb = B - C;
	vec3 ca = A - C;

	vec3 v = ab - project(ab, cb);
	float a = 1.0f - (dot(v, ap) / dot(v, ab));

	v = bc - project(bc, ac);
	float b = 1.0f - (dot(v, bp) / dot(v, bc));

	v = ca - project(ca, ab);
	float c = 1.0f - (dot(v, cp) / dot(v, ca));

	return { a, b, c };
}

bool Triangle::Raycast(Ray& ray, RaycastHit* outResult)
{
	if (outResult)
		*outResult = RaycastHit{}; // Reset to initial values

	Plane plane(*this);
	RaycastHit hit;
	if (!plane.Raycast(ray, &hit))
		// Ray is not hitting plane, therefore will definitely not hit this triangle
		return false;

	vec3 barycentric = Barycentric(hit.Point);
	if (barycentric.x >= 0.0f && barycentric.x <= 1.0f &&
		barycentric.y >= 0.0f && barycentric.y <= 1.0f &&
		barycentric.z >= 0.0f && barycentric.z <= 1.0f)
	{
		if (outResult)
			*outResult = hit;
		return true;
	}
	return false;
}

bool Triangle::LineTest(Line& line)
{
	Ray ray;
	ray.Origin = line.Start;
	ray.Direction = normalize(line.End - line.Start);
	RaycastHit hit;
	if (!Raycast(ray, &hit))
		return false;
	return hit.Distance >= 0 && hit.Distance <= line.Length();
}
#pragma endregion
