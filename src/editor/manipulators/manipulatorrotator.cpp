#include "manipulatorrotator.h"
#include "../editorcore.h"
#include "mesh.h"
#include "resource_manager.h"
#include "core.h"
#include <qdebug.h>
#include "render.h"
#include "../editor_common.h"
#include "gameobject.h"

const static int CircleSubdivision = 15;
const static vec4 ColorRed = vec4(1,0,0,1);
const static vec4 ColorGreen = vec4(0,1,0,1);
static vec2 Segements[CircleSubdivision]{};
const static float hordLen = sin(3.1415926f * 1.0f / CircleSubdivision);;

static ManagedPtr<Mesh> meshLine;
static ManagedPtr<Mesh> meshSphere;

static mat4 CircleCorrectionMat[3];


ManipulatorRotator::ManipulatorRotator()
{
	auto *resMan = editor->core->GetResourceManager();
	meshLine = resMan->CreateStreamMesh("std#line");
	meshSphere = resMan->CreateStreamMesh("standard\\meshes\\sphere.mesh");

	CircleCorrectionMat[0].el_2D[0][0] = 0.0f;
	CircleCorrectionMat[0].el_2D[2][2] = 0.0f;
	CircleCorrectionMat[0].el_2D[2][0] = 1.0f;
	CircleCorrectionMat[0].el_2D[0][2] = 1.0f;

	CircleCorrectionMat[1].el_2D[2][2] = 0.0f;
	CircleCorrectionMat[1].el_2D[1][1] = 0.0f;
	CircleCorrectionMat[1].el_2D[1][2] = 1.0f;
	CircleCorrectionMat[1].el_2D[2][1] = 1.0f;

	for (int i = 0; i < CircleSubdivision; i++)
	{
		Segements[i].x = 3.1415926f * 1.0f * static_cast<float>(i) / CircleSubdivision;
		Segements[i].y = 3.1415926f * 1.0f * static_cast<float>(i + 1) / CircleSubdivision;
	}
}

ManipulatorRotator::~ManipulatorRotator()
{
	meshLine.release();
	meshSphere.release();
}

