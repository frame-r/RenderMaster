#include "manipulatorrotator.h"
#include "../editorcore.h"
#include "mesh.h"
#include "resource_manager.h"
#include "core.h"
#include "render.h"
#include "../editor_common.h"

const static int CircleSubdivision = 40;
const static vec4 ColorRed = vec4(1,0,0,1);
const static vec4 ColorGreen = vec4(0,1,0,1);

static ManagedPtr<Mesh> meshLine;

static mat4 CircleCorrectionMat[3];


ManipulatorRotator::ManipulatorRotator()
{
	auto *resMan = editor->core->GetResourceManager();
	meshLine = resMan->CreateStreamMesh("std#line");

	CircleCorrectionMat[0].el_2D[0][0] = 0.0f;
	CircleCorrectionMat[0].el_2D[2][2] = 0.0f;
	CircleCorrectionMat[0].el_2D[2][0] = 1.0f;
	CircleCorrectionMat[0].el_2D[0][2] = 1.0f;

	CircleCorrectionMat[1].el_2D[2][2] = 0.0f;
	CircleCorrectionMat[1].el_2D[1][1] = 0.0f;
	CircleCorrectionMat[1].el_2D[1][2] = 1.0f;
	CircleCorrectionMat[1].el_2D[2][1] = 1.0f;
}

ManipulatorRotator::~ManipulatorRotator()
{
	meshLine.release();
}

void ManipulatorRotator::render(const CameraData &cam, const mat4 selectionTransform, const QRect &screen)
{
	Render *render = editor->core->GetRender();
	Shader *shader = render->GetShader("primitive.shader", meshLine.get());

	if (!shader)
		return;

	ICoreRender *coreRender = editor->core->GetCoreRender();

	coreRender->SetShader(shader);
	coreRender->SetDepthTest(0);

	vec4 center = vec4(selectionTransform.Column3(3));
	float hordLen = 2 * sin(3.1415926f * 1.0f / CircleSubdivision);
	mat4 scaleMat(0.7f * axisScale(center, cam.ViewMat, cam.ProjectionMat, QPoint(screen.width(), screen.height())));

	for (int j = 0; j < 3; j++)
	{
		shader->SetVec4Parameter("main_color", &AxesColors[j]);
		mat4 circleTransform = cam.ViewProjMat * selectionTransform * scaleMat * CircleCorrectionMat[j];

		for (int i = 0; i < CircleSubdivision; i++)
		{
			float angle = 3.1415926f * 2.0f * static_cast<float>(i) / CircleSubdivision;
			float x = sin(angle);
			float y = cos(angle);

			float angle1 = 3.1415926f * 2.0f * static_cast<float>(i + 1) / CircleSubdivision;
			float x1 = sin(angle1);
			float y1 = cos(angle1);

			vec2 xx;
			xx.x = x - x1;
			xx.y = y - y1;
			xx.Normalize();

			mat4 segmentTransform;
			segmentTransform.el_2D[0][3] = x;
			segmentTransform.el_2D[1][3] = y;
			segmentTransform.el_2D[0][0] = -xx.x * hordLen;
			segmentTransform.el_2D[1][0] = -xx.y * hordLen;
			segmentTransform.el_2D[0][1] = -xx.y;
			segmentTransform.el_2D[1][1] = xx.x;

			mat4 MVP = circleTransform * segmentTransform;
			shader->SetMat4Parameter("MVP", &MVP);

			shader->FlushParameters();

			coreRender->Draw(meshLine.get(), 1);
		}
	}
}

void ManipulatorRotator::update(const CameraData &cam, const mat4 selectionTransform, const QRect &screen, const vec2 &normalizedMousePos)
{

}

bool ManipulatorRotator::isMouseIntersect(const vec2 &normalizedMousePos)
{
	return false;
}

void ManipulatorRotator::mousePress(const CameraData &cam, const mat4 selectionTransform, const QRect &screen, const vec2 &normalizedMousePos)
{

}

void ManipulatorRotator::mouseRelease()
{

}
