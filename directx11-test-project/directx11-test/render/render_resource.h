#pragma once

namespace xtest {
namespace render {


	// base class for every render resource
	// warning: every render resource must be initialized via Init() before used, Bind() should be used to bind the resource to the pipeline
	class RenderResource
	{
	public:

		virtual void Init() = 0;
		virtual void Bind() = 0;
		
	};

} //render
} //xtest
