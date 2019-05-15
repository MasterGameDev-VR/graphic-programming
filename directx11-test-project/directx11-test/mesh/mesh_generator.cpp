#include "stdafx.h"
#include "mesh_generator.h"
#include <math/math_utils.h>


using namespace xtest::mesh;
using namespace DirectX;



xtest::mesh::MeshData xtest::mesh::GeneratePlane(float xLength, float zLength, uint32 zDivisions, uint32 xDivisions)
{
	const float halfWidth = 0.5f*xLength;
	const float halfDepth = 0.5f*zLength;
	const float dx = xLength / (xDivisions - 1);
	const float dz = zLength / (zDivisions - 1);
	const float du = 1.0f / (xDivisions - 1);
	const float dv = 1.0f / (zDivisions - 1);

	MeshData mesh;

	const uint32 vertexCount = zDivisions * xDivisions;
	mesh.vertices.resize(vertexCount);

	//vertices
	for (uint32 zIter = 0; zIter < zDivisions; ++zIter)
	{
		float z = halfDepth - zIter * dz;
		for (uint32 xIter = 0; xIter < xDivisions; ++xIter)
		{
			float x = -halfWidth + xIter * dx;

			mesh.vertices[zIter*xDivisions + xIter].position = { x, 0.0f, z };
			mesh.vertices[zIter*xDivisions + xIter].normal = { 0.0f, 1.0f, 0.0f };
			mesh.vertices[zIter*xDivisions + xIter].tangentU = { 1.0f, 0.0f, 0.0f, 1.f };

			mesh.vertices[zIter*xDivisions + xIter].uv.x = xIter * du;
			mesh.vertices[zIter*xDivisions + xIter].uv.y = zIter * dv;
		}
	}


	const uint32 faceCount = (zDivisions - 1)*(xDivisions - 1) * 2;
	mesh.indices.resize(faceCount * 3); // 3 indices per face
	
	// indices
	uint32 k = 0;
	for (uint32 zIter = 0; zIter < zDivisions - 1; ++zIter)
	{
		for (uint32 xIter = 0; xIter < xDivisions - 1; ++xIter)
		{
			mesh.indices[k] = zIter * xDivisions + xIter;
			mesh.indices[k + 1] = zIter * xDivisions + xIter + 1;
			mesh.indices[k + 2] = (zIter + 1)*xDivisions + xIter;

			mesh.indices[k + 3] = (zIter + 1)*xDivisions + xIter;
			mesh.indices[k + 4] = zIter * xDivisions + xIter + 1;
			mesh.indices[k + 5] = (zIter + 1)*xDivisions + xIter + 1;

			k += 6; // next quad
		}
	}

	return mesh;
}



