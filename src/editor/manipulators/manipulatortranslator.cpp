#include "manipulatortranslator.h"
#include "../editorcore.h"
#include "../editor_common.h"
#include "resource_manager.h"
#include "render.h"
#include "core.h"
#include "gameobject.h"
#include <qdebug.h>

static ManagedPtr<Mesh> meshArrow;
static ManagedPtr<Mesh> meshLine;

const static float SelectionThresholdInPixels = 8.0f;
const static float MaxDistanceInPixels = 1000000.0f;
const static vec3 AxesEndpoints[3] = {vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)};
const static vec4 ColorSelection = vec4(1,1,0,1);
const static vec4 ColorRed = vec4(1,0,0,1);
const static vec4 ColorGreen = vec4(0,1,0,1);
const static vec4 ColorBlue = vec4(0,0,1,1);
const static vec4 ColorMagneta = vec4(0,1,1,1);
mat4 static AxesCorrectionMat[3];

void intersectMouseWithAxis(const CameraData& cam, const mat4 selectionWorldTransform, const QRect &screen, const vec2 &normalizedMousePos, const vec3 &axisWorldSpace, MANIPULATOR_ELEMENT type, vec3 &worldOut, float &distance)
{
	vec3 center = selectionWorldTransform.Column3(3);
	vec4 center4 = vec4(selectionWorldTransform.Column3(3));
	vec3 V = (center - cam.pos).Normalized();
	vec3 VcrossAxis = V.Cross(axisWorldSpace).Normalized();
	vec3 N = axisWorldSpace.Cross(VcrossAxis).Normalized();

	Plane plane(N, center);

	Line3D ray = MouseToRay(cam.WorldTransform, cam.fovInDegrees, cam.aspect, normalizedMousePos);

	if (LineIntersectPlane(worldOut, plane, ray))
	{
	   vec2 A = NdcToScreen(WorldToNdc(center, cam.ViewProjMat), screen.width(), screen.height());

	   float distToCenter = DistanceTo(cam.ViewProjMat, selectionWorldTransform);
	   vec4 axisEndpointLocal = vec4(AxesEndpoints[(int)type] * axisScale(center4, cam.ViewMat, cam.ProjectionMat, QPoint(screen.width(), screen.height())));
	   vec4 axisEndpointWorld = selectionWorldTransform * axisEndpointLocal;
	   vec2 B = NdcToScreen(WorldToNdc((vec3&)axisEndpointWorld, cam.ViewProjMat), screen.width(), screen.height());

	   vec2 I = NdcToScreen(WorldToNdc(worldOut, cam.ViewProjMat), screen.width(), screen.height());

	   distance = PointToSegmentDistance(A, B, I);
	   return;
	}

	distance = MaxDistanceInPixels;
}

ManipulatorTranslator::ManipulatorTranslator()
{
	auto *resMan = editor->core->GetResourceManager();
	meshArrow = resMan->CreateStreamMesh("std#axes_arrows");
	meshLine = resMan->CreateStreamMesh("std#line");

	AxesCorrectionMat[1].el_2D[0][0] = 0.0f;
	AxesCorrectionMat[1].el_2D[1][1] = 0.0f;
	AxesCorrectionMat[1].el_2D[1][0] = 1.0f;
	AxesCorrectionMat[1].el_2D[0][1] = 1.0f;
	AxesCorrectionMat[2].el_2D[0][0] = 0.0f;
	AxesCorrectionMat[2].el_2D[2][2] = 0.0f;
	AxesCorrectionMat[2].el_2D[2][0] = 1.0f;
	AxesCorrectionMat[2].el_2D[0][2] = 1.0f;
}

ManipulatorTranslator::~ManipulatorTranslator()
{
	meshArrow.release();
	meshLine.release();
}

