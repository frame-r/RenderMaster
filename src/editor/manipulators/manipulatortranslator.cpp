#include "manipulatortranslator.h"
#include "../editorcore.h"
#include "../editor_common.h"
#include "resource_manager.h"
#include "render.h"
#include "core.h"
#include "gameobject.h"
#include <qdebug.h>

vec4 ColorYellow = vec4(1,1,0,1);
vec4 ColorTransparent = vec4(1,1,1,0.17f);
const static vec4 ColorRed = vec4(1,0,0,1);
const static vec4 ColorGreen = vec4(0,1,0,1);
const static vec4 ColorBlue = vec4(0,0,1,1);
const static vec4 ColorMagneta = vec4(0,1,1,1);
const static float PlaneScale = 0.2f;

static StreamPtr<Mesh> meshArrow;
static StreamPtr<Mesh> meshLine;
static StreamPtr<Mesh> meshPlane;

static mat4 AxesCorrectionMat[3];

static mat4 PlaneCorrectionMat[3];
static mat4 RuntimeSideMat[3];
static vec4 PlaneTriangles[2][3];

ManipulatorTranslator::ManipulatorTranslator()
{
	auto *resMan = editor->core->GetResourceManager();
	meshArrow = resMan->CreateStreamMesh("std#axes_arrows");
	meshLine = resMan->CreateStreamMesh("std#line");
	meshPlane = resMan->CreateStreamMesh("std#plane");

	AxesCorrectionMat[1].el_2D[0][0] = 0.0f;
	AxesCorrectionMat[1].el_2D[1][1] = 0.0f;
	AxesCorrectionMat[1].el_2D[1][0] = 1.0f;
	AxesCorrectionMat[1].el_2D[0][1] = 1.0f;
	AxesCorrectionMat[2].el_2D[0][0] = 0.0f;
	AxesCorrectionMat[2].el_2D[2][2] = 0.0f;
	AxesCorrectionMat[2].el_2D[2][0] = 1.0f;
	AxesCorrectionMat[2].el_2D[0][2] = 1.0f;

	PlaneCorrectionMat[0].el_2D[0][0] = PlaneScale;
	PlaneCorrectionMat[0].el_2D[1][1] = PlaneScale;
	PlaneCorrectionMat[0].el_2D[0][3] = PlaneScale;
	PlaneCorrectionMat[0].el_2D[1][3] = PlaneScale;

	PlaneCorrectionMat[1].el_2D[0][0] = PlaneScale;
	PlaneCorrectionMat[1].el_2D[0][3] = PlaneScale;
	PlaneCorrectionMat[1].el_2D[1][1] = 0.0f;
	PlaneCorrectionMat[1].el_2D[1][2] = PlaneScale;
	PlaneCorrectionMat[1].el_2D[2][1] = PlaneScale;
	PlaneCorrectionMat[1].el_2D[2][2] = 0.0f;
	PlaneCorrectionMat[1].el_2D[2][3] = PlaneScale;

	PlaneCorrectionMat[2].el_2D[0][0] = 0.0f;
	PlaneCorrectionMat[2].el_2D[0][2] = PlaneScale;
	PlaneCorrectionMat[2].el_2D[1][1] = PlaneScale;
	PlaneCorrectionMat[2].el_2D[1][3] = PlaneScale;
	PlaneCorrectionMat[2].el_2D[2][0] = PlaneScale;
	PlaneCorrectionMat[2].el_2D[2][2] = 0.0f;
	PlaneCorrectionMat[2].el_2D[2][3] = PlaneScale;

	PlaneTriangles[0][0] = vec4(-1.0f, -1.0f, 0.0f, 1.0f);
	PlaneTriangles[0][1] = vec4(+1.0f, -1.0f, 0.0f, 1.0f);
	PlaneTriangles[0][2] = vec4(+1.0f, +1.0f, 0.0f, 1.0f);
	PlaneTriangles[1][0] = vec4(-1.0f, -1.0f, 0.0f, 1.0f);
	PlaneTriangles[1][1] = vec4(+1.0f, +1.0f, 0.0f, 1.0f);
	PlaneTriangles[1][2] = vec4(-1.0f, +1.0f, 0.0f, 1.0f);
}

ManipulatorTranslator::~ManipulatorTranslator()
{
	meshArrow.release();
	meshLine.release();
	meshPlane.release();
}

