#pragma once

#include <mesh/mesh_format.h>

namespace xtest{
namespace mesh {
	
	
	MeshData GeneratePlane(float xLength, float zLength, uint32 zDivisions, uint32 xDivisions);
	MeshData GenerateSphere(float radius, uint32 sliceCount, uint32 stackCount);
	MeshData GenerateBox(float xLenght, float yLenght, float zLenght);
	MeshData GenerateTorus(float r, float R, int nsides, int rings);


} // xtest
} // mesh

