bool rayTriangleIntersect(float3 orig, float3 dir, float3 v0, float3 v1, float3 v2, out float3 hit)
{
	float3 v0v1 = v1 - v0;
	float3 v0v2 = v2 - v0;
	// no need to normalize
	float3 N = cross(v0v1, v0v2); // N 
	float area2 = length(N);

	// Step 1: finding P

	// check if ray and plane are parallel ?
	float NdotRayDirection = dot(N, dir);
	if (abs(NdotRayDirection) < 10e-5) // almost 0 
		return false; // they are parallel so they don't intersect ! 

	// compute d parameter using equation 2
	float d = dot(N, v0);

	// compute t (equation 3)
	float t = (-dot(N, orig) + d) / NdotRayDirection;
	// check if the triangle is in behind the ray
	if (t < 0) return false; // the triangle is behind 

	// compute the intersection point using equation 1
	hit = orig + dir * t;

	// Step 2: inside-outside test
	float3 C; // vector perpendicular to triangle's plane 

	// edge 0
	float3 edge0 = v1 - v0;
	float3 vp0 = hit - v0;
	C = cross(edge0, vp0);
	if (dot(N, C) < 0) return false; // P is on the right side 

	// edge 1
	float3 edge1 = v2 - v1;
	float3 vp1 = hit - v1;
	C = cross(edge1, vp1);
	if (dot(N, C) < 0)  return false; // P is on the right side 

	// edge 2
	float3 edge2 = v0 - v2;
	float3 vp2 = hit - v2;
	C = cross(edge2, vp2);
	if (dot(N, C) < 0) return false; // P is on the right side;

	return true; // this ray hits the triangle 
}


bool IntersectSphere(float3 center3, float radius, float3 orig, float3 dir, out float3 hit, out float3 N)
{
	hit = float3(0, 0, 0);
	N = float3(0, 0, 1);

	float t, t0, t1; // solutions for t if the ray intersects 

	//float3 center3 = sphere.center.xyz;

	// geometric solution
	float3 L = center3 - orig;
	float tca = dot(L, dir);
	// if (tca < 0) return false;
	float d2 = dot(L, L) - tca * tca;
	if (d2 > radius) return false;
	float thc = sqrt(radius - d2);
	t0 = tca - thc;
	t1 = tca + thc;

	if (t0 > t1)
	{
		float tmp = t0;
		t0 = t1;
		t1 = tmp;
	}

	if (t0 < 0) {
		t0 = t1; // if t0 is negative, let's use t1 instead 
		if (t0 < 0) return false; // both t0 and t1 are negative 
	}

	t = t0;
	hit = orig + dir * t;
	N = normalize(hit - center3);

	return true;
}

// iD -		0 -geometry
//			1 - lights

bool IntersectWorld(float3 orig, float3 dir, out float3 hit, out float3 N, out int id)
{
	#define MAX_DIST 1000

	float minDist = MAX_DIST;
	float3 retHit, retN;
	id = -1;

	for (uint j = 0; j < triCount; j++)
	{
		//rayTriangleIntersect(float3 orig, float3 dir, float3 v0, float3 v1, float3 v2, out float3 hit)
		if (rayTriangleIntersect(orig, dir, triangles[j].p0.xyz, triangles[j].p1.xyz, triangles[j].p2.xyz, hit))
		{
			float dist = length(hit - orig);
			if (dist < minDist)
			{
				minDist = dist;
				retHit = hit;
				retN = triangles[j].normal.xyz;
            }
		}
	}

	//for (uint i = 0; i < spheresCount; i++)
	//{
	//	if (IntersectSphere(spheres[i].center.xyz, spheres[i].radius2.x, orig, dir, hit, N))
	//	{
	//		float dist = length(hit - orig);
	//		if (dist < minDist)
	//		{
	//			minDist = dist;
	//			retHit = hit;
	//			retN = N;
 //              // id = 0;
 //           }
	//	}
	//}

	// Area light back face
	for (uint k = 0; k < lightsCount; k++)
	{
	    if (rayTriangleIntersect(orig, dir, lights[k].p0.xyz, lights[k].p1.xyz, lights[k].p2.xyz, hit) ||
		   rayTriangleIntersect(orig, dir, lights[k].p0.xyz, lights[k].p2.xyz, lights[k].p3.xyz, hit))
	    {
	        float dist = length(hit - orig);
	        if (dist < minDist && dot(dir, lights[k].normal.xyz) > 0.0f)
	        {
	            minDist = dist;
	            retHit = hit;
	            retN = -lights[k].normal.xyz;
	        }
	    }
	}

	N = retN;
	hit = retHit;

	return bool(minDist < MAX_DIST);
}

float3 GetWorldRay(float2 ndc, float3 forwardWS, float3 rightWS, float3 upWS)
{
	return normalize(forwardWS + rightWS * ndc.x + upWS * ndc.y);
}
