/** Templated Member Function Pointer.
	@remarks
	This code is based off CEGUI's implementation for event handlers.
	(known as subscribers)  A MemberFunctionPointer must have its
	type known during declaration.  In order to create lists of
	MemberFunctionPointers, the MemberFunctionSlot was created.
	@note
	Originally the execute function was done via overloading the () operator.
*/

#ifndef QUICKGUIMEMBERFUNCTIONPOINTER_H
#define QUICKGUIMEMBERFUNCTIONPOINTER_H

#include "OgrePrerequisites.h"
#include "QuickGUIExportDLL.h"

#include "QuickGUIPrerequisites.h"
namespace QuickGUI
{
	class _QuickGUIExport MemberFunctionSlot
	{
	public:
		virtual ~MemberFunctionSlot() {};
		virtual void execute(const EventArgs& args) = 0;
	};

	template<typename T>
	class MemberFunctionPointer :
		public MemberFunctionSlot
	{
	public:
		typedef void (T::*MemberFunction)(const EventArgs&);
	public:
		MemberFunctionPointer() :
			d_undefined(true)
		{}
		MemberFunctionPointer(MemberFunction func, T* obj) :
			d_function(func),
			d_object(obj),
			d_undefined(false)
		{}
		virtual ~MemberFunctionPointer() {}

		void execute(const EventArgs& args)
		{
			if(!d_undefined) 
				(d_object->*d_function)(args);
		}
	protected:
		MemberFunction	d_function;
		T*				d_object;
		bool			d_undefined;
	};
}

#endif
