#pragma once

#include <mesh/mesh_format.h>

namespace xtest{
namespace mesh {
	
	
	MeshData GeneratePlane(float xLength, float zLength, uint32 zDivisions, uint32 xDivisions);
	MeshData GenerateSphere(float radius, uint32 sliceCount, uint32 stackCount);
	MeshData GenerateBox(float xLenght, float yLenght, float zLenght);
	MeshData GenerateTorus(float max_radius, float min_radius, uint32 detailsCount);
	MeshData GenerateTorusKnot(float r, float R, float c, uint32 detailsCount, uint32 q, uint32 p);
	
} // xtest
} // mesh

