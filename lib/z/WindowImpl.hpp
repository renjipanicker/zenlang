#pragma once
#include "gui/Window.hpp"
#include "WidgetImpl.hpp"

namespace WindowImpl {

#if defined(WIN32)
inline z::widget impl(HWND hwnd) {
    void* p = (void*)::GetWindowLongPtr(hwnd, GWL_USERDATA);
    z::widget::impl* impl = reinterpret_cast<z::widget::impl*>(p);
    return z::widget(impl);
}

inline void setImpl(HWND hWnd, z::widget::impl* impl) {
    ::SetWindowLongPtr(hWnd, GWL_USERDATA, reinterpret_cast<long>(impl));
}

template<typename KeyT>
struct WidgetMap {
    typedef z::dict<KeyT, z::widget::impl*> Map;
    Map map;
    inline void add(KeyT k, z::widget::impl* v) {
        map[k] = v;
    }

    inline z::widget impl(KeyT id) {
        Map::const_iterator it = map.find(id);
        if(it == map.end()) {
            throw z::Exception("WidgetMap", z::string("Unknown ID %{i}").arg("i", id));
        }
        z::widget::impl* impl = it->second;
        return z::widget(impl);
    }
};
#endif

#if defined(WIN32)
uint32_t getNextWmID();
uint32_t getNextResID();
ULONGLONG GetDllVersion(LPCTSTR lpszDllName);

z::widget::impl& createWindow(const Window::Definition& def, const z::string& className, int style, int xstyle, HWND parent);
z::widget::impl& createMainFrame(const Window::Definition& def, int style, int xstyle);
z::widget::impl& createChildFrame(const Window::Definition& def, int style, int xstyle, const z::widget &parent);
z::widget::impl& createChildWindow(const Window::Definition& def, const z::string& className, int style, int xstyle, const z::widget& parent);
#elif defined(GTK)
z::widget::impl& initWindowImpl(GtkWidget* hwnd);
z::widget::impl& createWindow(const Window::Definition& def, GtkWidget *parent);
z::widget::impl& createChildWindow(GtkWidget* hwnd, const Window::Definition& def, const z::widget& parent);
#elif defined(OSX)
z::widget::impl& createMainFrame(const Window::Definition& def);
z::widget::impl& createChildWindow(const Window::Definition& def, const z::widget& parent, NSView* child);
#elif defined(IOS)
z::widget::impl& createMainFrame(const Window::Definition& def);
z::widget::impl& createChildWindow(const Window::Definition& def, const z::widget& parent);
#else
#endif

}
