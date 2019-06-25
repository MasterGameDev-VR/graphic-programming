#pragma once
#include "stdafx.h"

using namespace DirectX;
using namespace xtest;

class Quad {
public:

	struct VertexIn {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};

	VertexIn vertices[6] =
	{
		{ XMFLOAT3(-1.f, +1.f, 0.f), XMFLOAT2(+0.f, 0.f)},
		{ XMFLOAT3(+1.f, +1.f, 0.f), XMFLOAT2(+1.f, 0.f)},
		{ XMFLOAT3(-1.f, -1.f, 0.f), XMFLOAT2(+0.f, 1.f)},


		{ XMFLOAT3(-1.f, -1.f, 0.f), XMFLOAT2(+0.f, 1.f)},
		{ XMFLOAT3(+1.f, +1.f, 0.f), XMFLOAT2(+1.f, 0.f)},
		{ XMFLOAT3(+1.f, -1.f, 0.f), XMFLOAT2(+1.f, 1.f)}

	};
};