xtest::mesh::MeshData xtest::mesh::GenerateSphere(float radius, uint32 sliceCount, uint32 stackCount)
{
	// poles vertices
	MeshData::Vertex topVertex = { {0.0f, +radius, 0.0f}, {0.0f, +1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 0.0f} };
	MeshData::Vertex bottomVertex = { {0.0f, -radius, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 1.0f} };

	MeshData mesh;
	mesh.vertices.push_back(topVertex);

	const float phiStep = XM_PI / stackCount;
	const float thetaStep = 2.0f*XM_PI / sliceCount;

	//rings vertices
	for (uint32 stack = 1; stack <= stackCount - 1; ++stack)
	{
		float phi = stack * phiStep;

		for (uint32 slice = 0; slice <= sliceCount; ++slice)
		{
			float theta = slice * thetaStep;

			MeshData::Vertex vertex;

			// spherical to cartesian
			vertex.position.x = radius * sinf(phi)*cosf(theta);
			vertex.position.y = radius * cosf(phi);
			vertex.position.z = radius * sinf(phi)*sinf(theta);

			// Partial derivative of P with respect to theta
			vertex.tangentU.x = -radius * sinf(phi)*sinf(theta);
			vertex.tangentU.y = 0.0f;
			vertex.tangentU.z = +radius * sinf(phi)*cosf(theta);

			XMVECTOR tangentU = XMLoadFloat4(&vertex.tangentU);
			XMStoreFloat4(&vertex.tangentU, XMVector3Normalize(tangentU));
			vertex.tangentU.w = 1.f;

			XMVECTOR position = XMLoadFloat3(&vertex.position);
			XMStoreFloat3(&vertex.normal, XMVector3Normalize(position));

			vertex.uv.x = theta / XM_2PI;
			vertex.uv.y = phi / XM_PI;

			mesh.vertices.push_back(vertex);
		}
	}

	mesh.vertices.push_back(bottomVertex);


	// top indices
	for (uint32 slice = 1; slice <= sliceCount; ++slice)
	{
		mesh.indices.push_back(0);
		mesh.indices.push_back(slice + 1);
		mesh.indices.push_back(slice);
	}

	// stacks indices
	uint32 baseIndex = 1;
	uint32 ringVertexCount = sliceCount + 1;
	for (uint32 stack = 0; stack < stackCount - 2; ++stack)
	{
		for (uint32 slice = 0; slice < sliceCount; ++slice)
		{
			mesh.indices.push_back(baseIndex + stack * ringVertexCount + slice);
			mesh.indices.push_back(baseIndex + stack * ringVertexCount + slice + 1);
			mesh.indices.push_back(baseIndex + (stack + 1)*ringVertexCount + slice);
			
			mesh.indices.push_back(baseIndex + (stack + 1)*ringVertexCount + slice);
			mesh.indices.push_back(baseIndex + stack * ringVertexCount + slice + 1);
			mesh.indices.push_back(baseIndex + (stack + 1)*ringVertexCount + slice + 1);
		}
	}

	
	// South pole vertex was added last.
	uint32 southPoleIndex = uint32(mesh.vertices.size() - 1);

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (uint32 slice = 0; slice < sliceCount; ++slice)
	{
		mesh.indices.push_back(southPoleIndex);
		mesh.indices.push_back(baseIndex + slice);
		mesh.indices.push_back(baseIndex + slice + 1);
	}

	return mesh;
}


