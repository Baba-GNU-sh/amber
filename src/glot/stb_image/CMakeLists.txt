add_library(
	stb_image
	STATIC
		stb_impl.cpp
)

target_include_directories(
	stb_image
	PRIVATE
		include/stb_image
	PUBLIC
		include
)
