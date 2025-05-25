#pragma once

namespace cat
{
	class DepthPrepass
	{
	public:
		// CTOR & DTOR
		//------------------------------
		DepthPrepass();
		~DepthPrepass();

		DepthPrepass(const DepthPrepass&) = delete;
		DepthPrepass& operator=(const DepthPrepass&) = delete;
		DepthPrepass(DepthPrepass&&) = delete;
		DepthPrepass& operator=(DepthPrepass&&) = delete;


		// METHODS
		//------------------------------

	private:
		// Private methods
		//------------------------------
		void CreatePipeline();



		// Private members
		//------------------------------

	};
}