void ManipulatorTranslator::render(const CameraData& cam, const mat4& selectionTransform, const QRect& screen)
{
	Render *render = editor->core->GetRender();
	Shader *shader = render->GetShader(PrimitiveShaderName, meshLine.get());

	if (!shader)
		return;

	ICoreRender *coreRender = editor->core->GetCoreRender();

	coreRender->SetShader(shader);
	coreRender->SetDepthTest(0);

	vec4 center4 = vec4(selectionTransform.Column3(3));
	mat4 distanceScaleMat = mat4(axisScale(center4, cam.ViewMat, cam.ProjectionMat, QPoint(screen.width(), screen.height())));

	mat4 invSelectionWorldTransform = selectionTransform.Inverse();
	vec4 camPos_axesSpace = invSelectionWorldTransform * vec4(cam.pos);

	// Orientate manipulator-planes to viewer
	if (state == STATE::NONE)
	{
		RuntimeSideMat[0].el_2D[0][0] = camPos_axesSpace.x > 0 ? 1.0f : -1.0f;
		RuntimeSideMat[0].el_2D[1][1] = camPos_axesSpace.y > 0 ? 1.0f : -1.0f;
		RuntimeSideMat[2].el_2D[1][1] = camPos_axesSpace.y > 0 ? 1.0f : -1.0f;
		RuntimeSideMat[2].el_2D[2][2] = camPos_axesSpace.z > 0 ? 1.0f : -1.0f;
		RuntimeSideMat[1].el_2D[0][0] = camPos_axesSpace.x > 0 ? 1.0f : -1.0f;
		RuntimeSideMat[1].el_2D[2][2] = camPos_axesSpace.z > 0 ? 1.0f : -1.0f;
	}

	for (int i = 0; i < 3; i++)
	{
		vec4 col;
		if ((int)underMouse == i)
			col = ColorYellow;
		else
			col = AxesColors[i];

		mat4 MVP = cam.ViewProjMat * selectionTransform * distanceScaleMat * AxesCorrectionMat[i];
		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetVec4Parameter("main_color", &ColorYellow);
		shader->FlushParameters();
		coreRender->Draw(meshLine.get(), 1);

		if ((i == 0 && RuntimeSideMat[0].el_2D[0][0] < 0)
				|| (i == 1 && RuntimeSideMat[2].el_2D[1][1] < 0)
				|| (i == 2 && RuntimeSideMat[1].el_2D[2][2] < 0))
		{
			mat4 lineMat = MVP * -PlaneScale * 2.0f;
			shader->SetMat4Parameter("MVP", &lineMat);
			shader->SetVec4Parameter("main_color", &ColorYellow);
			shader->FlushParameters();
			coreRender->Draw(meshLine.get(), 1);
		}

		MVP = MVP * 1.04f;
		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetVec4Parameter("main_color", &col);
		shader->FlushParameters();
		coreRender->Draw(meshArrow.get(), 1);
	}

	coreRender->SetBlendState(BLEND_FACTOR::SRC_ALPHA, BLEND_FACTOR::ONE_MINUS_SRC_ALPHA); // additive

	mat4 planeTransform = cam.ViewProjMat * selectionTransform * distanceScaleMat;

	for (int i = 0; i < 3; i++)
	{
		mat4 MVP = planeTransform * RuntimeSideMat[i] * PlaneCorrectionMat[i];

		vec4 col;
		if ((int)underMouse == i + 3)
		{ col = ColorYellow; col.w = 0.5f; }
		else
			col = ColorTransparent;

		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetVec4Parameter("main_color", &col);
		shader->FlushParameters();

		coreRender->Draw(meshPlane.get(), 1);
	}
}

