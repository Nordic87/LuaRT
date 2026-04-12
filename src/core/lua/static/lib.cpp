#include <luart_static.h>
#include "luart_static.cpp"

class Initializer {
	public:
		Initializer() {
#ifdef WIN32
			void *hModule = (void*)GetModuleHandle(NULL);
#else
			void *hModule = NULL;
#endif
			for(int i=0;i<sizeof(LuaFunctionNames) / sizeof(LuaFunctionNames[0]);i++)
				((void**)&staticlua)[i] = GetProcAddress((HMODULE)hModule, LuaFunctionNames[i]);
		}

		~Initializer() {
        	lua_stop();
    	}

};
		
static Initializer globalInitializer;

