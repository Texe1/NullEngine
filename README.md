# NullEngine

## The Vision:
The goal I try to reach with this Vulkan-based Game "Engine"(this naming might be misleading, more on that later) is a single library, that makes it easier to work with vulkan by employing sort of a "breadth-first" approach, meaning you can directly start with programming your game and worry about optimizations later, when the main game is running. This mostly means, that you don't actually have to pass any config structs, which will make the Engine use the default. 
This way it works without you having to worry about vulkan related or OS specific(BUT at the moment, there is only a Windows version) stuff, but if you'd like to optimize for your game / program later, you can very easily do so as well

---

## This is technically not (yet) an "Engine" in the traditional sense
An Engine (like Unity, Godot, Gamemaker, Unreal, ...) usually wraps around your program, calls your functions, cleans up after you, provides its own entry point, etc. .
NullEngine is just a library. There is (currently) no GUI, no predefined entry-point, no config file, but most importantly, **there are no restrictions** 
> If you want NullEngine to not do something, you can simply not call the function or change a flag inside a config struct. You are in control of what Functions are called, and which aren't. \
\
> Exceptions are: 
	>- Functions that verify parameters 
	>- Debugging utils when the _DEBUG macro is defined 
	>- Functions the vk_exit funtion calls (*BUT you could not call vk_exit(), which I **do not recommend**, because it doesn't benefit you*)