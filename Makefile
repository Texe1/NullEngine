
SRC=src
OBJ=obj


SRC_DIRS = $(wildcard $(SRC)/*/)
OBJ_DIRS = $(wildcard $(OBJ)/*/)

SRCS=$(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)*.c))
OBJS=$(patsubst $(SRC)/%.c,$(OBJ)/%.obj, $(SRCS))

BIN=bin/NullEngine.lib
CL_ARGS=/c /D_DEBUG /D_ARM64_ /D_ENGINE /W0 /Z7 /ID:\SDKs\Vulkan\1.3.268.0\Include $(addprefix /I./, $(SRC_DIRS))
LIB_ARGS=
EXTERNAL_LIBS="D:\SDKs\Vulkan\1.3.268.0\Lib\vulkan-1.lib" "D:\SDKs\Vulkan\1.3.268.0\Lib\SDL2-static.lib"

all:$(BIN)

echoDirs:
	@echo src directories: $(SRC_DIRS)
	@echo C files: $(SRCS)
	@echo object files: $(OBJS)
	@echo binary: $(BIN)
	@echo include directories: $(addprefix /I./, $(SRC_DIRS))

$(BIN): $(OBJS)
	@echo "compiling library ($(BIN))"
	lib $(LIB_ARGS) $(OBJS) $(EXTERNAL_LIBS) /out:$@

$(OBJ)/%.obj: $(SRC)/%.c
	-mkdir ./$(dir $@)
	cl $(CL_ARGS) $^ /Fo"$@"

clean: 
	@echo deleting Binary...
	@del $(BIN)
	@echo deleting $(patsubst $(OBJ)%.obj, .\\$(OBJ)%.obj, $(subst /,\\, $(OBJS)))
	@del $(patsubst $(OBJ)%.obj, .\\$(OBJ)%.obj, $(subst /,\\, $(OBJS)))