void ManipulatorRotator::render(const CameraData &cam, const mat4& selectionTransform, const QRect &screen)
{
	Render *render = editor->core->GetRender();

	Shader *shader;
	mat4 MVP;

	ICoreRender *coreRender = editor->core->GetCoreRender();
	coreRender->PushStates();

	coreRender->SetDepthTest(1);
	coreRender->SetCullingMode(CULLING_MODE::BACK);

	float hordLen = 1 * sin(3.1415926f * 1.0f / CircleSubdivision);
	vec4 center = vec4(selectionTransform.Column3(3));
	mat4 scaleMat(0.7f * axisScale(center, cam.ViewMat, cam.ProjectionMat, QPoint(screen.width(), screen.height())));

	shader = render->GetShader(PrimitiveShaderName, meshLine.get());
	if (!shader)
		return;

	coreRender->SetShader(shader);

	mat4 invSelectionWorldTransform = selectionTransform.Inverse();
	vec4 camPos_axesSpace = invSelectionWorldTransform * vec4(cam.pos);
	float offsets[3] = {atan2(camPos_axesSpace.z, camPos_axesSpace.y),
						atan2(camPos_axesSpace.x, camPos_axesSpace.z),
						atan2(camPos_axesSpace.x, camPos_axesSpace.y)};

	for (int j = 0; j < 3; j++)
	{
		vec4 col = AxesColors[j];
		if (underMouse != ELEMENT::NONE && (int)underMouse - 1 == j)
			col = ColorYellow;
		shader->SetVec4Parameter("main_color", &col);
		mat4 circleTransform = cam.ViewProjMat * selectionTransform * scaleMat * CircleCorrectionMat[j];

		offsets[j] -= 3.1415926f * 0.5f;

		//Plane plane(axesWorldDirection[j], center);
		//Ray ray = MouseToRay(cam.WorldTransform, cam.fovInDegrees, cam.aspect, normalizedMousePos);

		for (int i = 0; i < CircleSubdivision; i++)
		{
			vec2 p1;
			p1.x = sin(offsets[j] + Segements[i].x);
			p1.y = cos(offsets[j] + Segements[i].x);

			vec2 p2;
			p2.x = sin(offsets[j] + Segements[i].y);
			p2.y = cos(offsets[j] + Segements[i].y);

			vec2 diff = p1 - p2;
			diff.Normalize();

			mat4 segmentTransform;
			segmentTransform.el_2D[0][3] = p1.x;
			segmentTransform.el_2D[1][3] = p1.y;
			segmentTransform.el_2D[0][0] = -diff.x * hordLen;
			segmentTransform.el_2D[1][0] = -diff.y * hordLen;
			segmentTransform.el_2D[0][1] = -diff.y;
			segmentTransform.el_2D[1][1] = diff.x;

			//if (RayPlaneIntersection(out.worldPos, plane, ray))
			//{
			//	vec4 A_WS = segmentTransform * vec4(0,0,0,1);
			//	vec4 B_WS = segmentTransform * vec4(1,0,0,1);
			//
			//	vec2 A = NdcToScreen(WorldToNdc(vec3(A_WS), cam.ViewProjMat), screen.width(), screen.height());
			//	vec2 B = NdcToScreen(WorldToNdc(vec3(B_WS), cam.ViewProjMat), screen.width(), screen.height());
			//}

			MVP = circleTransform * segmentTransform;
			shader->SetMat4Parameter("MVP", &MVP);
			shader->FlushParameters();

			coreRender->Draw(meshLine.get(), 1);
		}
	}

	coreRender->SetBlendState(BLEND_FACTOR::SRC_ALPHA, BLEND_FACTOR::ONE_MINUS_SRC_ALPHA); // alpha blend

	shader = render->GetShader(PrimitiveShaderName, meshSphere.get());
	if (!shader)
		return;
	coreRender->SetShader(shader);

	MVP = cam.ViewProjMat *selectionTransform * scaleMat * 0.7;
	shader->SetVec4Parameter("main_color", &ColorTransparent);
	shader->SetMat4Parameter("MVP", &MVP);
	shader->FlushParameters();
	coreRender->Draw(meshSphere.get(), 1);


	coreRender->PopStates();
}

