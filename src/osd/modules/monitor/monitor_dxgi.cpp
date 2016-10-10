// license:BSD-3-Clause
// copyright-holders: Brad Hughes
/*
* monitor_dxgi.cpp
*
*/

#include "modules/osdmodule.h"
#include "monitor_module.h"

#if defined(OSD_WINDOWS)

// standard windows headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#undef interface
#undef min
#undef max

#include "strconv.h"
#include "modules/lib/osdlib.h"
#include "monitor_common.h"
#include "window.h"
#include "windows/video.h"

using namespace Microsoft::WRL;

class dxgi_monitor_info : public osd_monitor_info
{
private:
	ComPtr<IDXGIOutput>    m_output;    // The output interface

public:
	dxgi_monitor_info(monitor_module& module, HMONITOR handle, const char* monitor_device, float aspect, ComPtr<IDXGIOutput> output)
		: osd_monitor_info(module, reinterpret_cast<std::uint64_t>(handle), monitor_device, aspect),
			m_output(output)
	{
		dxgi_monitor_info::refresh();
	}

	void refresh() override
	{
		DXGI_OUTPUT_DESC desc;
		m_output->GetDesc(&desc);

		// fetch the latest info about the monitor
		m_name = osd::text::from_wstring(desc.DeviceName);

		m_pos_size = RECT_to_osd_rect(desc.DesktopCoordinates);
		m_usuable_pos_size = RECT_to_osd_rect(desc.DesktopCoordinates);
		m_is_primary = desc.AttachedToDesktop;
	}
};

typedef DWORD(WINAPI *dxgi_create_factory_fn)(REFIID, void**);

class dxgi_monitor_module : public monitor_module_base
{
private:
	osd::dynamic_module::ptr m_dxgi_module;
	dxgi_create_factory_fn   m_create_factory_fn;

public:
	dxgi_monitor_module()
		: monitor_module_base(OSD_MONITOR_PROVIDER, "dxgi"),
			m_dxgi_module(osd::dynamic_module::open({"dxgi.dll"})),
			m_create_factory_fn(nullptr)
	{
	}

	bool probe() override
	{
		m_create_factory_fn = m_dxgi_module->bind<dxgi_create_factory_fn>("CreateDXGIFactory1");
		if (m_create_factory_fn == nullptr)
			return false;

		return true;
	}

	// Currently this method implementation is duplicated from the win32 module
	// however it needs to also handle being able to do this for UWP
	std::shared_ptr<osd_monitor_info> monitor_from_rect(const osd_rect& rect) override
	{
		if (!m_initialized)
			return nullptr;

		RECT p;
		p.top = rect.top();
		p.left = rect.left();
		p.bottom = rect.bottom();
		p.right = rect.right();

		auto nearest = monitor_from_handle(reinterpret_cast<std::uintptr_t>(MonitorFromRect(&p, MONITOR_DEFAULTTONEAREST)));
		assert(nearest != nullptr);
		return nearest;
	}

	// Currently this method implementation is duplicated from the win32 module
	// however it needs to also handle being able to do this for UWP
	std::shared_ptr<osd_monitor_info> monitor_from_window(const osd_window& window) override
	{
		if (!m_initialized)
			return nullptr;

		auto nearest = monitor_from_handle(reinterpret_cast<std::uintptr_t>(MonitorFromWindow(window.platform_window<HWND>(), MONITOR_DEFAULTTONEAREST)));
		assert(nearest != nullptr);
		return nearest;
	}

protected:
	int init_internal(const osd_options& options) override
	{
		HRESULT result;
		ComPtr<IDXGIDevice> dxgiDevice;
		ComPtr<IDXGIFactory2> factory;
		ComPtr<IDXGIAdapter> adapter;

		result = m_create_factory_fn(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(factory.GetAddressOf()));
		if (result != ERROR_SUCCESS)
		{
			osd_printf_error("CreateDXGIFactory1 failed with error 0x%x\n", static_cast<unsigned int>(result));
			return 1;
		}

		UINT iAdapter = 0;
		while (!factory->EnumAdapters(iAdapter, adapter.ReleaseAndGetAddressOf()))
		{
			UINT i = 0;
			ComPtr<IDXGIOutput> output;
			DXGI_OUTPUT_DESC desc;
			while (!adapter->EnumOutputs(i, output.GetAddressOf()))
			{
				output->GetDesc(&desc);

				// guess the aspect ratio assuming square pixels
				RECT * coords = &desc.DesktopCoordinates;
				float aspect = float(coords->right - coords->left) / float(coords->bottom - coords->top);

				// allocate a new monitor info
				std::string devicename = osd::text::from_wstring(desc.DeviceName);

				// allocate a new monitor info
				auto monitor = std::make_shared<dxgi_monitor_info>(*this, desc.Monitor, devicename.c_str(), aspect, output);

				// hook us into the list
				add_monitor(monitor);

				i++;
			}

			iAdapter++;
		}

		// if we're verbose, print the list of monitors
		{
			for (auto monitor : list())
			{
				osd_printf_verbose("Video: Monitor %I64u = \"%s\" %s\n", monitor->oshandle(), monitor->devicename().c_str(), monitor->is_primary() ? "(primary)" : "");
			}
		}

		return 0;
	}
};

#else
MODULE_NOT_SUPPORTED(dxgi_monitor_module, OSD_MONITOR_PROVIDER, "dxgi")
#endif

MODULE_DEFINITION(MONITOR_DXGI, dxgi_monitor_module)
