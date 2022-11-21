

cc = gcc
cppfolder = cpp
exefolder = exe

define flags
	-Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wmissing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_SIDE
endef

define compile
	$(sc) $(cc) $1 $2 -o $3 $(flags)
endef

sc = @ #showcommands
default_name = prog
default_path = $(exefolder)/$(default_name)


all: dfr


dfr:
	$(call compile, $(cppfolder)/maindfr.cpp, $(cppfolder)/dfr.cpp, $(default_path))

calc:
	$(call compile, $(cppfolder)/maincalc.cpp, $(cppfolder)/calc.cpp, $(default_path))

testdfr:
	$(call compile, $(cppfolder)/testdfr.cpp, $(cppfolder)/dfr.cpp, $(default_path))

test:
	$(call compile, $(cppfolder)/test.cpp, , $(default_path))