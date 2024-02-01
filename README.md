# NullEngine

## The Vision:
The goal I try to reach with this Vulkan-based Game Engine is a single library written in C for developing all kinds of Applications (currently only for Windows). It removes the Dangers of C while trying to keep the simplicity the Language brings. In the future I might provide an additional (Semi-Compiled) Language to work with the Engine.
I aim to create a framework that behaves the same across plattforms (watch out for a Linux or MacOS version), that is lightweight, (at least decently) fast, memory efficient, but doesn't dictate how you code your application.
### Don't expect anything usable in 2024
Developing an Engine is a lot of work, and I have a life besides Coding, so don't expect me to produce an Engine in 1 Year. This is also why I won't do versioning for now, as versions should all be at least somewhat usable, which is not yet the case.
### Don't do pull requests
This is **my** Project, I do this as a Hobby, please don't expect me to use your code. You can however report issues and bugs, or even request certain features
### I code by feel, not by plan
If all of this seems vague, then this is because it **is** vague. I don't know exactly where this will end up, but I hope you will have a great time looking through the code, bughunting certain versions, breaking the Engine, or even using it

---

## This is not (yet) a "Game Engine" like you may be used to
Traditional Game Engines (like [Unity](https://unity.com/de), [Godot](https://godotengine.org/), [Gamemaker Studio](https://gamemaker.io/), [Unreal Engine](https://www.unrealengine.com), ...) usually wrap around your program, call your functions, clean up after you, provide their own entry point, etc. .
NullEngine is just a library. There is (currently) no GUI, no predefined entry-point, and no config file. This might change though, so watch out for any changes!

## Code Format and conventions

### Naming
#### Functions

functions may start with certain attributes in their name. these include: 
- ***rec*** for recursive functions
- ***big*** for functions that are considerably larger than most others
- ***nst*** for functions that aren't recursive, but call a lot of nested funtions and thus impart similar dangers (e.g. stack overflows)
- ***dpc*** for outdated/deprecated functions. the comments above these functions *may* tell you the alternative to use
- ***ess*** for functions that can crash the program (i.e. 'essential' functions) (somthing like _ess_init_base_memory())

functions that start with **underscore ('_')** are intended for internal uses inside the Engine and should not be used by user applications. They are accessible through **#define _ENGINE**

function parameters also start with **underscore ('_')**. This makes it easier to detect (dangerous/unreliable) side effects.

#### Types

NullEngine uses the types defined in **'include/typedef.h'**
these are: 
- u8/u16/u32/u64 for unsigned integers
- u8/u16/u32/u64 for signed integers
- f32/f64 for floating-point numbers, as well as the standart precision ***decimal*** (which is just f32, but could change)

in the NullEngine source code, structs **must not** be typedef'd. This ensures that structs will immediately appear different to primitive types 

#### Objects
The garbage collector of **NullEngine** works by having everything wrapped in object structs. These objects are stored in a specifically for this purpose allocated object table and *may* only be acccessed through handles. accessing an object that doesn't have a handle associated with it is not possible, because the garbage collector will clean it up. Objects in essence are just sets of fields, which can take the form of a single value, an array of values or a reference to a different object.
References are useful to link other objects to your object so these referenced objects aren't accidentally picked up by the garbage collector when it runs. 