xtest::mesh::MeshData xtest::mesh::GenerateBox(float xLength, float yLength, float zLength)
{
	MeshData mesh;
	mesh.vertices.resize(24);

	float xHalfLength = 0.5f*xLength;
	float yHalfLength = 0.5f*yLength;
	float zHalfLength = 0.5f*zLength;

	// front
	mesh.vertices[0] = { {-xHalfLength, -yHalfLength, -zHalfLength}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 1.0f} };
	mesh.vertices[1] = { {-xHalfLength, +yHalfLength, -zHalfLength}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 0.0f} };
	mesh.vertices[2] = { {+xHalfLength, +yHalfLength, -zHalfLength}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.f }, {1.0f, 0.0f} };
	mesh.vertices[3] = { {+xHalfLength, -yHalfLength, -zHalfLength}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.f }, {1.0f, 1.0f} };

	// back		   																							
	mesh.vertices[4] = { {-xHalfLength, -yHalfLength, +zHalfLength}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f, 1.f }, {1.0f, 1.0f} };
	mesh.vertices[5] = { {+xHalfLength, -yHalfLength, +zHalfLength}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 1.0f} };
	mesh.vertices[6] = { {+xHalfLength, +yHalfLength, +zHalfLength}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 0.0f} };
	mesh.vertices[7] = { {-xHalfLength, +yHalfLength, +zHalfLength}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f, 1.f }, {1.0f, 0.0f} };

	// top
	mesh.vertices[8]  = {{-xHalfLength, +yHalfLength, -zHalfLength}, {0.0f, 1.0f, 0.0f }, {1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 1.0f}};
	mesh.vertices[9]  = {{-xHalfLength, +yHalfLength, +zHalfLength}, {0.0f, 1.0f, 0.0f }, {1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 0.0f}};
	mesh.vertices[10] = {{+xHalfLength, +yHalfLength, +zHalfLength}, {0.0f, 1.0f, 0.0f }, {1.0f, 0.0f, 0.0f, 1.f }, {1.0f, 0.0f}};
	mesh.vertices[11] = {{+xHalfLength, +yHalfLength, -zHalfLength}, {0.0f, 1.0f, 0.0f }, {1.0f, 0.0f, 0.0f, 1.f }, {1.0f, 1.0f}};
						
	// bottom			
	mesh.vertices[12] = {{-xHalfLength, -yHalfLength, -zHalfLength}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f, 1.f }, {1.0f, 1.0f}};
	mesh.vertices[13] = {{+xHalfLength, -yHalfLength, -zHalfLength}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 1.0f}};
	mesh.vertices[14] = {{+xHalfLength, -yHalfLength, +zHalfLength}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f, 1.f }, {0.0f, 0.0f}};
	mesh.vertices[15] = {{-xHalfLength, -yHalfLength, +zHalfLength}, {0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f, 1.f }, {1.0f, 0.0f}};

	// left
	mesh.vertices[16] = {{-xHalfLength, -yHalfLength, +zHalfLength}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.f }, {0.0f, 1.0f}};
	mesh.vertices[17] = {{-xHalfLength, +yHalfLength, +zHalfLength}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.f }, {0.0f, 0.0f}};
	mesh.vertices[18] = {{-xHalfLength, +yHalfLength, -zHalfLength}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.f }, {1.0f, 0.0f}};
	mesh.vertices[19] = {{-xHalfLength, -yHalfLength, -zHalfLength}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f, 1.f }, {1.0f, 1.0f}};

	// right
	mesh.vertices[20] = { {+xHalfLength, -yHalfLength, -zHalfLength}, {1.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f, 1.f }, {0.0f, 1.0f} };
	mesh.vertices[21] = { {+xHalfLength, +yHalfLength, -zHalfLength}, {1.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f, 1.f }, {0.0f, 0.0f} };
	mesh.vertices[22] = { {+xHalfLength, +yHalfLength, +zHalfLength}, {1.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f, 1.f }, {1.0f, 0.0f} };
	mesh.vertices[23] = { {+xHalfLength, -yHalfLength, +zHalfLength}, {1.0f, 0.0f, 0.0f }, {0.0f, 0.0f, 1.0f, 1.f }, {1.0f, 1.0f} };



	// Create the indices
	mesh.indices.resize(36);

	// front
	mesh.indices[0] = 0; 
	mesh.indices[1] = 1; 
	mesh.indices[2] = 2;
	mesh.indices[3] = 0; 
	mesh.indices[4] = 2; 
	mesh.indices[5] = 3;

	// back
	mesh.indices[6] = 4;
	mesh.indices[7] = 5;
	mesh.indices[8] = 6;
	mesh.indices[9] = 4;
	mesh.indices[10] = 6;
	mesh.indices[11] = 7;

	// top
	mesh.indices[12] = 8;
	mesh.indices[13] = 9;
	mesh.indices[14] = 10;
	mesh.indices[15] = 8;
	mesh.indices[16] = 10; 
	mesh.indices[17] = 11;

	// bottom
	mesh.indices[18] = 12; 
	mesh.indices[19] = 13; 
	mesh.indices[20] = 14;
	mesh.indices[21] = 12; 
	mesh.indices[22] = 14;
	mesh.indices[23] = 15;

	// left
	mesh.indices[24] = 16;
	mesh.indices[25] = 17; 
	mesh.indices[26] = 18;
	mesh.indices[27] = 16;
	mesh.indices[28] = 18; 
	mesh.indices[29] = 19;

	// right
	mesh.indices[30] = 20;
	mesh.indices[31] = 21; 
	mesh.indices[32] = 22;
	mesh.indices[33] = 20; 
	mesh.indices[34] = 22; 
	mesh.indices[35] = 23;

	return mesh;
}

