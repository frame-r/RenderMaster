#include "manipulatorscale.h"
#include "../editorcore.h"
#include "../editor_common.h"
#include "resource_manager.h"
#include "render.h"
#include "core.h"
#include "gameobject.h"

const static float PlaneScale = 0.2f;
static StreamPtr<Mesh> meshLine;
static StreamPtr<Mesh> meshCube;
static mat4 CubeTranslateMatDefault[3];
static mat4 CubeTranslateMat[3];
static mat4 AxesCorrectionMat[3];
static mat4 AxesCorrectionMatDefault[3];

ManipulatorScale::ManipulatorScale()
{
	auto *resMan = editor->core->GetResourceManager();
	meshLine = resMan->CreateStreamMesh("std#line");
	meshCube = resMan->CreateStreamMesh("std#cube");

	CubeTranslateMatDefault[0].el_2D[0][3] = 1.0f;
	CubeTranslateMatDefault[1].el_2D[1][3] = 1.0f;
	CubeTranslateMatDefault[2].el_2D[2][3] = 1.0f;

	AxesCorrectionMatDefault[1].el_2D[0][0] = 0.0f;
	AxesCorrectionMatDefault[1].el_2D[1][1] = 0.0f;
	AxesCorrectionMatDefault[1].el_2D[1][0] = 1.0f;
	AxesCorrectionMatDefault[1].el_2D[0][1] = 1.0f;
	AxesCorrectionMatDefault[2].el_2D[0][0] = 0.0f;
	AxesCorrectionMatDefault[2].el_2D[2][2] = 0.0f;
	AxesCorrectionMatDefault[2].el_2D[2][0] = 1.0f;
	AxesCorrectionMatDefault[2].el_2D[0][2] = 1.0f;
}

ManipulatorScale::~ManipulatorScale()
{
	meshLine.release();
	meshCube.release();
}

void ManipulatorScale::render(const CameraData &cam, const mat4 &selectionTransform, const QRect &screen)
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

	mat4 *CubeTranslatePtr;
	mat4 *AxesCorrectionPtr;

	if (state == STATE::SCALING_ARROW_HANDLE)
	{
		memcpy(CubeTranslateMat, CubeTranslateMatDefault, sizeof(CubeTranslateMatDefault));
		memcpy(AxesCorrectionMat, AxesCorrectionMatDefault, sizeof(AxesCorrectionMatDefault));

		CubeTranslateMat[0].el_2D[0][3] = scaleVector.x;
		CubeTranslateMat[1].el_2D[1][3] = scaleVector.y;
		CubeTranslateMat[2].el_2D[2][3] = scaleVector.z;

		AxesCorrectionMat[0].el_2D[0][0] = scaleVector.x;
		AxesCorrectionMat[1].el_2D[1][0] = scaleVector.y;
		AxesCorrectionMat[2].el_2D[2][0] = scaleVector.z;

		CubeTranslatePtr = CubeTranslateMat;
		AxesCorrectionPtr = AxesCorrectionMat;
	}
	else
	{
		CubeTranslatePtr = CubeTranslateMatDefault;
		AxesCorrectionPtr = AxesCorrectionMatDefault;
	}

	// axis
	for (int i = 0; i < 3; i++)
	{
		mat4 MVP = cam.ViewProjMat * selectionTransform * distanceScaleMat * AxesCorrectionPtr[i];
		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetVec4Parameter("main_color", &ColorYellow);
		shader->FlushParameters();
		coreRender->Draw(meshLine.get(), 1);
	}

	// cubes
	for(int i = 0; i < 3; i++)
	{
		vec4 col;
		if ((int)underMouse == i)
			col = ColorYellow;
		else
			col = AxesColors[i];

		mat4 scale = mat4{0.05f};
		mat4 MVP = cam.ViewProjMat * selectionTransform * distanceScaleMat * CubeTranslatePtr[i] * scale;
		shader->SetMat4Parameter("MVP", &MVP);

		shader->SetVec4Parameter("main_color", &col);
		shader->FlushParameters();
		coreRender->Draw(meshCube.get(), 1);
	}
}

void ManipulatorScale::updateMouse(const CameraData &cam, const mat4 &selectionTransform, const QRect &screen, const vec2 &normalizedMousePos)
{
	vec3 center = selectionTransform.Column3(3);

	if (state == STATE::SCALING_ARROW_HANDLE)
	{
		AxisIntersection I = intersectMouseWithAxis(cam, selectionTransform, screen, normalizedMousePos, movesAlongLine.direction, underMouse);

		if (I.minDistToAxes < MaxDistanceInPixels)
		{
			vec3 pos = movesAlongLine.projectPoint(I.worldPos);
			worldDelta = pos - center;
			float scale = /*(movesAlongLine.direction.Dot(worldDelta) >= 0.f? 1.f : -1.f) **/ worldDelta.Lenght() / initProjectedAxisDistance;

			scaleVector = {1.f, 1.f, 1.f};

			switch (underMouse) {
				case MANIPULATOR_ELEMENT::X: scaleVector.x *= scale; break;
				case MANIPULATOR_ELEMENT::Y: scaleVector.y *= scale; break;
				case MANIPULATOR_ELEMENT::Z: scaleVector.z *= scale; break;
			}

			vec3 newScale = originalLocalScale * scaleVector;

			editor->FirstSelectedObjects()->SetLocalScale(newScale);
		}

	}
	else if (state == STATE::NONE)
	{
		vec3 axesWorldDirection[3] = { selectionTransform.Column3(0).Normalized(),
										selectionTransform.Column3(1).Normalized(),
										selectionTransform.Column3(2).Normalized()};
		float minDist = MaxDistanceInPixels;

		underMouse = MANIPULATOR_ELEMENT::NONE;

		// Intersection with axis
		if (underMouse == MANIPULATOR_ELEMENT::NONE)
			for (int i = 0; i < 3; i++)
			{
				AxisIntersection I = intersectMouseWithAxis(cam, selectionTransform, screen, normalizedMousePos, axesWorldDirection[i], (MANIPULATOR_ELEMENT)i);

				// find minimum distance to each axis
				if (I.minDistToAxes < SelectionThresholdInPixels && I.minDistToAxes < minDist)
				{
					minDist = I.minDistToAxes;
					underMouse = static_cast<MANIPULATOR_ELEMENT>(i);

					movesAlongLine = Ray(axesWorldDirection[i], center);
					vec3 projectedToAxisLinePoint = movesAlongLine.projectPoint(I.worldPos);
					worldDelta = projectedToAxisLinePoint - center;
					initProjectedAxisDistance = worldDelta.Lenght();
				}
			}
	}
}

bool ManipulatorScale::isMouseIntersect(const vec2 &normalizedMousePos)
{
	return underMouse != MANIPULATOR_ELEMENT::NONE;
}

void ManipulatorScale::mousePress(const CameraData &cam, const mat4 &selectionTransform, const QRect &screen, const vec2 &normalizedMousePos)
{
	if (isAxisMovement(underMouse))
		state = STATE::SCALING_ARROW_HANDLE;

	originalLocalScale = editor->FirstSelectedObjects()->GetLocalScale();
}

void ManipulatorScale::mouseRelease()
{
	state = STATE::NONE;
	scaleVector = {1.f, 1.f, 1.f};
}
