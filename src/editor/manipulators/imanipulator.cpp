#include "imanipulator.h"

vec4 AxesColors[3] = {{1,0,0,1},{0,1,0,1},{0,0,1,1}};
extern const char* PrimitiveShaderName = "primitive.hlsl";

IManupulator::IManupulator(QObject *parent) : QObject(parent)
{
}
