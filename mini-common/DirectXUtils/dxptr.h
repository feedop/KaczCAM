#pragma once
#include <memory>
#include <type_traits>
#include <d3d11.h>
#include "ptr_vector.h"
#include "exceptions.h"

#define DX_ASSERT_IS_COM_TYPE(T) static_assert(std::is_base_of<IUnknown, T>::value, "T must be a COM Object!")

namespace mini::directx
{
	/**********************************************************************//*!
	 * Deleter function object class for COM pointers.
	 * @tparam T COM object type that derives from IUnknown.
	 * @remark This deleter is designed to be used with DirectX resources.
	 * It can however work with any COM objects, since that is what all DirectX
	 * resources are.
	 *************************************************************************/
	template<class T>
	class dx_deleter
	{
	public:
		DX_ASSERT_IS_COM_TYPE(T);

		/******************************************************************//*!
		 * Type of a pointer to a COM object that can be handled by this
		 * deleter class.
		 *********************************************************************/
		using pointer = std::add_pointer_t<T>;

		/******************************************************************//*!
		 * Releases the COM object.
		 * @param com_object pointer to COM object to be released.
		 * @remarks The only thing that needs to be done is to call
		 * IUnknown::Release because all COM object internally implement
		 * reference counting and delete themseleves if no longer used.
		 *********************************************************************/
		void operator () (pointer com_object) const
		{
			if (com_object)
				reinterpret_cast<IUnknown *>(com_object)->Release();
		}
	};

	/**********************************************************************//*!
	 * @brief Unique pointer to COM object.
	 *
	 * Pointer indicating the ownership of COM resource. It makes sure the
	 * resource is released correctly based on RAII principle.
	 *
	 * Copying is intentionally not supported. One can create multiple
	 * dx_ptr objects pointing to the same COM object and rely on its built-in
	 * reference counting. IUnknown::AddRef must manually be called on the COM
	 * object for each additional dx_ptr created that way, besides the first
	 * one.
	 * @tparam T COM object type that derives from IUnknown.
	 * @remark This smart pointer is designed to be used with DirectX
	 * resources. It can however work with any COM objects, since that is what
	 * all DirectX resources are.
	 *************************************************************************/
	template<class T>
	using dx_ptr = std::unique_ptr<T, dx_deleter<T>>;

	template<class T>
	dx_ptr<T> clone(dx_ptr<T> const &p)
	{
		DX_ASSERT_IS_COM_TYPE(T);
		if (p)
			reinterpret_cast<IUnknown &>(*p).AddRef();
		return dx_ptr<T>{ p.get() };
	}

	//******************* NEW *******************

	/**********************************************************************//*!
	 * @brief Vector of COM object pointers
	 *
	 * Vector owning COM resources indicated by contained pointers. It makes
	 * sure the resources are released correctly based on RAII principle.
	 *
	 * Certain D3D APIs require arrays of raw pointers as input. Since
	 * dx_ptr cannot guaratnee the pointer is it's only member, an array of
	 * dx_ptr-s will not be able to interop with such apis, requiring creation
	 * of temporary arrays. To avoid that, only one dx_deleter is stored in the
	 * vector and used to release all contained resources.
	 *
	 * @tparam T COM object type that derives from IUnknown.
	 * @remark This smart pointer is designed to be used with DirectX
	 * resources. It can however work with any COM objects, since that is what
	 * all DirectX resources are.
	 *************************************************************************/
	template<typename T>
	using dx_ptr_vector = ptr_vector<T, dx_deleter<T>>;
}
#undef DX_ASSERT_IS_COM_TYPE