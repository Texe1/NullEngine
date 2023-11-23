
SRC=src
OBJ=obj

SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c,$(OBJ)\\%.obj, $(SRCS))

BIN=bin\NullEngine.lib
CL_ARGS=/c /WX /Wall /Z7
LIB_ARGS=/WX
EXTERNAL_LIBS="D:\SDKs\Vulkan\1.3.268.0\Lib\vulkan-1.lib" "D:\SDKs\Vulkan\1.3.268.0\Lib\SDL2-static.lib"

all:$(BIN)

$(BIN):$(OBJS)
	@echo compiling library ($(BIN))
	@lib $(LIB_ARGS) $(OBJS) $(EXTERNAL_LIBS) /out:$@

$(OBJ)\\%.obj: $(SRC)/%.c
	@echo compiling $^ to $@
	@cl $(CL_ARGS) $^ /Fo"$@"

clean: 
	@echo deleting Binary...
	@del $(BIN)
	@echo deleting $(OBJS)
	@del $(patsubst $(SRC)/%.obj, $(OBJ)\\%.obj, $(OBJS))
