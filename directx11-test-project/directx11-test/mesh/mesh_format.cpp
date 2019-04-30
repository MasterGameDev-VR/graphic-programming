#include "stdafx.h"
#include "mesh_format.h"

bool xtest::mesh::MeshData::Vertex::operator==(const Vertex & other) const
{
	return position.x == other.position.x
		&& position.y == other.position.y
		&& position.z == other.position.z
		&& normal.x == other.normal.x
		&& normal.y == other.normal.y
		&& normal.z == other.normal.z
		&& tangentU.x == other.tangentU.x
		&& tangentU.y == other.tangentU.y
		&& tangentU.z == other.tangentU.z
		&& uv.x == other.uv.x
		&& uv.y == other.uv.y;
}
