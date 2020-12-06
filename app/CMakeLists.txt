add_executable(main-example
		WIN32
		main.cpp
		app.hpp
		app.cpp
		)
set(WINLIBS "Pathcch.lib;runtimeobject.lib;shlwapi.lib;kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;")
target_link_libraries(main-example PUBLIC todo_list ${WINLIBS})