void ManipulatorTranslator::updateMouse(const CameraData& cam, const mat4& selectionWorldTransform, const QRect& screen, const vec2 &normalizedMousePos)
{
	if (oldNormalizedMousePos.Aproximately(normalizedMousePos))
		return;

	oldNormalizedMousePos = normalizedMousePos;

	if (state == STATE::MOVING_ARROW_HANDLE) // axis movement => update position
	{
		AxisIntersection I = intersectMouseWithAxis(cam, selectionWorldTransform, screen, normalizedMousePos, movesAlongLine.direction, underMouse);

		if (I.minDistToAxes < MaxDistanceInPixels)
		{
			vec3 pos = movesAlongLine.projectPoint(I.worldPos) - worldDelta;
			editor->FirstSelectedObjects()->SetWorldPosition(pos);
		}

	} else if (state == STATE::MOVING_PLANE_HANDLE) // plane movement => update position
	{
		Ray ray = MouseToRay(cam.WorldTransform, cam.fovInDegrees, cam.aspect, normalizedMousePos);

		vec3 worldIntersection;
		if (RayPlaneIntersection(worldIntersection, movesAlongPlane, ray))
		{
			vec3 pos = worldIntersection - worldDelta;
			editor->FirstSelectedObjects()->SetWorldPosition(pos);
		}
	} else // Intersection with manipulator handles and set "underMouse"
	{
		underMouse = MANIPULATOR_ELEMENT::NONE;

		vec3 center = selectionWorldTransform.Column3(3);

		vec3 axesWorldDirection[3] = { selectionWorldTransform.Column3(0).Normalized(),
										selectionWorldTransform.Column3(1).Normalized(),
										selectionWorldTransform.Column3(2).Normalized()};
		float minDist = MaxDistanceInPixels;

		// Intersection with planes
		{
			vec4 center4 = vec4(selectionWorldTransform.Column3(3));
			mat4 distanceScaleMat = mat4(axisScale(center4, cam.ViewMat, cam.ProjectionMat, QPoint(screen.width(), screen.height())));
			mat4 planeTransform = cam.ViewProjMat * selectionWorldTransform * distanceScaleMat;
			vec2 ndcMouse = normalizedMousePos * 2.0f - vec2(1.0f, 1.0f);

			for (int k = 0; k < 3; k++) // each plane
			{
				mat4 axisToWorld = planeTransform * RuntimeSideMat[k] * PlaneCorrectionMat[k];
				for (int j = 0; j < 2; j++) // 2 triangle
				{
					vec4 ndc[3];
					for (int i = 0; i < 3; i++) // 3 point
					{
						ndc[i] = axisToWorld * PlaneTriangles[j][i];
						ndc[i] /= ndc[i].w;
					}

					if (PointInTriangle(ndcMouse, ndc[0], ndc[1], ndc[2]))
					{
						underMouse = static_cast<MANIPULATOR_ELEMENT>(int(MANIPULATOR_ELEMENT::XY) + k);
					}
				}
			}
		}

		// Intersection with axis
		if (underMouse == MANIPULATOR_ELEMENT::NONE)
		{
			for (int i = 0; i < 3; i++)
			{
				AxisIntersection I = intersectMouseWithAxis(cam, selectionWorldTransform, screen, normalizedMousePos, axesWorldDirection[i], (MANIPULATOR_ELEMENT)i);

				// find minimum distance to each axis
				if (I.minDistToAxes < SelectionThresholdInPixels && I.minDistToAxes < minDist)
				{
					minDist = I.minDistToAxes;
					underMouse = static_cast<MANIPULATOR_ELEMENT>(i);

					// Calculate worldDelta for axis
					movesAlongLine = Ray(axesWorldDirection[i], center);
					vec3 projectedToAxisLinePoint = movesAlongLine.projectPoint(I.worldPos);
					worldDelta = projectedToAxisLinePoint - center;
				}
			}
		}

		// Calculate worldDelta for plane
		if (isPlaneMovement(underMouse))
		{
			vec3 N;
			switch(underMouse)
			{
				case MANIPULATOR_ELEMENT::XY: N = axesWorldDirection[0].Cross(axesWorldDirection[1]); break;
				case MANIPULATOR_ELEMENT::ZX: N = axesWorldDirection[1].Cross(axesWorldDirection[2]); break;
				case MANIPULATOR_ELEMENT::YZ: N = axesWorldDirection[0].Cross(axesWorldDirection[2]); break;
			}

			movesAlongPlane = Plane(N, center);

			Ray ray = MouseToRay(cam.WorldTransform, cam.fovInDegrees, cam.aspect, normalizedMousePos);

			vec3 worldIntersection;
			if (RayPlaneIntersection(worldIntersection, movesAlongPlane, ray))
			{
				worldDelta = worldIntersection - center;
			}
		}
	}
}

bool ManipulatorTranslator::isMouseIntersect(const vec2 &)
{
	return underMouse != MANIPULATOR_ELEMENT::NONE;
}

void ManipulatorTranslator::mousePress(const CameraData &cam, const mat4& selectionTransform, const QRect &screen, const vec2 &normalizedMousePos)
{
	if (isAxisMovement(underMouse))
		state = STATE::MOVING_ARROW_HANDLE;
	else if (isPlaneMovement(underMouse))
		state = STATE::MOVING_PLANE_HANDLE;
}

void ManipulatorTranslator::mouseRelease()
{
	state = STATE::NONE;
}
