
SRC=src
OBJ=obj


SRC_DIRS = $(wildcard $(SRC)/*/)

SRCS=$(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)*.c))
OBJS=$(patsubst $(SRC)/%.c,$(OBJ)/%.obj, $(SRCS))

BIN=bin\NullEngine.lib
CL_ARGS=/c /W0 /Z7 /ID:\SDKs\Vulkan\1.3.268.0\Include
LIB_ARGS=
EXTERNAL_LIBS="D:\SDKs\Vulkan\1.3.268.0\Lib\vulkan-1.lib" "D:\SDKs\Vulkan\1.3.268.0\Lib\SDL2-static.lib"

all:$(BIN)

$(BIN): $(OBJS)
	@echo src directories: $(SRC_DIRS)
	@echo C files: $(SRCS)
	@echo object files: $(OBJS)
	@echo compiling library ($(BIN))
	@lib $(LIB_ARGS) $(OBJS) $(EXTERNAL_LIBS) /out:$@

$(OBJ)/%.obj: $(SRC)/%.c
	@echo compiling $^ to $@
	if not exist "./obj/vulkan/" { md $(subst /,\\,./$(dir $@)) }
	@cl $(CL_ARGS) $^ /Fo"$@"

clean: 
	@echo deleting Binary...
	@del $(BIN)
	@echo deleting $(OBJS)
	@del $(patsubst $(SRC)/%.obj, $(OBJ)\\%.obj, $(OBJS))