void ManipulatorTranslator::render(const CameraData& cam, const mat4 selectionTransform, const QRect& screen)
{
	Render *render = editor->core->GetRender();
	Shader *shader = render->GetShader({"primitive.shader", meshLine.get()->GetAttributes()});

	if (!shader)
		return;

	ICoreRender *coreRender = editor->core->GetCoreRender();

	coreRender->SetShader(shader);
	coreRender->SetDepthTest(0);

	vec4 center4 = vec4(selectionTransform.Column3(3));
	mat4 distanceScaleMat = mat4(axisScale(center4, cam.ViewMat, cam.ProjectionMat, QPoint(screen.width(), screen.height())));

	for (int i = 0; i < 3; i++)
	{
		mat4 M = selectionTransform * distanceScaleMat * AxesCorrectionMat[i];
		mat4 MVP = cam.ViewProjMat * M;
		mat4 NM = M.Inverse().Transpose();
		vec4 col;

		if ((int)underMouse == i)
			col = ColorSelection;
		else
			col = AxesColors[i];

		shader->SetMat4Parameter("MVP", &MVP);
		//shader->SetMat4Parameter("M", &M);
		//shader->SetMat4Parameter("NM", &NM);
		shader->SetVec4Parameter("main_color", &col);
		shader->FlushParameters();

		coreRender->Draw(meshLine.get(), 1);
		coreRender->Draw(meshArrow.get(), 1);
	}
}

void ManipulatorTranslator::update(const CameraData& cam, const mat4 selectionWorldTransform, const QRect& screen, const vec2 &normalizedMousePos)
{
	if (state == MANIPULATOR_STATE::MOVING_ARROW_HANDLE)
	{
		if (!oldNormalizedMousePos.Aproximately(normalizedMousePos))
		{
			oldNormalizedMousePos = normalizedMousePos;

			vec3 intersectionWorld;
			float intersectionDistance;

			intersectMouseWithAxis(cam, selectionWorldTransform, screen, normalizedMousePos, movesAlongLine.direction, underMouse, intersectionWorld, intersectionDistance);

			if (intersectionDistance < MaxDistanceInPixels)
			{
				vec3 pos = movesAlongLine.projectPoint(intersectionWorld) - worldDelta;
				editor->FirstSelectedObjects()->SetWorldPosition(pos);
			}
		}

	} else if (state == MANIPULATOR_STATE::MOVING_PLANE_HANDLE)
	{

	} else
	{
		underMouse = MANIPULATOR_ELEMENT::NONE;

		vec3 center = selectionWorldTransform.Column3(3);

		vec3 axesWorldDirection[3] = { selectionWorldTransform.Column3(0).Normalized(),
										selectionWorldTransform.Column3(1).Normalized(),
										selectionWorldTransform.Column3(2).Normalized()};
		float minDist = MaxDistanceInPixels;

		// Check if mouse intersects axes
		for (int i = 0; i < 3; i++)
		{
			vec3 intersectionWorld;
			float intersectionDistance;
			intersectMouseWithAxis(cam, selectionWorldTransform, screen, normalizedMousePos, axesWorldDirection[i], (MANIPULATOR_ELEMENT)i, intersectionWorld, intersectionDistance);

			// find minimum distance to each axis
			if (intersectionDistance < SelectionThresholdInPixels && intersectionDistance < minDist)
			{
				minDist = intersectionDistance;

				underMouse = static_cast<MANIPULATOR_ELEMENT>(i);

				Line3D axisLine = Line3D(axesWorldDirection[i], center);
				vec3 projectedToAxisLinePoint = axisLine.projectPoint(intersectionWorld);
				worldDelta = projectedToAxisLinePoint - center;
			}
		}

	//	if (underMouse != AXIS_ELEMENT::NONE)
	//		qDebug() << (int)underMouse << vec3ToString(worldDelta);
	}

}

bool ManipulatorTranslator::isMouseIntersect(const vec2 &normalizedMousePos)
{
	return underMouse != MANIPULATOR_ELEMENT::NONE;
}

void ManipulatorTranslator::mousePress(const CameraData &cam, const mat4 selectionTransform, const QRect &screen, const vec2 &normalizedMousePos)
{
	if (MANIPULATOR_ELEMENT::NONE < underMouse && underMouse <= MANIPULATOR_ELEMENT::Z)
	{
		state = MANIPULATOR_STATE::MOVING_ARROW_HANDLE;

		vec3 worldDirection = selectionTransform.Column3((int)underMouse).Normalized();
		vec3 center = selectionTransform.Column3(3);
		movesAlongLine = Line3D(worldDirection, center);
	}
	else if (MANIPULATOR_ELEMENT::XY <= underMouse && underMouse <= MANIPULATOR_ELEMENT::ZX)
	{
		state = MANIPULATOR_STATE::MOVING_PLANE_HANDLE;
	}
}

void ManipulatorTranslator::mouseRelease()
{
	state = MANIPULATOR_STATE::NONE;
}