void ManipulatorRotator::update(const CameraData &cam, const mat4& selectionTransform, const QRect &screen, const vec2 &normalizedMousePos)
{
	if (state_ == STATE::NONE)
	{
		vec4 center = vec4(selectionTransform.Column3(3));
		vec3 center3 = selectionTransform.Column3(3);

		mat4 scaleMat(0.7f * axisScale(center, cam.ViewMat, cam.ProjectionMat, QPoint(screen.width(), screen.height())));

		vec3 axesWorldDirection[3] = { selectionTransform.Column3(0).Normalized(),
										selectionTransform.Column3(1).Normalized(),
										selectionTransform.Column3(2).Normalized()};

		mat4 invSelectionWorldTransform = selectionTransform.Inverse();

		vec4 camPos_axesSpace = invSelectionWorldTransform * vec4(cam.pos);
		float offsets[3] = {atan2(camPos_axesSpace.z, camPos_axesSpace.y),
							atan2(camPos_axesSpace.x, camPos_axesSpace.z),
							atan2(camPos_axesSpace.x, camPos_axesSpace.y)};

		int axisIndex = -1;
		float minDist = 1000000.0f;
		vec3 worldPos;

		Plane rotPlanes[3];

		for (int j = 0; j < 3; j++)
		{
			rotPlanes[j] = Plane(axesWorldDirection[j], center3);

			mat4 circleTransform = selectionTransform * scaleMat * CircleCorrectionMat[j];

			Ray ray = MouseToRay(cam.WorldTransform, cam.fovInDegrees, cam.aspect, normalizedMousePos);
			offsets[j] -= 3.1415926f * 0.5f;

			for (int i = 0; i < CircleSubdivision; i++)
			{
				vec2 p1;
				p1.x = sin(offsets[j] + Segements[i].x);
				p1.y = cos(offsets[j] + Segements[i].x);

				vec2 p2;
				p2.x = sin(offsets[j] + Segements[i].y);
				p2.y = cos(offsets[j] + Segements[i].y);

				vec2 diff = p1 - p2;
				diff.Normalize();

				mat4 segmentTransform;
				segmentTransform.el_2D[0][3] = p1.x;
				segmentTransform.el_2D[1][3] = p1.y;
				segmentTransform.el_2D[0][0] = -diff.x * hordLen;
				segmentTransform.el_2D[1][0] = -diff.y * hordLen;
				segmentTransform.el_2D[0][1] = -diff.y;
				segmentTransform.el_2D[1][1] = diff.x;

				vec3 I_WS;
				if ( abs(ray.direction.Dot(axesWorldDirection[j].Normalized())) > 0.01f && RayPlaneIntersection(I_WS, rotPlanes[j], ray))
				{
					vec4 A_WS = circleTransform * segmentTransform * vec4(0,0,0,1);
					vec4 B_WS = circleTransform * segmentTransform * vec4(1,0,0,1);

					vec2 A = NdcToScreen(WorldToNdc(vec3(A_WS), cam.ViewProjMat), screen);
					vec2 B = NdcToScreen(WorldToNdc(vec3(B_WS), cam.ViewProjMat), screen);
					vec2 I = NdcToScreen(WorldToNdc(I_WS, cam.ViewProjMat), screen);
					float d = PointToSegmentDistance(A, B, I);

					//qDebug() << "A_WS" << vec3ToString(vec3(A_WS)) << "A(NDC)" << d;

					if (d < minDist)
					{
						minDist = d;
						axisIndex = j;
						worldPos = I_WS;
					}
				}
			}
		}

		//qDebug() << "minDist" << minDist;

		if (minDist < SelectionThresholdInPixels && axisIndex >= 0)
		{
			underMouse = static_cast<ELEMENT>(axisIndex + 1);

			vec4 intersection_axesSpace = invSelectionWorldTransform * vec4(worldPos);
			float offsets[3] = {atan2(intersection_axesSpace.z, intersection_axesSpace.y),
								atan2(intersection_axesSpace.x, intersection_axesSpace.z),
								atan2(intersection_axesSpace.x, intersection_axesSpace.y)};

			startAngle = offsets[axisIndex];

			rotatePlane = rotPlanes[axisIndex];
			//qDebug() << startAngle;
		}
		else
			underMouse = ELEMENT::NONE;
	} else
	{
		Ray ray = MouseToRay(cam.WorldTransform, cam.fovInDegrees, cam.aspect, normalizedMousePos);

		vec3 I_WS;
		RayPlaneIntersection(I_WS, rotatePlane, ray);

		vec4 intersection_axesSpace = startInvSelectionWorldTransform * vec4(I_WS);

		float offsets[3] = {atan2(intersection_axesSpace.z, intersection_axesSpace.y),
							atan2(intersection_axesSpace.x, intersection_axesSpace.z),
							atan2(intersection_axesSpace.x, intersection_axesSpace.y)};

		float newOffset = offsets[(int)state_ - 1];
		float diff = (state_ == STATE::ROTATING_Z ? -1.0f : 1.0f) * (newOffset - startAngle);

		vec3 xyz = rotatePlane.normal * sin(diff/2);
		float w = cos(diff/2);
		quat newQuat = quat(xyz.x, xyz.y, xyz.z, w);
		newQuat = newQuat * startQuat;

		GameObject *obj = editor->FirstSelectedObjects();
		obj->SetWorldRotation(newQuat);

		//qDebug() << newOffset;
	}
}

bool ManipulatorRotator::isMouseIntersect(const vec2 &normalizedMousePos)
{
	return underMouse != ELEMENT::NONE;
}

void ManipulatorRotator::mousePress(const CameraData &cam, const mat4& selectionTransform, const QRect &screen, const vec2 &normalizedMousePos)
{
	if (underMouse != ELEMENT::NONE)
	{
		state_ = static_cast<STATE>((int)underMouse);
		startQuat = editor->FirstSelectedObjects()->GetWorldRotation();
		startInvSelectionWorldTransform = selectionTransform.Inverse();
		//qDebug() << startAngle;
	}
}

void ManipulatorRotator::mouseRelease()
{
	state_ = STATE::NONE;
}
