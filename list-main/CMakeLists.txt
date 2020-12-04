add_library(todo_list
		include/udv/todo.hpp
		include/udv/todo_list.hpp
		include/udv/todo_item.hpp

		src/todo_list.cpp
		)

target_include_directories(
		todo_list
		PUBLIC
		include
		PRIVATE
		src
)