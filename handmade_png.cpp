/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */
#include <windows.h>
#include <math.h>

#include "handmade_types.h"

#pragma pack(push, 1)
struct png_header
{
    u8 Signature[8];
};
global u8 PNGSignature[] = {137, 80, 78, 71, 13, 10, 26, 10};

struct png_chunk_header
{
    u32 Length;
    union
    {
        u32 TypeU32;
        char Type[4];
    };
};

struct png_chunk_footer
{
    u32 CRC;
};

struct png_ihdr
{
    u32 Width;
    u32 Height;
    u8 BitDepth;
    u8 ColorType;
    u8 CompressionMethod;
    u8 FilterMethod;
    u8 InterlaceMethod;
};

struct png_idat_header
{
    u8 ZLibMethodFlags;
    u8 AdditionalFlags;
};

struct png_idat_footer
{
    u32 CheckValue;
};

struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
};

#pragma pack(pop)

struct png_huffman_entry
{
    u16 Symbol;
    u16 BitsUsed;
};

#define PNG_HUFFMAN_MAX_BIT_COUNT 16
struct png_huffman
{
    u32 MaxCodeLengthInBits;
    u32 EntryCount;
    png_huffman_entry *Entries;
};

struct image_u32
{
    u32 Width;
    u32 Height;
    u32 *Pixels;
};

struct mip_iterator
{
    u32 Level;
    image_u32 Image;
};

#define counted_pointer(IGNORED)

/*
  NOTE(casey):

  HANDMADE_INTERNAL:
    0 - Build for public release
    1 - Build for developer only

  HANDMADE_SLOW:
    0 - Not slow code allowed!
    1 - Slow code welcome.
*/

#pragma pack(push, 1)
    struct bitmap_id
    {
        u32 Value;
    };

    struct sound_id
    {
        u32 Value;
    };

    struct font_id
    {
        u32 Value;
    };
#pragma pack(pop)

    /*
      NOTE(casey): Services that the platform layer provides to the game
    */
#if HANDMADE_INTERNAL
    /* IMPORTANT(casey):

       These are NOT for doing anything in the shipping game - they are
       blocking and the write doesn't protect against lost data!
    */
    typedef struct debug_read_file_result
    {
        uint32 ContentsSize;
        void *Contents;
    } debug_read_file_result;

    typedef struct debug_executing_process
    {
        u64 OSHandle;
    } debug_executing_process;

    typedef struct debug_process_state
    {
        b32 StartedSuccessfully;
        b32 IsRunning;
        s32 ReturnCode;
    } debug_process_state;

    typedef struct debug_platform_memory_stats
    {
        umm BlockCount;
        umm TotalSize; // NOTE(casey): Does not technically include the header
        umm TotalUsed;
    } debug_platform_memory_stats;

#define DEBUG_PLATFORM_EXECUTE_SYSTEM_COMMAND(name) debug_executing_process name(char *Path, char *Command, char *CommandLine)
    typedef DEBUG_PLATFORM_EXECUTE_SYSTEM_COMMAND(debug_platform_execute_system_command);

#define DEBUG_PLATFORM_GET_MEMORY_STATS(name) debug_platform_memory_stats name(void)
    typedef DEBUG_PLATFORM_GET_MEMORY_STATS(debug_platform_get_memory_stats);

    // TODO(casey): Do we want a formal release mechanism here?
#define DEBUG_PLATFORM_GET_PROCESS_STATE(name) debug_process_state name(debug_executing_process Process)
    typedef DEBUG_PLATFORM_GET_PROCESS_STATE(debug_platform_get_process_state);

    // TODO(casey): Actually start using this???
    extern struct game_memory *DebugGlobalMemory;

#endif

    // TODO(casey): Not really sure exactly where these will eventually live.
    // They might want to be dynamic, depending on the graphics memory?
#define HANDMADE_NORMAL_TEXTURE_COUNT 256
#define HANDMADE_SPECIAL_TEXTURE_COUNT 16
#define HANDMADE_TEXTURE_TRANSFER_BUFFER_SIZE (128*1024*1024)

    /*
      NOTE(casey): Services that the game provides to the platform layer.
      (this may expand in the future - sound on separate thread, etc.)
    */

    // FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use

    struct manual_sort_key
    {
        u16 AlwaysInFrontOf;
        u16 AlwaysBehind;
    };

#define BITMAP_BYTES_PER_PIXEL 4
    typedef struct game_offscreen_buffer
    {
        // NOTE(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
        void *Memory;
        int Width;
        int Height;
        int Pitch;
    } game_offscreen_buffer;

    typedef struct game_sound_output_buffer
    {
        int SamplesPerSecond;
        int SampleCount;

        // IMPORTANT(casey): Samples must be padded to a multiple of 4 samples!
        int16 *Samples;
    } game_sound_output_buffer;

    typedef struct game_button_state
    {
        int HalfTransitionCount;
        bool32 EndedDown;
    } game_button_state;

    typedef struct game_controller_input
    {
        b32 IsConnected;
        b32 IsAnalog;
        f32 StickAverageX;
        f32 StickAverageY;
        f32 ClutchMax; // NOTE(casey): This is the "dodge" clutch, eg. triggers or space bar?

        union
        {
            game_button_state Buttons[12];
            struct
            {
                game_button_state MoveUp;
                game_button_state MoveDown;
                game_button_state MoveLeft;
                game_button_state MoveRight;

                game_button_state ActionUp;
                game_button_state ActionDown;
                game_button_state ActionLeft;
                game_button_state ActionRight;

                game_button_state LeftShoulder;
                game_button_state RightShoulder;

                game_button_state Back;
                game_button_state Start;

                // NOTE(casey): All buttons must be added above this line

                game_button_state Terminator;
            };
        };
    } game_controller_input;

#define MAX_CONTROLLER_COUNT 5
    enum game_input_mouse_button
    {
        PlatformMouseButton_Left,
        PlatformMouseButton_Middle,
        PlatformMouseButton_Right,
        PlatformMouseButton_Extended0,
        PlatformMouseButton_Extended1,

        PlatformMouseButton_Count,
    };
    typedef struct game_input
    {
        r32 dtForFrame;
        u32 Entropy;

        game_controller_input Controllers[MAX_CONTROLLER_COUNT];

        // NOTE(casey): Signals back to the platform layer
        b32 QuitRequested;
        b32 EntropyRequested;

        // NOTE(casey): For debugging only
        game_button_state MouseButtons[PlatformMouseButton_Count];
        v3 ClipSpaceMouseP;
        b32 ShiftDown, AltDown, ControlDown;
        b32 FKeyPressed[13]; // NOTE(casey): 1 is F1, etc., for clarity - 0 is not used!
    } game_input;

    inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
    {
        Assert(ControllerIndex < ArrayCount(Input->Controllers));

        game_controller_input *Result = &Input->Controllers[ControllerIndex];
        return(Result);
    }

    inline b32 WasPressed(game_button_state State)
    {
        b32 Result = ((State.HalfTransitionCount > 1) ||
                      ((State.HalfTransitionCount == 1) && (State.EndedDown)));

        return(Result);
    }

    inline b32 IsDown(game_button_state State)
    {
        b32 Result = (State.EndedDown);

        return(Result);
    }

    typedef struct platform_file_handle
    {
        b32 NoErrors;
        void *Platform;
    } platform_file_handle;

    typedef struct platform_file_info
    {
        platform_file_info *Next;
        u64 FileDate; // NOTE(casey): This is a 64-bit number that _means_ the date to the platform, but doesn't have to be understood by the app as any particular date.
        u64 FileSize;
        char *BaseName; // NOTE(casey): Doesn't include a path or an extension
        void *Platform;
    } platform_file_info;
    typedef struct platform_file_group
    {
        u32 FileCount;
        platform_file_info *FirstFileInfo;
        void *Platform;
    } platform_file_group;

    typedef enum platform_file_type
    {
        PlatformFileType_AssetFile,
        PlatformFileType_SavedGameFile,
        PlatformFileType_HHT,
        PlatformFileType_Dump,

        PlatformFileType_Count,
    } platform_file_type;

    enum platform_memory_block_flags
    {
        PlatformMemory_NotRestored = 0x1,
        PlatformMemory_OverflowCheck = 0x2,
        PlatformMemory_UnderflowCheck = 0x4,
    };
    struct platform_memory_block
    {
        u64 Flags;
        u64 Size;
        u8 *Base;
        umm Used;
        platform_memory_block *ArenaPrev;
    };

#define PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(name) platform_file_group name(platform_file_type Type)
    typedef PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(platform_get_all_files_of_type_begin);

#define PLATFORM_GET_ALL_FILE_OF_TYPE_END(name) void name(platform_file_group *FileGroup)
    typedef PLATFORM_GET_ALL_FILE_OF_TYPE_END(platform_get_all_files_of_type_end);

    // TODO(casey): Add a way to replace a file, or set the size of a file?
    enum platform_open_file_mode_flags
    {
        OpenFile_Read = 0x1,
        OpenFile_Write = 0x2,
    };
#define PLATFORM_OPEN_FILE(name) platform_file_handle name(platform_file_group *FileGroup, platform_file_info *Info, u32 ModeFlags)
    typedef PLATFORM_OPEN_FILE(platform_open_file);

#define PLATFORM_SET_FILE_SIZE(name) void name(platform_file_handle *Handle, u64 Size)
    typedef PLATFORM_SET_FILE_SIZE(platform_set_file_size);

#define PLATFORM_GET_FILE_BY_PATH(name) platform_file_info *name(platform_file_group *FileGroup, char *Path, u32 ModeFlags)
    typedef PLATFORM_GET_FILE_BY_PATH(platform_get_file_by_path);

#define PLATFORM_READ_DATA_FROM_FILE(name) void name(platform_file_handle *Handle, u64 Offset, u64 Size, void *Dest)
    typedef PLATFORM_READ_DATA_FROM_FILE(platform_read_data_from_file);

#define PLATFORM_WRITE_DATA_TO_FILE(name) void name(platform_file_handle *Handle, u64 Offset, u64 Size, void *Source)
    typedef PLATFORM_WRITE_DATA_TO_FILE(platform_write_data_to_file);

#define PLATFORM_ATOMIC_REPLACE_FILE_CONTENTS(name) b32 name(platform_file_info *Info, u64 Size, void *Source)
    typedef PLATFORM_ATOMIC_REPLACE_FILE_CONTENTS(platform_atomic_replace_file_contents);

#define PLATFORM_FILE_ERROR(name) void name(platform_file_handle *Handle, char *Message)
    typedef PLATFORM_FILE_ERROR(platform_file_error);

#define PLATFORM_CLOSE_FILE(name) void name(platform_file_handle *Handle)
    typedef PLATFORM_CLOSE_FILE(platform_close_file);

#define PlatformNoFileErrors(Handle) ((Handle)->NoErrors)

    struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_work_queue *Queue, void *Data)
    typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

#define PLATFORM_ALLOCATE_MEMORY(name) platform_memory_block *name(memory_index Size, u64 Flags)
    typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);

#define PLATFORM_DEALLOCATE_MEMORY(name) void name(platform_memory_block *Block)
    typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);

    typedef void platform_add_entry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data);
    typedef void platform_complete_all_work(platform_work_queue *Queue);


    enum platform_error_type
    {
        PlatformError_Fatal,
        PlatformError_Nonfatal,
    };
#define PLATFORM_ERROR_MESSAGE(name) void name(platform_error_type Type, char *Message)
    typedef PLATFORM_ERROR_MESSAGE(platform_error_message);

    typedef struct platform_api
    {
        platform_add_entry *AddEntry;
        platform_complete_all_work *CompleteAllWork;

        platform_get_all_files_of_type_begin *GetAllFilesOfTypeBegin;
        platform_get_all_files_of_type_end *GetAllFilesOfTypeEnd;
        platform_get_file_by_path *GetFileByPath;
        platform_open_file *OpenFile;
        platform_set_file_size *SetFileSize;
        platform_read_data_from_file *ReadDataFromFile;
        platform_write_data_to_file *WriteDataToFile;
        platform_atomic_replace_file_contents *AtomicReplaceFileContents;
        platform_file_error *FileError;
        platform_close_file *CloseFile;

        platform_allocate_memory *AllocateMemory;
        platform_deallocate_memory *DeallocateMemory;

        platform_error_message *ErrorMessage;

#if HANDMADE_INTERNAL
        debug_platform_execute_system_command *DEBUGExecuteSystemCommand;
        debug_platform_get_process_state *DEBUGGetProcessState;
        debug_platform_get_memory_stats *DEBUGGetMemoryStats;
#endif

    } platform_api;
    platform_api Platform;

    typedef struct game_memory
    {
        struct game_state *GameState;

#if HANDMADE_INTERNAL
        struct debug_table *DebugTable;
        struct debug_state *DebugState;
#endif

        platform_work_queue *HighPriorityQueue;
        platform_work_queue *LowPriorityQueue;
        struct renderer_texture_queue *TextureQueue;

        b32 ExecutableReloaded;
        platform_api PlatformAPI;
    } game_memory;

    struct game_render_commands;
    struct game_render_settings;
#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_input *Input, game_render_commands *RenderCommands)
    typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

    // NOTE(casey): At the moment, this has to be a very fast function, it cannot be
    // more than a millisecond or so.
    // TODO(casey): Reduce the pressure on this function's performance by measuring it
    // or asking about it, etc.
#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory *Memory, game_sound_output_buffer *SoundBuffer)
    typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

struct memory_arena
{
    // TODO(casey): If we see perf problems here, maybe move Used/Base/Size out?
    platform_memory_block *CurrentBlock;
    umm MinimumBlockSize;

    u64 AllocationFlags;
    s32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    platform_memory_block *Block;
    umm Used;
};

inline void
SetMinimumBlockSize(memory_arena *Arena, memory_index MinimumBlockSize)
{
    Arena->MinimumBlockSize = MinimumBlockSize;
}

inline memory_index
GetAlignmentOffset(memory_arena *Arena, memory_index Alignment)
{
    memory_index AlignmentOffset = 0;

    memory_index ResultPointer = (memory_index)Arena->CurrentBlock->Base + Arena->CurrentBlock->Used;
    memory_index AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }

    return(AlignmentOffset);
}

enum arena_push_flag
{
    ArenaFlag_ClearToZero = 0x1,
};
struct arena_push_params
{
    u32 Flags;
    u32 Alignment;
};

inline arena_push_params
DefaultArenaParams(void)
{
    arena_push_params Params;
    Params.Flags = ArenaFlag_ClearToZero;
    Params.Alignment = 4;
    return(Params);
}

inline arena_push_params
AlignNoClear(u32 Alignment)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    Params.Alignment = Alignment;
    return(Params);
}

inline arena_push_params
Align(u32 Alignment, b32 Clear)
{
    arena_push_params Params = DefaultArenaParams();
    if(Clear)
    {
        Params.Flags |= ArenaFlag_ClearToZero;
    }
    else
    {
        Params.Flags &= ~ArenaFlag_ClearToZero;
    }
    Params.Alignment = Alignment;
    return(Params);
}

inline arena_push_params
NoClear(void)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    return(Params);
}

struct arena_bootstrap_params
{
    u64 AllocationFlags;
    umm MinimumBlockSize;
};

inline arena_bootstrap_params
DefaultBootstrapParams(void)
{
    arena_bootstrap_params Params = {};
    return(Params);
}

inline arena_bootstrap_params
NonRestoredArena(void)
{
    arena_bootstrap_params Params = DefaultBootstrapParams();
    Params.AllocationFlags = PlatformMemory_NotRestored;
    return(Params);
}

#if HANDMADE_INTERNAL
#define DEBUG_MEMORY_NAME(Name) DEBUG_NAME_(__FILE__, __LINE__, __COUNTER__),
#define INTERNAL_MEMORY_PARAM char *GUID,
#define INTERNAL_MEMORY_PASS GUID,
#else
#define DEBUG_MEMORY_NAME(Name)
#define INTERNAL_MEMORY_PARAM
#define INTERNAL_MEMORY_PASS
#endif

#define PushStruct(Arena, type, ...) (type *)PushSize_(DEBUG_MEMORY_NAME("PushStruct") Arena, sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(DEBUG_MEMORY_NAME("PushArray") Arena, (Count)*sizeof(type), ## __VA_ARGS__)
#define PushSize(Arena, Size, ...) PushSize_(DEBUG_MEMORY_NAME("PushSize") Arena, Size, ## __VA_ARGS__)
#define PushCopy(...) PushCopy_(DEBUG_MEMORY_NAME("PushCopy") __VA_ARGS__)
#define PushStringZ(...) PushStringZ_(DEBUG_MEMORY_NAME("PushStringZ") __VA_ARGS__)
#define PushString(...) PushString_(DEBUG_MEMORY_NAME("PushString") __VA_ARGS__)
#define PushBuffer(...) PushBuffer_(DEBUG_MEMORY_NAME("PushBuffer") __VA_ARGS__)
#define PushAndNullTerminate(...) PushAndNullTerminate_(DEBUG_MEMORY_NAME("PushAndNullTerminate") __VA_ARGS__)
#define BootstrapPushStruct(type, Member, ...) (type *)BootstrapPushSize_(DEBUG_MEMORY_NAME("BootstrapPushSize") sizeof(type), OffsetOf(type, Member), ## __VA_ARGS__)

inline memory_index
GetEffectiveSizeFor(memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    memory_index Size = SizeInit;

    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    Size += AlignmentOffset;

    return(Size);
}

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

inline int32
SignOf(int32 Value)
{
    int32 Result = (Value >= 0) ? 1 : -1;
    return(Result);
}

enum win32_memory_block_flag
{
    Win32Mem_AllocatedDuringLooping = 0x1,
    Win32Mem_FreedDuringLooping = 0x2,
};
struct win32_memory_block
{
    platform_memory_block Block;
    win32_memory_block *Prev;
    win32_memory_block *Next;
    u64 LoopingFlags;
};

struct win32_saved_memory_block
{
    u64 BasePointer;
    u64 Size;
};

inline f32
SignOf(f32 Value)
{
    // TODO(casey): Look at octahedral code and figure out
    // if we're actually using SignOf for SignOrZero in places?
    // Because I don't see a SignOrZero and I feel like there
    // should be one!

    u32 MaskU32 = (u32)(1 << 31);
    __m128 Mask = _mm_set_ss(*(float *)&MaskU32);

    __m128 One = _mm_set_ss(1.0f);
    __m128 SignBit = _mm_and_ps(_mm_set_ss(Value), Mask);
    __m128 Combined = _mm_or_ps(One, SignBit);

    f32 Result = _mm_cvtss_f32(Combined);

    return(Result);
}

inline f32
SquareRoot(f32 Real32)
{
    f32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(Real32)));
    return(Result);
}

inline f32
ReciprocalSquareRoot(f32 Real32)
{
    f32 Result = (1.0f / SquareRoot(Real32));
    return(Result);
}

inline real32
AbsoluteValue(real32 Real32)
{
    real32 Result = fabsf(Real32);
    return(Result);
}

inline uint32
RotateLeft(uint32 Value, int32 Amount)
{
#if COMPILER_MSVC
    uint32 Result = _rotl(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif

    return(Result);
}

inline u64
RotateLeft(u64 Value, s32 Amount)
{
#if COMPILER_MSVC
    u64 Result = _rotl64(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 63;
    u64 Result = ((Value << Amount) | (Value >> (64 - Amount)));
#endif

    return(Result);
}

inline uint32
RotateRight(uint32 Value, int32 Amount)
{
#if COMPILER_MSVC
    uint32 Result = _rotr(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif

    return(Result);
}

inline s32
RoundReal32ToInt32(f32 Real32)
{
    s32 Result = _mm_cvtss_si32(_mm_set_ss(Real32));
    return(Result);
}

inline u32
RoundReal32ToUInt32(real32 Real32)
{
    u32 Result = (u32)_mm_cvtss_si32(_mm_set_ss(Real32));
    return(Result);
}

inline int32
FloorReal32ToInt32(real32 Real32)
{
    // TODO(casey): Do we want to forgo the use of SSE 4.1?
    int32 Result = _mm_cvtss_si32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(Real32)));
    return(Result);
}

inline f32
Round(f32 Real32)
{
    f32 Result = _mm_cvtss_f32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(Real32),
                               (_MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC)));
    return(Result);
}

inline f32
Floor(f32 Real32)
{
    // TODO(casey): Do we want to forgo the use of SSE 4.1?
    f32 Result = _mm_cvtss_f32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(Real32)));
    return(Result);
}

inline int32
CeilReal32ToInt32(real32 Real32)
{
    // TODO(casey): Do we want to forgo the use of SSE 4.1?
    int32 Result = _mm_cvtss_si32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(Real32)));
    return(Result);
}

inline u32
Hash32(u64 A, u64 B = 0)
{
    u8 Seed[16] =
    {
        0xaa, 0x9b, 0xbd, 0xb8,
        0xa1, 0x98, 0xac, 0x3f,
        0x1f, 0x94, 0x07, 0xb3,
        0x8c, 0x27, 0x93, 0x69,
    };
    __m128i Hash = _mm_set_epi64x(A, B);
    Hash = _mm_aesdec_si128(Hash, _mm_loadu_si128((__m128i *)Seed));
    Hash = _mm_aesdec_si128(Hash, _mm_loadu_si128((__m128i *)Seed));
    u32 Result = _mm_extract_epi32(Hash, 0);
    return(Result);
}

inline int32
TruncateReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)Real32;
    return(Result);
}

inline real32
Sin(real32 Angle)
{
    real32 Result = sinf(Angle);
    return(Result);
}

inline real32
Cos(real32 Angle)
{
    real32 Result = cosf(Angle);
    return(Result);
}

inline real32
ATan2(real32 Y, real32 X)
{
    real32 Result = atan2f(Y, X);
    return(Result);
}

struct bit_scan_result
{
    bool32 Found;
    uint32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(uint32 Value)
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(s32 Test = 0;
        Test < 32;
        ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif

    return(Result);
}

inline bit_scan_result
FindMostSignificantSetBit(uint32 Value)
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found = _BitScanReverse((unsigned long *)&Result.Index, Value);
#else
    for(s32 Test = 32;
        Test > 0;
        --Test)
    {
        if(Value & (1 << (Test - 1)))
        {
            Result.Index = Test - 1;
            Result.Found = true;
            break;
        }
    }
#endif

    return(Result);
}

/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include <stdarg.h>

struct adler_32
{
    u32 S1;
    u32 S2;
};

#define ConstZ(Z) {sizeof(Z) - 1, (u8 *)(Z)}
#define BundleZ(Z) BundleString(sizeof(Z) - 1, (Z))

#define CopyArray(Count, Source, Dest) Copy((Count)*sizeof(*(Source)), (Source), (Dest))
internal void *
Copy(memory_index Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--) {*Dest++ = *Source++;}

    return(DestInit);
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
#define ZeroArray(Count, Pointer) ZeroSize((Count)*sizeof((Pointer)[0]), Pointer)
internal void
ZeroSize(memory_index Size, void *Ptr)
{
    uint8 *Byte = (uint8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

internal b32x
IsValid(buffer Buffer)
{
    b32x Result = (Buffer.Count > 0);
    return(Result);
}

internal string
WrapZ(char *Z)
{
    string Result;

    Result.Count = StringLength(Z);
    Result.Data = (u8 *)Z;

    return(Result);
}

internal string
BundleString(umm Count, char *Z)
{
    string Result;

    Result.Count = Count;
    Result.Data = (u8 *)Z;

    return(Result);
}

internal string
RemoveExtension(string FileNameInit)
{
    string FileName = FileNameInit;

    umm NewCount = FileName.Count;
    for(umm Index = 0;
        Index < FileName.Count;
        ++Index)
    {
        char C = FileName.Data[Index];
        if(C == '.')
        {
            NewCount = Index;
        }
        else if((C == '/') || (C == '\\'))
        {
            NewCount = FileName.Count;
        }
    }

    FileName.Count = NewCount;

    return(FileName);
}

internal string
RemovePath(string FileNameInit)
{
    string FileName = FileNameInit;

    umm NewStart = 0;
    for(umm Index = 0;
        Index < FileName.Count;
        ++Index)
    {
        char C = FileName.Data[Index];
        if((C == '/') || (C == '\\'))
        {
            NewStart = Index + 1;
        }
    }

    FileName.Data += NewStart;
    FileName.Count -= NewStart;

    return(FileName);
}

internal u8 *
Advance(buffer *Buffer, umm Count)
{
    u8 *Result = 0;

    if(Buffer->Count >= Count)
    {
        Result = Buffer->Data;
        Buffer->Data += Count;
        Buffer->Count -= Count;
    }
    else
    {
        Buffer->Data += Buffer->Count;
        Buffer->Count = 0;
    }

    return(Result);
}

internal char
ToLowercase(char Char)
{
    char Result = Char;

    if((Result >= 'A') && (Result <= 'Z'))
    {
        Result += 'a' - 'A';
    }

    return(Result);
}

internal void
UpdateStringHash(u32 *HashValue, char Value)
{
    // TODO(casey): Better hash function
    *HashValue = 65599*(*HashValue) + Value;
}

internal u32
StringHashOf(char *Z)
{
    u32 HashValue = 0;

    while(*Z)
    {
        UpdateStringHash(&HashValue, *Z++);
    }

    return(HashValue);
}

internal u32
StringHashOf(string S)
{
    u32 HashValue = 0;

    for(umm Index = 0;
        Index < S.Count;
        ++Index)
    {
        UpdateStringHash(&HashValue, S.Data[Index]);
    }

    return(HashValue);
}

internal adler_32
BeginAdler32(void)
{
    adler_32 Result = {};
    Result.S1 = 1;
    Result.S2 = 0;
    return(Result);
}

internal void
Adler32Append(adler_32 *Adler, umm Size, void *Data)
{
    u32 S1 = Adler->S1;
    u32 S2 = Adler->S2;

    for(umm Index = 0; Index < Size; ++Index)
    {
        S1 = (S1 + ((u8 *)Data)[Index]) % 65521;
        S2 = (S2 + S1) % 65521;
    }

    Adler->S1 = S1;
    Adler->S2 = S2;
}

internal u32
EndAdler32(adler_32 *Adler)
{
    u32 Result = (Adler->S2*65536 + Adler->S1);
    return(Result);
}

internal b32
IsEndOfLine(char C)
{
    b32 Result = ((C == '\n') ||
                  (C == '\r'));

    return(Result);
}

internal b32
IsSpacing(char C)
{
    b32 Result = ((C == ' ') ||
                  (C == '\t') ||
                  (C == '\v') ||
                  (C == '\f'));

    return(Result);
}

internal b32
IsWhitespace(char C)
{
    b32 Result = (IsSpacing(C) || IsEndOfLine(C));

    return(Result);
}

internal b32x
IsAlpha(char C)
{
    b32x Result = (((C >= 'a') && (C <= 'z')) ||
                   ((C >= 'A') && (C <= 'Z')));

    return(Result);
}

internal b32x
IsNumber(char C)
{
    b32x Result = ((C >= '0') && (C <= '9'));

    return(Result);
}

internal b32
IsHex(char Char)
{
    b32 Result = (((Char >= '0') && (Char <= '9')) ||
                  ((Char >= 'A') && (Char <= 'F')) ||
                  ((Char >= 'a') && (Char <= 'f')));

    return(Result);
}

internal u32
GetHex(char Char)
{
    u32 Result = 0;

    if((Char >= '0') && (Char <= '9'))
    {
        Result = Char - '0';
    }
    else if((Char >= 'A') && (Char <= 'F'))
    {
        Result = 0xA + (Char - 'A');
    }
    else if((Char >= 'a') && (Char <= 'f'))
    {
        Result = 0xA + (Char - 'a');
    }

    return(Result);
}

internal b32x
MemoryIsEqual(umm Count, void *AInit, void *BInit)
{
    u8 *A = (u8 *)AInit;
    u8 *B = (u8 *)BInit;
    while(Count--)
    {
        if(*A++ != *B++)
        {
            return(false);
        }
    }

    return(true);
}

internal b32x
MemoryIsEqual(buffer FileBuffer, buffer HHTContents)
{
    b32x Result = ((FileBuffer.Count == HHTContents.Count) &&
                   MemoryIsEqual(FileBuffer.Count, FileBuffer.Data, HHTContents.Data));
    return(Result);
}

internal b32
StringsAreEqual(char *A, char *B)
{
    b32 Result = (A == B);

    if(A && B)
    {
        while(*A && *B && (*A == *B))
        {
            ++A;
            ++B;
        }

        Result = ((*A == 0) && (*B == 0));
    }

    return(Result);
}

internal b32
StringsAreEqual(umm ALength, char *A, char *B)
{
    b32 Result = false;

    if(B)
    {
        char *At = B;
        for(umm Index = 0;
            Index < ALength;
            ++Index, ++At)
        {
            if((*At == 0) ||
               (A[Index] != *At))
            {
                return(false);
            }
        }

        Result = (*At == 0);
    }
    else
    {
        Result = (ALength == 0);
    }

    return(Result);
}

internal b32
StringsAreEqual(memory_index ALength, char *A, memory_index BLength, char *B)
{
    b32 Result = (ALength == BLength);

    if(Result)
    {
        Result = true;
        for(u32 Index = 0;
            Index < ALength;
            ++Index)
        {
            if(A[Index] != B[Index])
            {
                Result = false;
                break;
            }
        }
    }

    return(Result);
}

internal b32x
StringsAreEqual(string A, char *B)
{
    b32x Result = StringsAreEqual(A.Count, (char *)A.Data, B);
    return(Result);
}

internal b32x
StringsAreEqual(string A, string B)
{
    b32x Result = StringsAreEqual(A.Count, (char *)A.Data, B.Count, (char *)B.Data);
    return(Result);
}

internal b32
StringsAreEqualLowercase(memory_index ALength, char *A, memory_index BLength, char *B)
{
    b32 Result = (ALength == BLength);

    if(Result)
    {
        Result = true;
        for(u32 Index = 0;
            Index < ALength;
            ++Index)
        {
            if(ToLowercase(A[Index]) != ToLowercase(B[Index]))
            {
                Result = false;
                break;
            }
        }
    }

    return(Result);
}

internal b32x
StringsAreEqualLowercase(string A, string B)
{
    b32x Result = StringsAreEqualLowercase(A.Count, (char *)A.Data, B.Count, (char *)B.Data);
    return(Result);
}

internal s32
S32FromZInternal(char **AtInit)
{
    s32 Result = 0;

    char *At = *AtInit;
    while((*At >= '0') &&
          (*At <= '9'))
    {
        Result *= 10;
        Result += (*At - '0');
        ++At;
    }

    *AtInit = At;

    return(Result);
}

internal s32
S32FromZ(char *At)
{
    char *Ignored = At;
    s32 Result = S32FromZInternal(&Ignored);
    return(Result);
}

struct format_dest
{
    umm Size;
    char *At;
};

internal void
OutChar(format_dest *Dest, char Value)
{
    if(Dest->Size)
    {
        --Dest->Size;
        *Dest->At++ = Value;
    }
}

internal void
OutChars(format_dest *Dest, char *Value)
{
    // NOTE(casey): Not particularly speedy, are we?  :P
    while(*Value)
    {
        OutChar(Dest, *Value++);
    }
}

#define ReadVarArgUnsignedInteger(Length, ArgList) ((Length) == 8) ? va_arg(ArgList, u64) : (u64)va_arg(ArgList, u32)
#define ReadVarArgSignedInteger(Length, ArgList) ((Length) == 8) ? va_arg(ArgList, s64) : (s64)va_arg(ArgList, s32)
#define ReadVarArgFloat(Length, ArgList) va_arg(ArgList, f64)

char DecChars[] = "0123456789";
char LowerHexChars[] = "0123456789abcdef";
char UpperHexChars[] = "0123456789ABCDEF";
internal void
U64ToASCII(format_dest *Dest, u64 Value, u32 Base, char *Digits)
{
    Assert(Base != 0);

    char *Start = Dest->At;
    do
    {
        u64 DigitIndex = (Value % Base);
        char Digit = Digits[DigitIndex];
        OutChar(Dest, Digit);

        Value /= Base;
    } while(Value != 0);
    char *End = Dest->At;

    while(Start < End)
    {
        --End;
        char Temp = *End;
        *End = *Start;
        *Start = Temp;
        ++Start;
    }
}

internal void
F64ToASCII(format_dest *Dest, f64 Value, u32 Precision)
{
    if(Value < 0)
    {
        OutChar(Dest, '-');
        Value = -Value;
    }

    u64 IntegerPart = (u64)Value;
    Value -= (f64)IntegerPart;
    U64ToASCII(Dest, IntegerPart, 10, DecChars);

    OutChar(Dest, '.');

    // TODO(casey): Note that this is NOT an accurate way to do this!
    for(u32 PrecisionIndex = 0;
        PrecisionIndex < Precision;
        ++PrecisionIndex)
    {
        Value *= 10.0f;
        u32 Integer = (u32)Value;
        Value -= (f32)Integer;
        OutChar(Dest, DecChars[Integer]);
    }
}

internal u64
MurmurHashUpdate(u64 h, u64 k)
{
    // NOTE(casey): This is based on the 128-bit MurmurHash from MurmurHash3

    u64 c1 = 0x87c37b91114253d5ULL;
    u64 c2 = 0x4cf5ad432745937fULL;
    u64 m1 = 5;
    u64 n1 = 0x52dce729ULL;

    k *= c1;
    k = RotateLeft(k, 31);
    k *= c2;

    h ^= k;

    h = RotateLeft(h, 27);
    h = h*m1+n1;

    return(h);
}

internal u64
MurmurHashFinalize(u64 h)
{
    // NOTE(casey): This is based on the 128-bit MurmurHash from MurmurHash3

    h ^= h >> 33ULL;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33ULL;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33ULL;

    return(h);
}

internal u64
CheckSumOf(buffer Buffer, u64 Seed = 1234)
{
    // TODO(casey): We would have to special-case the MurmurHash on big-endian machines
    // to make sure we matched, but I HATE BIG ENDIAN MACHINES AND THEY SHOULD
    // ALL GO AWAY DIE DIE DIE so we are not going to do that.

    u64 Result = Seed;

    u64 Count64 = (Buffer.Count / sizeof(u64));
    u64 Count8 = Buffer.Count - (Count64 * sizeof(u64));

    // TODO(casey): This may be unaligned... if we find speed problems on
    // an align-required platform, we may need to move to an aligned location
    // first.
    u64 *At = (u64 *)Buffer.Data;
    for(u64 Index = 0;
        Index < Count64;
        ++Index)
    {
        Result = MurmurHashUpdate(Result, *At++);
    }

    if(Count8)
    {
        u64 Residual = 0;
        Copy(Count8, At, &Residual);
        Result = MurmurHashUpdate(Result, Residual);
    }

    Result = MurmurHashFinalize(Result);

    return(Result);
}

global v3 DebugColorTable[] =
{
    /* 00 */ {1, 0, 0},
    /* 01 */ {0, 1, 0},
    /* 02 */ {0, 0, 1},
    /* 03 */ {1, 1, 0},
    /* 04 */ {0, 1, 1},
    /* 05 */ {1, 0, 1},
    /* 06 */ {1, 0.5f, 0},
    /* 07 */ {1, 0, 0.5f},
    /* 08 */ {0.5f, 1, 0},
    /* 09 */ {0, 1, 0.5f},
    /* 10 */ {0.5f, 0, 1},
    /* 11 */ {1, 0.75f, 0.5f},
    /* 12 */ {1, 0.5f, 0.75f},
    /* 13 */ {0.75f, 1, 0.5f},
    /* 14 */ {0.5f, 1, 0.75f},
    /* 15 */ {0.5f, 0.75f, 1},

    /* 16 */ {1, 0.25f, 0.25f},
    /* 17 */ {0.25f, 1, 0.25f},
    /* 18 */ {0.25f, 0.25f, 1},
    /* 19 */ {1, 1, 0.25f},
    /* 20 */ {0.25f, 1, 1},
    /* 21 */ {1, 0.25f, 1},
    /* 22 */ {1, 0.5f, 0.25f},
    /* 23 */ {1, 0.25f, 0.5f},
    /* 24 */ {0.5f, 1, 0.25f},
    /* 25 */ {0.25f, 1, 0.5f},
    /* 26 */ {0.5f, 0.25f, 1},
    /* 27 */ {1, 0.25f, 0.5f},
    /* 28 */ {1, 0.5f, 0.25f},
    /* 29 */ {0.25f, 1, 0.5f},
    /* 30 */ {0.5f, 1, 0.25f},
    /* 31 */ {0.5f, 0.25f, 1},
};

/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define CUBE(a) ((a)*(a)*(a))
#define SQUARE(a) ((a)*(a))

inline v2u
V2U(u32 X, u32 Y)
{
    v2u Result = {X, Y};

    return(Result);
}

inline v2s
V2S(s32 X, s32 Y)
{
    v2s Result = {X, Y};

    return(Result);
}

inline v2
V2i(int32 X, int32 Y)
{
    v2 Result = {(real32)X, (real32)Y};

    return(Result);
}

inline v2
V2i(uint32 X, uint32 Y)
{
    v2 Result = {(real32)X, (real32)Y};

    return(Result);
}

inline v2
V2(real32 X, real32 Y)
{
    v2 Result;

    Result.x = X;
    Result.y = Y;

    return(Result);
}

inline v2
V2(v2s A)
{
    v2 Result;

    Result.x = (f32)A.x;
    Result.y = (f32)A.y;

    return(Result);
}

inline v2
V2From(v2u Source)
{
    v2 Result = {(f32)Source.x, (f32)Source.y};
    return(Result);
}

inline v3
V3(real32 X, real32 Y, real32 Z)
{
    v3 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;

    return(Result);
}

inline v3
V3(v2 XY, real32 Z)
{
    v3 Result;

    Result.x = XY.x;
    Result.y = XY.y;
    Result.z = Z;

    return(Result);
}

inline v4
V4(real32 X, real32 Y, real32 Z, real32 W)
{
    v4 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;

    return(Result);
}

inline v4
V4(v3 XYZ, real32 W)
{
    v4 Result;

    Result.xyz = XYZ;
    Result.w = W;

    return(Result);
}

//
// NOTE(casey): Scalar operations
//

inline real32
Square(real32 A)
{
    real32 Result = A*A;

    return(Result);
}

inline r32
Sin01(r32 t)
{
    r32 Result = Sin(Pi32*t);

    return(Result);
}

inline r32
Triangle01(r32 t)
{
    r32 Result = 2.0f*t;
    if(Result > 1.0f)
    {
        Result = 2.0f - Result;
    }

    return(Result);
}

inline real32
Lerp(real32 A, real32 t, real32 B)
{
    real32 Result = (1.0f - t)*A + t*B;

    return(Result);
}

inline s32
S32BinormalLerp(s32 A, f32 tBinormal, s32 B)
{
    f32 t = 0.5f + (0.5f*tBinormal);
    f32 fResult = Lerp((f32)A, t, (f32)B);
    s32 Result = RoundReal32ToInt32(fResult);

    return(Result);
}

inline s32
Clamp(s32 Min, s32 Value, s32 Max)
{
    s32 Result = Value;

    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }

    return(Result);
}

inline real32
Clamp(real32 Min, real32 Value, real32 Max)
{
    real32 Result = Value;

    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }

    return(Result);
}

inline real32
Clamp01(real32 Value)
{
    real32 Result = Clamp(0.0f, Value, 1.0f);

    return(Result);
}

inline real32
Clamp01MapToRange(real32 Min, real32 t, real32 Max)
{
    real32 Result = 0.0f;

    real32 Range = Max - Min;
    if(Range != 0.0f)
    {
        Result = Clamp01((t - Min) / Range);
    }

    return(Result);
}

inline real32
ClampBinormalMapToRange(real32 Min, real32 t, real32 Max)
{
    real32 Result = -1.0f + 2.0f*Clamp01MapToRange(Min, t, Max);
    return(Result);
}

inline r32
ClampAboveZero(r32 Value)
{
    r32 Result = (Value < 0) ? 0.0f : Value;
    return(Result);
}

inline real32
SafeRatioN(real32 Numerator, real32 Divisor, real32 N)
{
    real32 Result = N;

    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }

    return(Result);
}

inline real32
SafeRatio0(real32 Numerator, real32 Divisor)
{
    real32 Result = SafeRatioN(Numerator, Divisor, 0.0f);

    return(Result);
}

inline real32
SafeRatio1(real32 Numerator, real32 Divisor)
{
    real32 Result = SafeRatioN(Numerator, Divisor, 1.0f);

    return(Result);
}

inline f64
SafeRatioN(f64 Numerator, f64 Divisor, f64 N)
{
    f64 Result = N;

    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }

    return(Result);
}

inline f64
SafeRatio0(f64 Numerator, f64 Divisor)
{
    f64 Result = SafeRatioN(Numerator, Divisor, 0.0);

    return(Result);
}

//
// NOTE(casey): v2u operations
//

function v2u operator+(v2u A, v2u B)
{
    v2u Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return Result;
}

function v2s operator+(v2s A, v2s B)
{
    v2s Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return Result;
}


//
// NOTE(casey): v2 operations
//

inline v2
Perp(v2 A)
{
    v2 Result = {-A.y, A.x};
    return(Result);
}

inline v2
operator*(real32 A, v2 B)
{
    v2 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;

    return(Result);
}

inline v2s
operator*(s32 A, v2s B)
{
    v2s Result;

    Result.x = A*B.x;
    Result.y = A*B.y;

    return(Result);
}

inline v2
operator*(v2 B, real32 A)
{
    v2 Result = A*B;

    return(Result);
}

inline v2 &
operator*=(v2 &B, real32 A)
{
    B = A * B;

    return(B);
}

inline v2
operator-(v2 A)
{
    v2 Result;

    Result.x = -A.x;
    Result.y = -A.y;

    return(Result);
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return(Result);
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;

    return(A);
}

inline v2s &
operator+=(v2s &A, v2s B)
{
    A = A + B;

    return(A);
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;

    return(Result);
}

inline v2 &
operator-=(v2 &A, v2 B)
{
    A = A - B;

    return(A);
}

inline v2
Hadamard(v2 A, v2 B)
{
    v2 Result = {A.x*B.x, A.y*B.y};

    return(Result);
}

inline v2s
Hadamard(v2s A, v2s B)
{
    v2s Result = {A.x*B.x, A.y*B.y};

    return(Result);
}

inline real32
Inner(v2 A, v2 B)
{
    real32 Result = A.x*B.x + A.y*B.y;

    return(Result);
}

inline real32
LengthSq(v2 A)
{
    real32 Result = Inner(A, A);

    return(Result);
}

inline real32
Length(v2 A)
{
    real32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v2
Clamp01(v2 Value)
{
    v2 Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);

    return(Result);
}

inline v2
Arm2(r32 Angle)
{
    v2 Result = {Cos(Angle), Sin(Angle)};

    return(Result);
}

//
// NOTE(casey): v3 operations
//

inline v3
operator*(real32 A, v3 B)
{
    v3 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;

    return(Result);
}

inline v3
operator*(v3 B, real32 A)
{
    v3 Result = A*B;

    return(Result);
}

inline v3 &
operator*=(v3 &B, real32 A)
{
    B = A * B;

    return(B);
}

inline v3
operator/(v3 B, real32 A)
{
    v3 Result = (1.0f/A)*B;

    return(Result);
}

inline v3
operator/(f32 B, v3 A)
{
    v3 Result =
    {
        B / A.x,
        B / A.y,
        B / A.z,
    };

    return(Result);
}

inline v3 &
operator/=(v3 &B, real32 A)
{
    B = B / A;

    return(B);
}

inline v3
operator-(v3 A)
{
    v3 Result;

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;

    return(Result);
}

inline v3
operator+(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return(Result);
}

inline v3 &
operator+=(v3 &A, v3 B)
{
    A = A + B;

    return(A);
}

inline v3
operator-(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;

    return(Result);
}

inline v3 &
operator-=(v3 &A, v3 B)
{
    A = A - B;

    return(A);
}

inline v3
operator*(v3 A, v3 B)
{
    v3 Result = {A.x*B.x, A.y*B.y, A.z*B.z};

    return(Result);
}

inline v3
operator/(v3 A, v3 B)
{
    v3 Result = {A.x/B.x, A.y/B.y, A.z/B.z};

    return(Result);
}

inline v3s
operator*(v3s A, v3s B)
{
    v3s Result = {A.x*B.x, A.y*B.y, A.z*B.z};

    return(Result);
}

inline real32
Inner(v3 A, v3 B)
{
    real32 Result = A.x*B.x + A.y*B.y + A.z*B.z;

    return(Result);
}

inline v3
Cross(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.y*B.z - A.z*B.y;
    Result.y = A.z*B.x - A.x*B.z;
    Result.z = A.x*B.y - A.y*B.x;

    return(Result);
}

inline real32
LengthSq(v3 A)
{
    real32 Result = Inner(A, A);

    return(Result);
}

inline real32
Length(v3 A)
{
    real32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v3
Normalize(v3 A)
{
    v3 Result = A * (1.0f / Length(A));

    return(Result);
}

inline v3
NOZ(v3 A)
{
    v3 Result = {};

    r32 LenSq = LengthSq(A);
    if(LenSq > Square(0.0001f))
    {
        Result = A * (1.0f / SquareRoot(LenSq));
    }

    return(Result);
}

internal v3
Floor(v3 Value)
{
    v3 Result = {Floor(Value.x), Floor(Value.y), Floor(Value.z)};
    return(Result);
}

internal v3
Round(v3 Value)
{
    v3 Result = {Round(Value.x), Round(Value.y), Round(Value.z)};
    return(Result);
}

internal v3
Clamp01(v3 Value)
{
    v3 Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);

    return(Result);
}

inline v3
Lerp(v3 A, real32 t, v3 B)
{
    v3 Result = (1.0f - t)*A + t*B;

    return(Result);
}

inline v3
Min(v3 A, v3 B)
{
    v3 Result =
    {
        Minimum(A.x, B.x),
        Minimum(A.y, B.y),
        Minimum(A.z, B.z),
    };

    return(Result);
}

inline v3
Max(v3 A, v3 B)
{
    v3 Result =
    {
        Maximum(A.x, B.x),
        Maximum(A.y, B.y),
        Maximum(A.z, B.z),
    };

    return(Result);
}

//
// NOTE(casey): v4 operations
//

inline v4
operator*(real32 A, v4 B)
{
    v4 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    Result.w = A*B.w;

    return(Result);
}

inline v4
operator*(v4 B, real32 A)
{
    v4 Result = A*B;

    return(Result);
}

inline v4 &
operator*=(v4 &B, real32 A)
{
    B = A * B;

    return(B);
}

inline v4
operator-(v4 A)
{
    v4 Result;

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;

    return(Result);
}

inline v4
operator+(v4 A, v4 B)
{
    v4 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;

    return(Result);
}

inline v4 &
operator+=(v4 &A, v4 B)
{
    A = A + B;

    return(A);
}

inline v4
operator-(v4 A, v4 B)
{
    v4 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;

    return(Result);
}

inline v4 &
operator-=(v4 &A, v4 B)
{
    A = A - B;

    return(A);
}

inline v4
Hadamard(v4 A, v4 B)
{
    v4 Result = {A.x*B.x, A.y*B.y, A.z*B.z, A.w*B.w};

    return(Result);
}

inline real32
Inner(v4 A, v4 B)
{
    real32 Result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;

    return(Result);
}

inline real32
LengthSq(v4 A)
{
    real32 Result = Inner(A, A);

    return(Result);
}

inline real32
Length(v4 A)
{
    real32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v4
Clamp01(v4 Value)
{
    v4 Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    Result.w = Clamp01(Value.w);

    return(Result);
}

inline v4
Lerp(v4 A, real32 t, v4 B)
{
    v4 Result = (1.0f - t)*A + t*B;

    return(Result);
}

inline v4
Min(v4 A, v4 B)
{
    v4 Result =
    {
        Minimum(A.x, B.x),
        Minimum(A.y, B.y),
        Minimum(A.z, B.z),
        Minimum(A.w, B.w),
    };

    return(Result);
}

inline v4
Max(v4 A, v4 B)
{
    v4 Result =
    {
        Maximum(A.x, B.x),
        Maximum(A.y, B.y),
        Maximum(A.z, B.z),
        Maximum(A.w, B.w),
    };

    return(Result);
}

//
// NOTE(casey): Rectangle2
//

inline rectangle2
InvertedInfinityRectangle2(void)
{
    rectangle2 Result;

    Result.Min.x = Result.Min.y = F32Max;
    Result.Max.x = Result.Max.y = -F32Max;

    return(Result);
}

inline rectangle2
Union(rectangle2 A, rectangle2 B)
{
    rectangle2 Result;

    Result.Min.x = (A.Min.x < B.Min.x) ? A.Min.x : B.Min.x;
    Result.Min.y = (A.Min.y < B.Min.y) ? A.Min.y : B.Min.y;
    Result.Max.x = (A.Max.x > B.Max.x) ? A.Max.x : B.Max.x;
    Result.Max.y = (A.Max.y > B.Max.y) ? A.Max.y : B.Max.y;

    return(Result);
}

inline v2
GetMinCorner(rectangle2 Rect)
{
    v2 Result = Rect.Min;
    return(Result);
}

inline v2
GetMaxCorner(rectangle2 Rect)
{
    v2 Result = Rect.Max;
    return(Result);
}

inline v2
GetDim(rectangle2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline rectangle2
Rectangle2From(rectangle2i Source)
{
    rectangle2 Result =
    {
        (f32)Source.Min.x,
        (f32)Source.Min.y,
        (f32)Source.Max.x,
        (f32)Source.Max.y,
    };

    return(Result);
}

inline v2
GetCenter(rectangle2 Rect)
{
    v2 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rectangle2
RectMinMax(v2 Min, v2 Max)
{
    rectangle2 Result;

    Result.Min = Min;
    Result.Max = Max;

    return(Result);
}

inline rectangle2
RectMinDim(v2 Min, v2 Dim)
{
    rectangle2 Result;

    Result.Min = Min;
    Result.Max = Min + Dim;

    return(Result);
}

inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result;

    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;

    return(Result);
}

inline rectangle2
AddRadiusTo(rectangle2 A, v2 Radius)
{
    rectangle2 Result;
    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;

    return(Result);
}

inline rectangle2
Offset(rectangle2 A, v2 Offset)
{
    rectangle2 Result;

    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;

    return(Result);
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f*Dim);

    return(Result);
}

inline bool32
IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    bool32 Result = ((Test.x >= Rectangle.Min.x) &&
                     (Test.y >= Rectangle.Min.y) &&
                     (Test.x < Rectangle.Max.x) &&
                     (Test.y < Rectangle.Max.y));

    return(Result);
}

inline b32
RectanglesIntersect(rectangle2 A, rectangle2 B)
{
    b32 Result = !((B.Max.x <= A.Min.x) ||
                   (B.Min.x >= A.Max.x) ||
                   (B.Max.y <= A.Min.y) ||
                   (B.Min.y >= A.Max.y));
    return(Result);
}

inline v2
GetBarycentric(rectangle2 A, v2 P)
{
    v2 Result;

    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);

    return(Result);
}

inline r32
GetArea(rectangle2 A)
{
    v2 Dim = GetDim(A);
    r32 Result = Dim.x*Dim.y;
    return(Result);
}

//
// NOTE(casey): Rectangle3
//

inline rectangle3
InvertedInfinityRectangle3(void)
{
    rectangle3 Result;

    Result.Min.x = Result.Min.y = Result.Min.z = F32Max;
    Result.Max.x = Result.Max.y = Result.Max.z = -F32Max;

    return(Result);
}

inline v3
GetMinCorner(rectangle3 Rect)
{
    v3 Result = Rect.Min;
    return(Result);
}

inline v3
GetMaxCorner(rectangle3 Rect)
{
    v3 Result = Rect.Max;
    return(Result);
}

inline v3
GetDim(rectangle3 Rect)
{
    v3 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline v3
GetRadius(rectangle3 Rect)
{
    v3 Result = 0.5f*(Rect.Max - Rect.Min);
    return(Result);
}

inline v3
GetCenter(rectangle3 Rect)
{
    v3 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rectangle3
RectMinMax(v3 Min, v3 Max)
{
    rectangle3 Result;

    Result.Min = Min;
    Result.Max = Max;

    return(Result);
}

inline rectangle3
RectMinDim(v3 Min, v3 Dim)
{
    rectangle3 Result;

    Result.Min = Min;
    Result.Max = Min + Dim;

    return(Result);
}

inline rectangle3
RectCenterHalfDim(v3 Center, v3 HalfDim)
{
    rectangle3 Result;

    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;

    return(Result);
}

inline rectangle3
AddRadiusTo(rectangle3 A, v3 Radius)
{
    rectangle3 Result;

    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;

    return(Result);
}

inline rectangle3
Offset(rectangle3 A, v3 Offset)
{
    rectangle3 Result;

    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;

    return(Result);
}

inline rectangle3
RectCenterDim(v3 Center, v3 Dim)
{
    rectangle3 Result = RectCenterHalfDim(Center, 0.5f*Dim);

    return(Result);
}

inline bool32
IsInRectangle(rectangle3 Rectangle, v3 Test)
{
    bool32 Result = ((Test.x >= Rectangle.Min.x) &&
                     (Test.y >= Rectangle.Min.y) &&
                     (Test.z >= Rectangle.Min.z) &&
                     (Test.x < Rectangle.Max.x) &&
                     (Test.y < Rectangle.Max.y) &&
                     (Test.z < Rectangle.Max.z));

    return(Result);
}

inline b32x
IsInRectangleCenterHalfDim(v3 P, v3 Radius, v3 Test)
{
    v3 Rel = Test - P;
    b32x Result = ((AbsoluteValue(Rel.x) <= Radius.x) &&
                   (AbsoluteValue(Rel.y) <= Radius.y) &&
                   (AbsoluteValue(Rel.z) <= Radius.z));

    return(Result);
}

inline bool32
RectanglesIntersect(rectangle3 A, rectangle3 B)
{
    bool32 Result = !((B.Max.x <= A.Min.x) ||
                      (B.Min.x >= A.Max.x) ||
                      (B.Max.y <= A.Min.y) ||
                      (B.Min.y >= A.Max.y) ||
                      (B.Max.z <= A.Min.z) ||
                      (B.Min.z >= A.Max.z));
    return(Result);
}

inline v3
GetBarycentric(rectangle3 A, v3 P)
{
    v3 Result;

    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    Result.z = SafeRatio0(P.z - A.Min.z, A.Max.z - A.Min.z);

    return(Result);
}

inline v3
PointFromUVW(rectangle3 A, v3 UVW)
{
    v3 Result;

    Result.x = Lerp(A.Min.x, UVW.x, A.Max.x);
    Result.y = Lerp(A.Min.y, UVW.y, A.Max.y);
    Result.z = Lerp(A.Min.z, UVW.z, A.Max.z);

    return(Result);
}

inline rectangle2
ToRectangleXY(rectangle3 A)
{
    rectangle2 Result;

    Result.Min = A.Min.xy;
    Result.Max = A.Max.xy;

    return(Result);
}

inline rectangle3
Union(rectangle3 A, rectangle3 B)
{
    rectangle3 Result;

    Result.Min.x = (A.Min.x < B.Min.x) ? A.Min.x : B.Min.x;
    Result.Min.y = (A.Min.y < B.Min.y) ? A.Min.y : B.Min.y;
    Result.Min.z = (A.Min.z < B.Min.z) ? A.Min.z : B.Min.z;

    Result.Max.x = (A.Max.x > B.Max.x) ? A.Max.x : B.Max.x;
    Result.Max.y = (A.Max.y > B.Max.y) ? A.Max.y : B.Max.y;
    Result.Max.z = (A.Max.z > B.Max.z) ? A.Max.z : B.Max.z;

    return(Result);
}

inline rectangle3
Intersect(rectangle3 A, rectangle3 B)
{
    rectangle3 Result;

    Result.Min.x = (A.Min.x < B.Min.x) ? B.Min.x : A.Min.x;
    Result.Min.y = (A.Min.y < B.Min.y) ? B.Min.y : A.Min.y;
    Result.Min.z = (A.Min.z < B.Min.z) ? B.Min.z : A.Min.z;

    Result.Max.x = (A.Max.x > B.Max.x) ? B.Max.x : A.Max.x;
    Result.Max.y = (A.Max.y > B.Max.y) ? B.Max.y : A.Max.y;
    Result.Max.z = (A.Max.z > B.Max.z) ? B.Max.z : A.Max.z;

    return(Result);
}

internal v3
GetMinZCenterP(rectangle3 R)
{
    v3 Result = GetCenter(R);
    Result.z = GetMinCorner(R).z;
    return(Result);
}

internal v3
GetMaxZCenterP(rectangle3 R)
{
    v3 Result = GetCenter(R);
    Result.z = GetMaxCorner(R).z;
    return(Result);
}

internal rectangle3
MakeRelative(rectangle3 R, v3 P)
{
    rectangle3 Result = Offset(R, -P);
    return(Result);
}

//
//
//

inline s32
GetWidth(rectangle2i A)
{
    s32 Result = A.Max.x - A.Min.x;
    return(Result);
}

inline s32
GetHeight(rectangle2i A)
{
    s32 Result = A.Max.y - A.Min.y;
    return(Result);
}

inline rectangle2i
Intersect(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;

    Result.Min.x = (A.Min.x < B.Min.x) ? B.Min.x : A.Min.x;
    Result.Min.y = (A.Min.y < B.Min.y) ? B.Min.y : A.Min.y;
    Result.Max.x = (A.Max.x > B.Max.x) ? B.Max.x : A.Max.x;
    Result.Max.y = (A.Max.y > B.Max.y) ? B.Max.y : A.Max.y;

    return(Result);
}

inline rectangle2i
Union(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;

    Result.Min.x = (A.Min.x < B.Min.x) ? A.Min.x : B.Min.x;
    Result.Min.y = (A.Min.y < B.Min.y) ? A.Min.y : B.Min.y;
    Result.Max.x = (A.Max.x > B.Max.x) ? A.Max.x : B.Max.x;
    Result.Max.y = (A.Max.y > B.Max.y) ? A.Max.y : B.Max.y;

    return(Result);
}

inline int32
GetClampedRectArea(rectangle2i A)
{
    int32 Width = (A.Max.x - A.Min.x);
    int32 Height = (A.Max.y - A.Min.y);
    int32 Result = 0;
    if((Width > 0) && (Height > 0))
    {
        Result = Width*Height;
    }

    return(Result);
}

inline bool32
HasArea(rectangle2i A)
{
    bool32 Result = ((A.Min.x < A.Max.x) && (A.Min.y < A.Max.y));

    return(Result);
}

internal b32 IsOnEdge(rectangle2i Rect, v2s TileIndex)
{
    b32 Result = ((TileIndex.x == Rect.Min.x) ||
                  (TileIndex.x == (Rect.Max.x - 1)) ||
                  (TileIndex.y == Rect.Min.y) ||
                  (TileIndex.y == (Rect.Max.y - 1)));
    return Result;
}

#if 0

internal b32x IsInVolume(gen_volume *Vol, s32 X, s32 Y)
{
    b32x Result = ((X >= Vol->Min.x) && (X <= Vol->Max.x) &&
                   (Y >= Vol->Min.y) && (Y <= Vol->Max.y));

    return(Result);
}

internal b32x IsInVolume(gen_volume *Vol, v2s P)
{
    b32x Result = IsInVolume(Vol, P.x, P.y);
    return(Result);
}

internal b32x IsInArrayBounds(gen_v3 Bounds, gen_v3 P)
{
    b32x Result = ((P.x >= 0) && (P.x < Bounds.x) &&
                   (P.y >= 0) && (P.y < Bounds.y) &&
                   (P.z >= 0) && (P.z < Bounds.z));

    return(Result);
}

internal gen_v3 GetDim(gen_volume Vol)
{
    gen_v3 Result =
    {
        (Vol.Max.x - Vol.Min.x) + 1,
        (Vol.Max.y - Vol.Min.y) + 1,
        (Vol.Max.z - Vol.Min.z) + 1,
    };

    return(Result);
}

internal b32x IsMinimumDimensionsForRoom(gen_volume Vol)
{
    gen_v3 Dim = GetDim(Vol);
    b32x Result = ((Dim.x >= 4) &&
                   (Dim.y >= 4) &&
                   (Dim.z >= 1));

    return(Result);
}

internal b32x HasVolume(gen_volume Vol)
{
    gen_v3 Dim = GetDim(Vol);
    b32x Result = ((Dim.x > 0) &&
                   (Dim.y > 0) &&
                   (Dim.z > 0));

    return(Result);
}

internal s32 GetTotalVolume(gen_v3 Dim)
{
    s32 Result = (Dim.x*Dim.y*Dim.z);
    return(Result);
}

internal gen_volume GetMaximumVolumeFor(gen_volume MinVol, gen_volume MaxVol)
{
    gen_volume Result;

    Result.Min.x = MinVol.Min.x;
    Result.Min.y = MinVol.Min.y;
    Result.Min.z = MinVol.Min.z;

    Result.Max.x = MaxVol.Max.x;
    Result.Max.y = MaxVol.Max.y;
    Result.Max.z = MaxVol.Max.z;

    return(Result);
}

internal void ClipMin(gen_volume *Vol, u32 Dim, s32 Val)
{
    if(Vol->Min.E[Dim] < Val)
    {
        Vol->Min.E[Dim] = Val;
    }
}

internal void ClipMax(gen_volume *Vol, u32 Dim, s32 Val)
{
    if(Vol->Max.E[Dim] > Val)
    {
        Vol->Max.E[Dim] = Val;
    }
}

internal gen_volume Union(gen_volume *A, gen_volume *B)
{
    gen_volume Result;

    for(u32 Dim = 0;
        Dim < 3;
        ++Dim)
    {
        Result.Min.E[Dim] = Minimum(A->Min.E[Dim], B->Min.E[Dim]);
        Result.Max.E[Dim] = Maximum(A->Max.E[Dim], B->Max.E[Dim]);
    }

    return(Result);
}

internal gen_volume Intersect(gen_volume *A, gen_volume *B)
{
    gen_volume Result;

    for(u32 Dim = 0;
        Dim < 3;
        ++Dim)
    {
        Result.Min.E[Dim] = Maximum(A->Min.E[Dim], B->Min.E[Dim]);
        Result.Max.E[Dim] = Minimum(A->Max.E[Dim], B->Max.E[Dim]);
    }

    return(Result);
}

internal gen_volume AddRadiusTo(gen_volume *A, gen_v3 Radius)
{
    gen_volume Result = *A;

    Result.Min.E[0] -= Radius.E[0];
    Result.Min.E[1] -= Radius.E[1];
    Result.Min.E[2] -= Radius.E[2];

    Result.Max.E[0] += Radius.E[0];
    Result.Max.E[1] += Radius.E[1];
    Result.Max.E[2] += Radius.E[2];

    return(Result);
}
#endif

inline b32x
HasArea(rectangle3 A)
{
    b32x Result = ((A.Min.x < A.Max.x) && (A.Min.y < A.Max.y) && (A.Min.z < A.Max.z));

    return(Result);
}

inline rectangle2i
InvertedInfinityRectangle2i(void)
{
    rectangle2i Result;

    Result.Min.x = Result.Min.y = INT_MAX;
    Result.Max.x = Result.Max.y = -INT_MAX;

    return(Result);
}

inline rectangle2i
Offset(rectangle2i A, s32 X, s32 Y)
{
    rectangle2i Result = A;

    Result.Min.x += X;
    Result.Max.x += X;
    Result.Min.y += Y;
    Result.Max.y += Y;

    return(Result);
}

inline rectangle2i
RectMinMax(s32 MinX, s32 MinY, s32 MaxX, s32 MaxY)
{
    rectangle2i Result;

    Result.Min.x = MinX;
    Result.Min.y = MinY;
    Result.Max.x = MaxX;
    Result.Max.y = MaxY;

    return(Result);
}

inline rectangle2i
RectMinDim(s32 MinX, s32 MinY, s32 DimX, s32 DimY)
{
    rectangle2i Result;

    Result.Min.x = MinX;
    Result.Min.y = MinY;
    Result.Max.x = MinX + DimX;
    Result.Max.y = MinY + DimY;

    return(Result);
}

inline v4
sRGBLinearize(v4 C)
{
    v4 Result;

    Result.r = Square(C.r);
    Result.g = Square(C.g);
    Result.b = Square(C.b);
    Result.a = C.a;

    return(Result);
}

inline v4
LinearTosRGB(v4 C)
{
    v4 Result;

    Result.r = SquareRoot(C.r);
    Result.g = SquareRoot(C.g);
    Result.b = SquareRoot(C.b);
    Result.a = C.a;

    return(Result);
}

inline v4
sRGBLinearize(f32 R, f32 G, f32 B, f32 A)
{
    v4 Input = {R, G, B, A};
    v4 Result = sRGBLinearize(Input);
    return(Result);
}

inline v4
SRGB255ToLinear1(v4 C)
{
    v4 Result;

    real32 Inv255 = 1.0f / 255.0f;

    Result.r = Square(Inv255*C.r);
    Result.g = Square(Inv255*C.g);
    Result.b = Square(Inv255*C.b);
    Result.a = Inv255*C.a;

    return(Result);
}

inline v4
Linear1ToSRGB255(v4 C)
{
    v4 Result;

    real32 One255 = 255.0f;

    Result.r = One255*SquareRoot(C.r);
    Result.g = One255*SquareRoot(C.g);
    Result.b = One255*SquareRoot(C.b);
    Result.a = One255*C.a;

    return(Result);
}

internal m4x4
operator*(m4x4 A, m4x4 B)
{
    // NOTE(casey): This is written to be instructive, not optimal!

    m4x4 R = {};

    for(int r = 0; r <= 3; ++r) // NOTE(casey): Rows (of A)
    {
        for(int c = 0; c <= 3; ++c) // NOTE(casey): Column (of B)
        {
            for(int i = 0; i <= 3; ++i) // NOTE(casey): Columns of A, rows of B!
            {
                R.E[r][c] += A.E[r][i]*B.E[i][c];
            }
        }
    }

    return(R);
}

internal v4
Transform(m4x4 A, v4 P)
{
    // NOTE(casey): This is written to be instructive, not optimal!

    v4 R;

    R.x = P.x*A.E[0][0] + P.y*A.E[0][1] + P.z*A.E[0][2] + P.w*A.E[0][3];
    R.y = P.x*A.E[1][0] + P.y*A.E[1][1] + P.z*A.E[1][2] + P.w*A.E[1][3];
    R.z = P.x*A.E[2][0] + P.y*A.E[2][1] + P.z*A.E[2][2] + P.w*A.E[2][3];
    R.w = P.x*A.E[3][0] + P.y*A.E[3][1] + P.z*A.E[3][2] + P.w*A.E[3][3];

    return(R);
}

inline v3
operator*(m4x4 A, v3 P)
{
    v3 R = Transform(A, V4(P, 1.0f)).xyz;
    return(R);
}

inline v4
operator*(m4x4 A, v4 P)
{
    v4 R = Transform(A, P);
    return(R);
}

inline m4x4
Identity(void)
{
    m4x4 R =
    {
        {{1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}},
    };

    return(R);
}

inline m4x4
XRotation(f32 Angle)
{
    f32 c = Cos(Angle);
    f32 s = Sin(Angle);

    m4x4 R =
    {
        {{1, 0, 0, 0},
            {0, c,-s, 0},
            {0, s, c, 0},
            {0, 0, 0, 1}},
    };

    return(R);
}

inline m4x4
YRotation(f32 Angle)
{
    f32 c = Cos(Angle);
    f32 s = Sin(Angle);

    m4x4 R =
    {
        {{ c, 0, s, 0},
            { 0, 1, 0, 0},
            {-s, 0, c, 0},
            { 0, 0, 0, 1}},
    };

    return(R);
}

inline m4x4
ZRotation(f32 Angle)
{
    f32 c = Cos(Angle);
    f32 s = Sin(Angle);

    m4x4 R =
    {
        {{c,-s, 0, 0},
            {s, c, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}},
    };

    return(R);
}

inline m4x4
Translation(v3 T)
{
    m4x4 R =
    {
        {{1, 0, 0, T.x},
            {0, 1, 0, T.y},
            {0, 0, 1, T.z},
            {0, 0, 0, 1}},
    };

    return(R);
}

inline m4x4
Transpose(m4x4 A)
{
    m4x4 R;

    for(int j = 0; j <= 3; ++j)
    {
        for(int i = 0; i <= 3; ++i)
        {
            R.E[j][i] = A.E[i][j];
        }
    }

    return(R);
}

inline m4x4_inv
PerspectiveProjection(f32 AspectWidthOverHeight, f32 FocalLength, f32 NearClipPlane, f32 FarClipPlane)
{
    f32 a = 1.0f;
    f32 b = AspectWidthOverHeight;
    f32 c = FocalLength; // NOTE(casey): This should really be called "film back distance"

    f32 n = NearClipPlane; // NOTE(casey): Near clip plane _distance_
    f32 f = FarClipPlane; // NOTE(casey): Far clip plane _distance_

    // NOTE(casey): These are the perspective correct terms, for when you divide by -z
    f32 d = (n+f) / (n-f);
    f32 e = (2*f*n) / (n-f);

    m4x4_inv Result =
    {
        // NOTE(casey): Forward
        {{{a*c,    0,  0,  0},
                {  0,  b*c,  0,  0},
                {  0,    0,  d,  e},
                {  0,    0, -1,  0}}},

        // NOTE(casey): Inverse
        {{{1/(a*c),       0,   0,   0},
                {      0, 1/(b*c),   0,   0},
                {      0,       0,   0,  -1},
                {      0,       0, 1/e, d/e}}},
    };

#if HANDMADE_SLOW
    m4x4 I = Result.Inverse*Result.Forward;
    v4 Test0 = Result.Forward*V4(0, 0, -n, 1);
    Test0.xyz /= Test0.w;
    v4 Test1 = Result.Forward*V4(0, 0, -f, 1);
    Test1.xyz /= Test1.w;
#endif

    return(Result);
}

inline m4x4_inv
OrthographicProjection(f32 AspectWidthOverHeight, f32 NearClipPlane, f32 FarClipPlane)
{
    f32 a = 1.0f;
    f32 b = AspectWidthOverHeight;

    f32 n = NearClipPlane; // NOTE(casey): Near clip plane _distance_
    f32 f = FarClipPlane; // NOTE(casey): Far clip plane _distance_

    // NOTE(casey): These are the non-perspective corrected terms, for orthographic
    f32 d = 2.0f / (n - f);
    f32 e = (n + f) / (n - f);

    m4x4_inv Result =
    {
        {{{a,  0,  0,  0},
                {0,  b,  0,  0},
                {0,  0,  d,  e},
                {0,  0,  0,  1}}},

        {{{1/a,   0,   0,    0},
                {  0, 1/b,   0,    0},
                {  0,   0, 1/d, -e/d},
                {  0,   0,   0,    1}}},
    };

#if HANDMADE_SLOW
    m4x4 I = Result.Inverse*Result.Forward;
    v3 Test0 = Result.Forward*V3(0, 0, -n);
    v3 Test1 = Result.Forward*V3(0, 0, -f);
#endif

    return(Result);
}

internal m4x4
Columns3x3(v3 X, v3 Y, v3 Z)
{
    m4x4 R =
    {
        {{X.x, Y.x, Z.x, 0},
            {X.y, Y.y, Z.y, 0},
            {X.z, Y.z, Z.z, 0},
            {  0,   0,   0, 1}}
    };

    return(R);
}

internal m4x4
Rows3x3(v3 X, v3 Y, v3 Z)
{
    m4x4 R =
    {
        {{X.x, X.y, X.z, 0},
            {Y.x, Y.y, Y.z, 0},
            {Z.x, Z.y, Z.z, 0},
            {  0,   0,   0, 1}}
    };

    return(R);
}

internal m4x4
Translate(m4x4 A, v3 T)
{
    m4x4 R = A;

    R.E[0][3] += T.x;
    R.E[1][3] += T.y;
    R.E[2][3] += T.z;

    return(R);
}

inline v3
GetColumn(m4x4 A, u32 C)
{
    v3 R = {A.E[0][C], A.E[1][C], A.E[2][C]};
    return(R);
}

inline v3
GetRow(m4x4 A, u32 R)
{
    v3 Result = {A.E[R][0], A.E[R][1], A.E[R][2]};
    return(Result);
}

internal m4x4_inv
CameraTransform(v3 X, v3 Y, v3 Z, v3 P)
{
    m4x4_inv Result;

    // TODO(casey): It seems really suspicious that unary negation binds first
    // to the m4x4... is that actually the C++ grammar?  I guess it is :(
    m4x4 A = Rows3x3(X, Y, Z);
    v3 AP = -(A*P);
    A = Translate(A, AP);
    Result.Forward = A;

    v3 iX = X/LengthSq(X);
    v3 iY = Y/LengthSq(Y);
    v3 iZ = Z/LengthSq(Z);
    v3 iP = {AP.x*iX.x + AP.y*iY.x + AP.z*iZ.x,
        AP.x*iX.y + AP.y*iY.y + AP.z*iZ.y,
        AP.x*iX.z + AP.y*iY.z + AP.z*iZ.z};

    m4x4 B = Columns3x3(iX, iY, iZ);
    B = Translate(B, -iP);
    Result.Inverse = B;

#if HANDMADE_SLOW
    m4x4 I = Result.Inverse*Result.Forward;
#endif

    return(Result);
}

internal v2
RayIntersect2(v2 Pa, v2 ra, v2 Pb, v2 rb)
{
    v2 Result = {};

    /* NOTE(casey):

       Pa.x + ta*ra.x = Pb.x + tb*rb.x
       Pa.y + ta*ra.y = Pb.y + tb*rb.y
    */

    f32 d = (rb.x*ra.y - rb.y*ra.x);
    if(d != 0.0f)
    {
        f32 ta = ((Pa.x - Pb.x)*rb.y + (Pb.y - Pa.y)*rb.x) / d;
        f32 tb = ((Pa.x - Pb.x)*ra.y + (Pb.y - Pa.y)*ra.x) / d;

        Result = V2(ta, tb);
    }

    return(Result);
}

internal u32
SwapRAndB(u32 C)
{
    u32 Result = ((C & 0xFF00FF00) |
                  ((C >> 16) & 0xFF) |
                  ((C & 0xFF) << 16));

    return(Result);
}

internal u32
ReplAlpha(u32 C)
{
    u32 Alpha = (C >> 24);
    u32 Result = ((Alpha << 24) |
                  (Alpha << 16) |
                  (Alpha <<  8) |
                  (Alpha <<  0));

    return(Result);
}

internal u32
MulAlpha(u32 C)
{
    u32 C0 = ((C >> 0) & 0xFF);
    u32 C1 = ((C >> 8) & 0xFF);
    u32 C2 = ((C >> 16) & 0xFF);
    u32 Alpha = (C >> 24);

    // NOTE(casey): This is a quick-and-dirty lossy multiply, where you lose one bit
    C0 = ((C0*Alpha) >> 8);
    C1 = ((C1*Alpha) >> 8);
    C2 = ((C2*Alpha) >> 8);

    u32 Result = ((Alpha << 24) |
                  (C2 << 16) |
                  (C1 <<  8) |
                  (C0 <<  0));

    return(Result);
}

inline v4
BGRAUnpack4x8(u32 Packed)
{
    v4 Result = {(real32)((Packed >> 16) & 0xFF),
        (real32)((Packed >> 8) & 0xFF),
        (real32)((Packed >> 0) & 0xFF),
        (real32)((Packed >> 24) & 0xFF)};

    return(Result);
}

inline u32
BGRAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundReal32ToUInt32(Unpacked.a) << 24) |
                  (RoundReal32ToUInt32(Unpacked.r) << 16) |
                  (RoundReal32ToUInt32(Unpacked.g) << 8) |
                  (RoundReal32ToUInt32(Unpacked.b) << 0));

    return(Result);
}

inline u64
BGRAPack4x16(v4 Unpacked)
{
    u64 Result = (((u64)RoundReal32ToUInt32(Unpacked.a) << 48) |
                  ((u64)RoundReal32ToUInt32(Unpacked.r) << 32) |
                  ((u64)RoundReal32ToUInt32(Unpacked.g) << 16) |
                  ((u64)RoundReal32ToUInt32(Unpacked.b) << 0));

    return(Result);
}

inline v4
RGBAUnpack4x8(u32 Packed)
{
    v4 Result = {(real32)((Packed >> 0) & 0xFF),
        (real32)((Packed >> 8) & 0xFF),
        (real32)((Packed >> 16) & 0xFF),
        (real32)((Packed >> 24) & 0xFF)};

    return(Result);
}

inline u32
RGBAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundReal32ToUInt32(Unpacked.a) << 24) |
                  (RoundReal32ToUInt32(Unpacked.b) << 16) |
                  (RoundReal32ToUInt32(Unpacked.g) << 8) |
                  (RoundReal32ToUInt32(Unpacked.r) << 0));

    return(Result);
}

inline b32x
IsInRange(f32 Min, f32 Value, f32 Max)
{
    b32x Result = ((Min <= Value) &&
                   (Value <= Max));

    return(Result);
}

internal rectangle2i
AspectRatioFit(u32 RenderWidth, u32 RenderHeight,
               u32 WindowWidth, u32 WindowHeight)
{
    rectangle2i Result = {};

    if((RenderWidth > 0) &&
       (RenderHeight > 0) &&
       (WindowWidth > 0) &&
       (WindowHeight > 0))
    {
        r32 OptimalWindowWidth = (r32)WindowHeight * ((r32)RenderWidth / (r32)RenderHeight);
        r32 OptimalWindowHeight = (r32)WindowWidth * ((r32)RenderHeight / (r32)RenderWidth);

        if(OptimalWindowWidth > (r32)WindowWidth)
        {
            // NOTE(casey): Width-constrained display - top and bottom black bars
            Result.Min.x = 0;
            Result.Max.x = WindowWidth;

            r32 Empty = (r32)WindowHeight - OptimalWindowHeight;
            s32 HalfEmpty = RoundReal32ToInt32(0.5f*Empty);
            s32 UseHeight = RoundReal32ToInt32(OptimalWindowHeight);

            Result.Min.y = HalfEmpty;
            Result.Max.y = Result.Min.y + UseHeight;
        }
        else
        {
            // NOTE(casey): Height-constrained display - left and right black bars
            Result.Min.y = 0;
            Result.Max.y = WindowHeight;

            r32 Empty = (r32)WindowWidth - OptimalWindowWidth;
            s32 HalfEmpty = RoundReal32ToInt32(0.5f*Empty);
            s32 UseWidth = RoundReal32ToInt32(OptimalWindowWidth);

            Result.Min.x = HalfEmpty;
            Result.Max.x = Result.Min.x + UseWidth;
        }
    }

    return(Result);
}

internal r32
FitCameraDistanceToHalfDim(f32 FocalLength, f32 MonitorHalfDimInMeters, f32 HalfDimInMeters)
{
    f32 Result = (FocalLength*HalfDimInMeters) / MonitorHalfDimInMeters;
    return(Result);
}

internal v2
FitCameraDistanceToHalfDim(f32 FocalLength, f32 MonitorHalfDimInMeters, v2 HalfDimInMeters)
{
    v2 Result =
    {
        FitCameraDistanceToHalfDim(FocalLength, MonitorHalfDimInMeters, HalfDimInMeters.x),
        FitCameraDistanceToHalfDim(FocalLength, MonitorHalfDimInMeters, HalfDimInMeters.y),
    };

    return(Result);
}

internal v3s
FloorToV3S(v3 A)
{
    v3s Result =
    {
        FloorReal32ToInt32(A.x),
        FloorReal32ToInt32(A.y),
        FloorReal32ToInt32(A.z),
    };

    return(Result);
}

internal v3s
RoundToV3S(v3 A)
{
    v3s Result =
    {
        RoundReal32ToInt32(A.x),
        RoundReal32ToInt32(A.y),
        RoundReal32ToInt32(A.z),
    };

    return(Result);
}

internal v3s
V3S(s32 X, s32 Y, s32 Z)
{
    v3s Result = {X, Y, Z};

    return(Result);
}

internal v3
V3(v3s A)
{
    v3 Result =
    {
        (f32)A.x,
        (f32)A.y,
        (f32)A.z,
    };

    return(Result);
}

internal v3s
operator-(v3s A, v3s B)
{
    v3s Result =
    {
        A.x - B.x,
        A.y - B.y,
        A.z - B.z,
    };

    return(Result);
}

internal v2s
operator-(v2s A, v2s B)
{
    v2s Result =
    {
        A.x - B.x,
        A.y - B.y,
    };

    return(Result);
}

internal v3s
operator+(v3s A, v3s B)
{
    v3s Result =
    {
        A.x + B.x,
        A.y + B.y,
        A.z + B.z,
    };

    return(Result);
}

internal v3s &
operator+=(v3s &A, v3s B)
{
    A = A + B;
    return A;
}

internal v3s
operator*(v3s B, s32 A)
{
    v3s Result =
    {
        A * B.x,
        A * B.y,
        A * B.z,
    };

    return(Result);
}

internal v3s
operator*(s32 A, v3s B)
{
    v3s Result =
    {
        A * B.x,
        A * B.y,
        A * B.z,
    };

    return(Result);
}

internal v3s
operator/(v3s A, s32 B)
{
    v3s Result =
    {
        A.x / B,
        A.y / B,
        A.z / B,
    };

    return(Result);
}

internal v3s
Clamp(v3s Min, v3s V, v3s Max)
{
    v3s Result =
    {
        Clamp(Min.x, V.x, Max.x),
        Clamp(Min.y, V.y, Max.y),
        Clamp(Min.z, V.z, Max.z),
    };

    return Result;
}

internal v3
GetClosestPointInBox(rectangle3 Box, v3 P)
{
    v3 Result = Min(Box.Max, Max(Box.Min, P));
    return(Result);
}

internal f32
GetDistanceToBoxSq(rectangle3 Box, v3 P)
{
    v3 ClosestP = GetClosestPointInBox(Box, P);
    f32 DistanceSq = LengthSq(P - ClosestP);
    return(DistanceSq);
}

internal v2
OctahedralFromUnitVector(v3 V)
{
    f32 OneNorm = AbsoluteValue(V.x) + AbsoluteValue(V.y) + AbsoluteValue(V.z);
    v2 Result = (1.0f / OneNorm)*V.xy;
    if(V.z < 0)
    {
        f32 Ox = SignOf(Result.x) * (1.0f - AbsoluteValue(Result.y));
        f32 Oy = SignOf(Result.y) * (1.0f - AbsoluteValue(Result.x));
        Result.x = Ox;
        Result.y = Oy;
    }

    return(Result);
}

internal v3
UnitVectorFromOctahedral(v2 O)
{
    f32 Ox = O.x;
    f32 Oy = O.y;
    f32 SumXY = (AbsoluteValue(Ox) + AbsoluteValue(Oy));
    f32 Oz = 1.0f - SumXY;
    if(SumXY > 1)
    {
        Ox = SignOf(O.x) * (1.0f - AbsoluteValue(O.y));
        Oy = SignOf(O.y) * (1.0f - AbsoluteValue(O.x));
    }

    v3 Result = V3(Ox, Oy, Oz);
    Result = Normalize(Result);

    return(Result);
}

internal v3
DirectionFromTxTy(v2 OxyCoefficient, u32 Tx, u32 Ty)
{
    v2 Oxy =
    {
        OxyCoefficient.x * (f32)(Tx - 1),
        OxyCoefficient.y * (f32)(Ty - 1),
    };
    Oxy = 2.0f*Oxy - V2(1.0f, 1.0f);
    //Oxy += (1.0f / (f32)(LIGHT_COLOR_LOOKUP_SQUARE_DIM - 2))*V2(1,1);
    v3 SampleDir = UnitVectorFromOctahedral(Oxy);

    return(SampleDir);
}

internal v2u
GetOctahedralOffset(v2 OctDimCoefficient, v3 Dir)
{
    v2 UV = OctahedralFromUnitVector(Dir);
    UV = 0.5f*(V2(1.0f, 1.0f) + UV);

    v2 I = Hadamard(OctDimCoefficient, UV) + V2(1.5f, 1.5f);
    v2u Result =
    {
        (u32)FloorReal32ToInt32(I.x),
        (u32)FloorReal32ToInt32(I.y),
    };

    return(Result);
}

internal b32x
AreEqual(v3s A, v3s B)
{
    b32x Result = ((A.x == B.x) &&
                   (A.y == B.y) &&
                   (A.z == B.z));

    return(Result);
}

function b32 AreEqual(v2s A, v2s B)
{
    b32 Result = ((A.x == B.x) &&
                  (A.y == B.y));
    return Result;
}

internal b32x
AreEqual(v2u A, v2u B)
{
    b32x Result = ((A.x == B.x) &&
                   (A.y == B.y));


    return(Result);
}

internal b32x
LooksFishy(f32 A)
{
    b32x Result = !((A >= -10000.0f) &&
                    (A <=  10000.0f));
    return(Result);
}

internal b32x
LooksFishy(v3 A)
{
    b32x Result = (LooksFishy(A.x) ||
                   LooksFishy(A.y) ||
                   LooksFishy(A.z));

    return(Result);
}

internal f32 DistanceBetweenLineSegmentAndPointSq(v2 L0, v2 L1, v2 P)
{
    v2 Delta = L1 - L0;
    v2 RelP = P - L0;

    f32 DeltaLen = Length(Delta);
    v2 Dir = (1.0f / DeltaLen) * Delta;
    f32 t = Clamp(0.0f, Inner(Dir, RelP), DeltaLen);

    v2 ClosestP = L0 + t*Dir;
    f32 Result = LengthSq(ClosestP - P);

    return Result;
}

internal v3
GetDebugColor3(u32 Value)
{
    v3 Result = DebugColorTable[Value % ArrayCount(DebugColorTable)];
    return(Result);
}

internal v4
GetDebugColor4(u32 Value, f32 Alpha = 1.0f)
{
    v4 Result = V4(GetDebugColor3(Value), Alpha);
    return(Result);
}

enum debug_type
{
    DebugType_Unknown,

    DebugType_Name,

    DebugType_FrameMarker,
    DebugType_BeginBlock,
    DebugType_EndBlock,

    DebugType_OpenDataBlock,
    DebugType_CloseDataBlock,
    DebugType_SetHUD,

    DebugType_ArenaSetName,
    DebugType_ArenaBlockFree,
    DebugType_ArenaBlockTruncate,
    DebugType_ArenaBlockAllocate,
    DebugType_ArenaAllocate,

    //    DebugType_MarkDebugValue,

    DebugType_string_ptr,
    DebugType_b32,
    DebugType_r32,
    DebugType_u32,
    DebugType_umm,
    DebugType_s32,
    DebugType_v2,
    DebugType_v3,
    DebugType_v4,
    DebugType_rectangle2,
    DebugType_rectangle3,
    DebugType_bitmap_id,
    DebugType_sound_id,
    DebugType_font_id,

    DebugType_ThreadIntervalGraph,
    DebugType_FrameBarGraph,
    // DebugType_CounterFunctionList,
    DebugType_LastFrameInfo,
    DebugType_FrameSlider,
    DebugType_TopClocksList,
    DebugType_FunctionSummary,
    DebugType_MemoryByArena,
    DebugType_MemoryByFrame,
    DebugType_MemoryBySize,
};

#define DEBUG_NAME__(A, B, C) A "|" #B "|" #C
#define DEBUG_NAME_(A, B, C) DEBUG_NAME__(A, B, C)
#define DEBUG_NAME(Name) DEBUG_NAME_(__FILE__, __LINE__, __COUNTER__)

#define DEBUG_RECORD_BLOCK_ALLOCATION(Bl) \
{ \
RecordDebugEvent(DebugType_ArenaBlockAllocate, DEBUG_NAME("ArenaBlockAllocate"), "BlockAlloc"); \
Event->Value_debug_memory_block_op.ArenaLookupBlock = (Bl)->ArenaPrev; \
Event->Value_debug_memory_block_op.Block = (Bl); \
Event->Value_debug_memory_block_op.AllocatedSize = (Bl)->Size; \
}

internal void *
AllocateMemory(umm Size)
{
    umm TotalSize = sizeof(platform_memory_block) + Size;
    platform_memory_block *Block = (platform_memory_block *)malloc(TotalSize);
    memset(Block, 0, TotalSize);

    Block->Size = Size;
    Block->Base = (u8 *)(Block + 1);
    Block->Used = 0;

    return(Block);
}

inline void *
PushSize_(INTERNAL_MEMORY_PARAM
          memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    void *Result = 0;

    Assert(Params.Alignment <= 128);
    Assert(IsPow2(Params.Alignment));

    memory_index Size = 0;
    if(Arena->CurrentBlock)
    {
        Size = GetEffectiveSizeFor(Arena, SizeInit, Params);
    }

    if(!Arena->CurrentBlock ||
       ((Arena->CurrentBlock->Used + Size) > Arena->CurrentBlock->Size))
    {
        Size = SizeInit; // NOTE(casey): The base will automatically be aligned now!
        if(Arena->AllocationFlags & (PlatformMemory_OverflowCheck|
                                     PlatformMemory_UnderflowCheck))
        {
            Arena->MinimumBlockSize = 0;
            Size = AlignPow2(Size, Params.Alignment);
        }
        else if(!Arena->MinimumBlockSize)
        {
            // TODO(casey): Tune default block size eventually?
            Arena->MinimumBlockSize = 1024*1024;
        }

        memory_index BlockSize = Maximum(Size, Arena->MinimumBlockSize);

        platform_memory_block *NewBlock =
            (platform_memory_block *) AllocateMemory(BlockSize);
        NewBlock->ArenaPrev = Arena->CurrentBlock;
        Arena->CurrentBlock = NewBlock;
    }

    Assert((Arena->CurrentBlock->Used + Size) <= Arena->CurrentBlock->Size);

    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    umm OffsetInBlock = Arena->CurrentBlock->Used + AlignmentOffset;
    Result = Arena->CurrentBlock->Base + OffsetInBlock;
    Arena->CurrentBlock->Used += Size;

    Assert(Size >= SizeInit);

    // NOTE(casey): This is just to guarantee that nobody passed in an alignment
    // on their first allocation that was _greater_ that than the page alignment
    Assert(Arena->CurrentBlock->Used <= Arena->CurrentBlock->Size);

    if(Params.Flags & ArenaFlag_ClearToZero)
    {
        ZeroSize(SizeInit, Result);
    }

    return(Result);
}

internal void *
PushCopy_(INTERNAL_MEMORY_PARAM
          memory_arena *Arena, umm Size, void *Source, arena_push_params Params = DefaultArenaParams())
{
    void *Result = PushSize_(INTERNAL_MEMORY_PASS Arena, Size, Params);
    Copy(Size, Source, Result);
    return(Result);
}

// NOTE(casey): This is generally not for production use, this is probably
// only really something we need during testing, but who knows
inline char *
PushStringZ_(INTERNAL_MEMORY_PARAM
             memory_arena *Arena, char *Source)
{
    u32 Size = 1;
    for(char *At = Source;
        *At;
        ++At)
    {
        ++Size;
    }

    char *Dest = (char *)PushSize_(INTERNAL_MEMORY_PASS Arena, Size, NoClear());
    for(u32 CharIndex = 0;
        CharIndex < Size;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }

    return(Dest);
}

internal buffer
PushBuffer_(INTERNAL_MEMORY_PARAM
            memory_arena *Arena, umm Size, arena_push_params Params = DefaultArenaParams())
{
    buffer Result;
    Result.Count = Size;
    Result.Data = (u8 *)PushSize_(INTERNAL_MEMORY_PASS Arena, Result.Count, Params);

    return(Result);
}

internal string
PushString_(INTERNAL_MEMORY_PARAM
            memory_arena *Arena, char *Source)
{
    string Result;
    Result.Count = StringLength(Source);
    Result.Data = (u8 *)PushCopy_(INTERNAL_MEMORY_PASS Arena, Result.Count, Source);

    return(Result);
}

internal string
PushString_(INTERNAL_MEMORY_PARAM
            memory_arena *Arena, string Source)
{
    string Result;
    Result.Count = Source.Count;
    Result.Data = (u8 *)PushCopy_(INTERNAL_MEMORY_PASS Arena, Result.Count, Source.Data);

    return(Result);
}

inline char *
PushAndNullTerminate_(INTERNAL_MEMORY_PARAM
                      memory_arena *Arena, u32 Length, char *Source)
{
    char *Dest = (char *)PushSize_(INTERNAL_MEMORY_PASS Arena, Length + 1, NoClear());
    for(u32 CharIndex = 0;
        CharIndex < Length;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }
    Dest[Length] = 0;

    return(Dest);
}

internal void
Win32FreeMemoryBlock(win32_memory_block *Block)
{
    //BeginTicketMutex(&GlobalWin32State.MemoryMutex);
    Block->Prev->Next = Block->Next;
    Block->Next->Prev = Block->Prev;
    //EndTicketMutex(&GlobalWin32State.MemoryMutex);

    // NOTE(casey): For porting to other platforms that need the size to unmap
    // pages, you can get it from Block->Block.Size!

    BOOL Result = VirtualFree(Block, 0, MEM_RELEASE);
    Assert(Result);
}

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;

    Result.Arena = Arena;
    Result.Block = Arena->CurrentBlock;
    Result.Used = Arena->CurrentBlock ? Arena->CurrentBlock->Used : 0;

    ++Arena->TempCount;

    return(Result);
}

void DeallocateMemory(platform_memory_block *Block)
{
    if(Block)
    {
        win32_memory_block *Win32Block = ((win32_memory_block *)Block);
            Win32FreeMemoryBlock(Win32Block);
    }
}

inline void
FreeLastBlock(memory_arena *Arena)
{
    platform_memory_block *Free = Arena->CurrentBlock;
    Arena->CurrentBlock = Free->ArenaPrev;
    DeallocateMemory(Free);
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    while(Arena->CurrentBlock != TempMem.Block)
    {
        FreeLastBlock(Arena);
    }

    if(Arena->CurrentBlock)
    {
        Assert(Arena->CurrentBlock->Used >= TempMem.Used);
        Arena->CurrentBlock->Used = TempMem.Used;
    }

    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void
KeepTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;

    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void
Clear(memory_arena *Arena)
{
    while(Arena->CurrentBlock)
    {
        // NOTE(casey): Because the arena itself may be stored in the last block,
        // we must ensure that we don't look at it after freeing.
        b32 ThisIsLastBlock = (Arena->CurrentBlock->ArenaPrev == 0);
        FreeLastBlock(Arena);
        if(ThisIsLastBlock)
        {
            break;
        }
    }
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

inline void *
BootstrapPushSize_(INTERNAL_MEMORY_PARAM umm StructSize, umm OffsetToArena,
                   arena_bootstrap_params BootstrapParams = DefaultBootstrapParams(),
                   arena_push_params Params = DefaultArenaParams())
{
    memory_arena Bootstrap = {};
    Bootstrap.AllocationFlags = BootstrapParams.AllocationFlags;
    Bootstrap.MinimumBlockSize = BootstrapParams.MinimumBlockSize;
    void *Struct = PushSize_(INTERNAL_MEMORY_PASS &Bootstrap, StructSize, Params);
    *(memory_arena *)((u8 *)Struct + OffsetToArena) = Bootstrap;

    return(Struct);
}

struct memory_arena;

struct stream_chunk
{
    buffer Contents;
    stream_chunk *Next;

#if HANDMADE_INTERNAL
    char *GUID;
#endif
};

struct stream
{
    memory_arena *Memory;
    stream *Errors;

    buffer Contents;

    u32 BitCount;
    u32 BitBuf;

    b32 Underflowed;

    stream_chunk *First;
    stream_chunk *Last;
};

#if HANDMADE_INTERNAL
#else
#endif

internal stream OnDemandMemoryStream(memory_arena *Memory, stream *Errors = 0);

#define OutStruct(Stream, type) (type *)OutSize(Stream, sizeof(type));
#define OutStructCopy(Stream, Instance) OutCopy(Stream, sizeof(Instance), &(Instance))

internal stream_chunk *
AppendChunk(stream *Stream, umm Size, void *Contents)
{
    stream_chunk *Chunk = PushStruct(Stream->Memory, stream_chunk);

    Chunk->Contents.Count = Size;
    Chunk->Contents.Data = (u8 *)Contents;
    Chunk->Next = 0;

    Stream->Last = ((Stream->Last ? Stream->Last->Next : Stream->First) = Chunk);

    return(Chunk);
}

internal void
RefillIfNecessary(stream *File)
{
    // TODO(casey): Use a free list to recycle chunks, if we ever care

    if((File->Contents.Count == 0) && File->First)
    {
        stream_chunk *This = File->First;
        File->Contents = This->Contents;
        File->First = This->Next;
    }
}

#define Consume(File, type) (type *)ConsumeSize(File, sizeof(type))
internal void *
ConsumeSize(stream *File, u32 Size)
{
    RefillIfNecessary(File);

    void *Result = Advance(&File->Contents, Size);
    if(!Result)
    {
        File->Underflowed = true;
    }

    Assert(!File->Underflowed);

    return(Result);
}

internal u32
PeekBits(stream *Buf, u32 BitCount)
{
    Assert(BitCount <= 32);

    u32 Result = 0;

    while((Buf->BitCount < BitCount) &&
          !Buf->Underflowed)
    {
        u32 Byte = *Consume(Buf, u8);
        Buf->BitBuf |= (Byte << Buf->BitCount);
        Buf->BitCount += 8;
    }

    Result = Buf->BitBuf & ((1 << BitCount) - 1);

    return(Result);
}

internal void
DiscardBits(stream *Buf, u32 BitCount)
{
    Buf->BitCount -= BitCount;
    Buf->BitBuf >>= BitCount;
}

internal u32
ConsumeBits(stream *Buf, u32 BitCount)
{
    u32 Result = PeekBits(Buf, BitCount);
    DiscardBits(Buf, BitCount);

    return(Result);
}

internal void
FlushByte(stream *Buf)
{
    u32 FlushCount = (Buf->BitCount % 8);
    ConsumeBits(Buf, FlushCount);
}

internal void *
OutCopy(stream *Dest, umm Count, void *Data)
{
    void *Result = PushCopy(Dest->Memory, Count, Data);
    AppendChunk(Dest, Count, Result);
    return(Result);
}

internal void *
OutSize(stream *Dest, umm Count)
{
    void *Result = PushSize(Dest->Memory, Count);
    AppendChunk(Dest, Count, Result);
    return(Result);
}

internal stream
MakeReadStream(buffer Contents, stream *Errors)
{
    stream Result = {};

    Result.Errors = Errors;
    Result.Contents = Contents;

    return(Result);
}

internal stream
OnDemandMemoryStream(memory_arena *Memory, stream *Errors)
{
    stream Result = {};

    Result.Memory = Memory;
    Result.Errors = Errors;

    return(Result);
}

internal umm
GetTotalSize(stream *Stream)
{
    umm Result = 0;

    for(stream_chunk *Chunk = Stream->First;
        Chunk;
        Chunk = Chunk->Next)
    {
        Result += Chunk->Contents.Count;
    }

    return(Result);
}

internal void
CopyStreamToBuffer(stream *Source, buffer Dest)
{
    Assert(GetTotalSize(Source) <= Dest.Count);

    u64 DataOffset = 0;
    for(stream_chunk *Chunk = Source->First;
        Chunk;
        Chunk = Chunk->Next)
    {
        Copy((u32)Chunk->Contents.Count, Chunk->Contents.Data, Dest.Data + DataOffset);
        DataOffset += Chunk->Contents.Count;
    }
}

internal image_u32 ParsePNG(memory_arena *Memory, stream File, stream *Info = 0);
internal void WritePNG(u32 Width, u32 Height, u32 *Pixels, stream *Out);

global u32 GlobalCRCTable[256] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

internal void
EndianSwap(u16 *Value)
{
    u16 V = (*Value);
#if 1
    *Value = ((V << 8) | (V >> 8));
#else
    *Value = _byteswap_ushort(V);
#endif
}

internal void
EndianSwap(u32 *Value)
{
    u32 V = (*Value);
#if 1
    *Value = ((V << 24) |
              ((V & 0xFF00) << 8) |
              ((V >> 8) & 0xFF00) |
              (V >> 24));
#else
    *Value = _byteswap_ulong(V);
#endif
}

internal u32
EndBigCRC(umm SkipN, stream_chunk *StartChunk, stream_chunk *OnePastLastChunk = 0)
{
    // NOTE(casey): This code is just a modified version of the reference
    // code for the CRC given in Annex D of the PNG specification.
    u32 Result = 0xffffffffL;

    for(stream_chunk *Chunk = StartChunk;
        Chunk != OnePastLastChunk;
        Chunk = Chunk->Next)
    {
        Assert(SkipN <= Chunk->Contents.Count);
        for(umm n = SkipN; n < Chunk->Contents.Count; ++n)
        {
            Result = GlobalCRCTable[(Result ^ Chunk->Contents.Data[n]) & 0xff] ^ (Result >> 8);
        }
        SkipN = 0;
    }

    Result = (Result ^ 0xffffffffL);
    EndianSwap(&Result);
    return(Result);
}

internal u32
GetTotalImageSize(image_u32 Image)
{
    u32 Result = Image.Width*Image.Height*4;
    return(Result);
}
internal image_u32
PushImage(memory_arena *Arena, u32 Width, u32 Height)
{
    image_u32 Result;

    Result.Width = Width;
    Result.Height = Height;
    Result.Pixels = (u32 *)PushSize(Arena, GetTotalImageSize(Result));
    return(Result);
}

internal u32
ReverseBits(u32 V, u32 BitCount)
{
    u32 Result = 0;

    for(u32 BitIndex = 0;
        BitIndex <= (BitCount / 2);
        ++BitIndex)
    {
        u32 Inv = (BitCount - (BitIndex + 1));
        Result |= ((V >> BitIndex) & 0x1) << Inv;
        Result |= ((V >> Inv) & 0x1) << BitIndex;
    }

    return(Result);
}

internal void *
AllocatePixels(memory_arena *Memory, u32 Width, u32 Height, u32 BPP, u32x ExtraBytes = 0)
{
    void *Result = PushSize(Memory, Width*Height*BPP + (ExtraBytes*Height));

    return(Result);
}

internal png_huffman
AllocateHuffman(memory_arena *Memory, u32 MaxCodeLengthInBits)
{
    Assert(MaxCodeLengthInBits <= PNG_HUFFMAN_MAX_BIT_COUNT);

    png_huffman Result = {};

    Result.MaxCodeLengthInBits = MaxCodeLengthInBits;
    Result.EntryCount = (1 << MaxCodeLengthInBits);
    Result.Entries = PushArray(Memory, Result.EntryCount, png_huffman_entry);

    memset(Result.Entries, 0, sizeof(png_huffman_entry));

    return(Result);
}

internal void
ComputeHuffman(u32 SymbolCount, u32 *SymbolCodeLength, png_huffman *Result, u32 SymbolAddend = 0)
{
    u32 CodeLengthHist[PNG_HUFFMAN_MAX_BIT_COUNT] = {};
    for(u32 SymbolIndex = 0;
        SymbolIndex < SymbolCount;
        ++SymbolIndex)
    {
        u32 Count = SymbolCodeLength[SymbolIndex];
        Assert(Count <= ArrayCount(CodeLengthHist));
        ++CodeLengthHist[Count];
    }

    u32 NextUnusedCode[PNG_HUFFMAN_MAX_BIT_COUNT];
    NextUnusedCode[0] = 0;
    CodeLengthHist[0] = 0;
    for(u32 BitIndex = 1;
        BitIndex < ArrayCount(NextUnusedCode);
        ++BitIndex)
    {
        NextUnusedCode[BitIndex] = ((NextUnusedCode[BitIndex - 1] +
                                     CodeLengthHist[BitIndex - 1]) << 1);
    }

    for(u32 SymbolIndex = 0;
        SymbolIndex < SymbolCount;
        ++SymbolIndex)
    {
        u32 CodeLengthInBits = SymbolCodeLength[SymbolIndex];
        if(CodeLengthInBits)
        {
            Assert(CodeLengthInBits < ArrayCount(NextUnusedCode));
            u32 Code = NextUnusedCode[CodeLengthInBits]++;

            u32 ArbitraryBits = Result->MaxCodeLengthInBits - CodeLengthInBits;
            u32 EntryCount = (1 << ArbitraryBits);

            for(u32 EntryIndex = 0;
                EntryIndex < EntryCount;
                ++EntryIndex)
            {
                u32 BaseIndex = (Code << ArbitraryBits) | EntryIndex;
                u32 Index = ReverseBits(BaseIndex, Result->MaxCodeLengthInBits);

                png_huffman_entry *Entry = Result->Entries + Index;

                u32 Symbol = (SymbolIndex + SymbolAddend);
                Entry->BitsUsed = (u16)CodeLengthInBits;
                Entry->Symbol = (u16)Symbol;

                Assert(Entry->BitsUsed == CodeLengthInBits);
                Assert(Entry->Symbol == Symbol);
            }
        }
    }
}

internal u32
HuffmanDecode(png_huffman *Huffman, stream *Input)
{
    u32 EntryIndex = PeekBits(Input, Huffman->MaxCodeLengthInBits);
    Assert(EntryIndex < Huffman->EntryCount);

    png_huffman_entry Entry = Huffman->Entries[EntryIndex];

    u32 Result = Entry.Symbol;
    DiscardBits(Input, Entry.BitsUsed);
    Assert(Entry.BitsUsed);

    return(Result);
}

global png_huffman_entry PNGLengthExtra[] =
{
    {3, 0}, // NOTE(casey): 257
    {4, 0}, // NOTE(casey): 258
    {5, 0}, // NOTE(casey): 259
    {6, 0}, // NOTE(casey): 260
    {7, 0}, // NOTE(casey): 261
    {8, 0}, // NOTE(casey): 262
    {9, 0}, // NOTE(casey): 263
    {10, 0}, // NOTE(casey): 264
    {11, 1}, // NOTE(casey): 265
    {13, 1}, // NOTE(casey): 266
    {15, 1}, // NOTE(casey): 267
    {17, 1}, // NOTE(casey): 268
    {19, 2}, // NOTE(casey): 269
    {23, 2}, // NOTE(casey): 270
    {27, 2}, // NOTE(casey): 271
    {31, 2}, // NOTE(casey): 272
    {35, 3}, // NOTE(casey): 273
    {43, 3}, // NOTE(casey): 274
    {51, 3}, // NOTE(casey): 275
    {59, 3}, // NOTE(casey): 276
    {67, 4}, // NOTE(casey): 277
    {83, 4}, // NOTE(casey): 278
    {99, 4}, // NOTE(casey): 279
    {115, 4}, // NOTE(casey): 280
    {131, 5}, // NOTE(casey): 281
    {163, 5}, // NOTE(casey): 282
    {195, 5}, // NOTE(casey): 283
    {227, 5}, // NOTE(casey): 284
    {258, 0}, // NOTE(casey): 285
};

global png_huffman_entry PNGDistExtra[] =
{
    {1, 0}, // NOTE(casey): 0
    {2, 0}, // NOTE(casey): 1
    {3, 0}, // NOTE(casey): 2
    {4, 0}, // NOTE(casey): 3
    {5, 1}, // NOTE(casey): 4
    {7, 1}, // NOTE(casey): 5
    {9, 2}, // NOTE(casey): 6
    {13, 2}, // NOTE(casey): 7
    {17, 3}, // NOTE(casey): 8
    {25, 3}, // NOTE(casey): 9
    {33, 4}, // NOTE(casey): 10
    {49, 4}, // NOTE(casey): 11
    {65, 5}, // NOTE(casey): 12
    {97, 5}, // NOTE(casey): 13
    {129, 6}, // NOTE(casey): 14
    {193, 6}, // NOTE(casey): 15
    {257, 7}, // NOTE(casey): 16
    {385, 7}, // NOTE(casey): 17
    {513, 8}, // NOTE(casey): 18
    {769, 8}, // NOTE(casey): 19
    {1025, 9}, // NOTE(casey): 20
    {1537, 9}, // NOTE(casey): 21
    {2049, 10}, // NOTE(casey): 22
    {3073, 10}, // NOTE(casey): 23
    {4097, 11}, // NOTE(casey): 24
    {6145, 11}, // NOTE(casey): 25
    {8193, 12}, // NOTE(casey): 26
    {12289, 12}, // NOTE(casey): 27
    {16385, 13}, // NOTE(casey): 28
    {24577, 13}, // NOTE(casey): 29
};

internal u8
PNGFilter1And2(u8 *x, u8 *a, u32 Channel)
{
    u8 Result = (u8)x[Channel] + (u8)a[Channel];
    return(Result);
}

internal u8
PNGFilter3(u8 *x, u8 *a, u8 *b, u32 Channel)
{
    u32 Average = ((u32)a[Channel] + (u32)b[Channel]) / 2;
    u8 Result = (u8)x[Channel] + (u8)Average;
    return(Result);
}

internal u8
PNGFilter4(u8 *x, u8 *aFull, u8 *bFull, u8 *cFull, u32 Channel)
{
    s32 a = (s32)aFull[Channel];
    s32 b = (s32)bFull[Channel];
    s32 c = (s32)cFull[Channel];
    s32 p = a + b - c;

    s32 pa = p - a;
    if(pa < 0) {pa = -pa;}

    s32 pb = p - b;
    if(pb < 0) {pb = -pb;}

    s32 pc = p - c;
    if(pc < 0) {pc = -pc;}

    s32 Paeth;
    if((pa <= pb) && (pa <= pc))
    {
        Paeth = a;
    }
    else if(pb <= pc)
    {
        Paeth = b;
    }
    else
    {
        Paeth = c;
    }

    u8 Result = (u8)x[Channel] + (u8)Paeth;
    return(Result);
}

internal void
PNGFilterReconstruct(u32x Width, u32x Height, u8 *DecompressedPixels, u8 *FinalPixels,
                     stream *Errors)
{
    // NOTE(casey): If you cared about speed, this filter process
    // seems tailor-made for SIMD - you could go 4-wide trivially.
    u32 Zero = 0;
    u8 *PriorRow = (u8 *)&Zero;
    s32 PriorRowAdvance = 0;
    u8 *Source = DecompressedPixels;
    u8 *Dest = FinalPixels;

    for(u32 Y = 0;
        Y < Height;
        ++Y)
    {
        u8 Filter = *Source++;
        u8 *CurrentRow = Dest;

        switch(Filter)
        {
            case 0:
            {
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    *(u32 *)Dest = *(u32 *)Source;
                    Dest += 4;
                    Source += 4;
                }
            } break;

            case 1:
            {
                u32 APixel = 0;
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    Dest[0] = PNGFilter1And2(Source, (u8 *)&APixel, 0);
                    Dest[1] = PNGFilter1And2(Source, (u8 *)&APixel, 1);
                    Dest[2] = PNGFilter1And2(Source, (u8 *)&APixel, 2);
                    Dest[3] = PNGFilter1And2(Source, (u8 *)&APixel, 3);

                    APixel = *(u32 *)Dest;

                    Dest += 4;
                    Source += 4;
                }
            } break;

            case 2:
            {
                u8 *BPixel = PriorRow;
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    Dest[0] = PNGFilter1And2(Source, BPixel, 0);
                    Dest[1] = PNGFilter1And2(Source, BPixel, 1);
                    Dest[2] = PNGFilter1And2(Source, BPixel, 2);
                    Dest[3] = PNGFilter1And2(Source, BPixel, 3);

                    BPixel += PriorRowAdvance;
                    Dest += 4;
                    Source += 4;
                }
            } break;

            case 3:
            {
                // TODO(casey): This has not been tested!

                u32 APixel = 0;
                u8 *BPixel = PriorRow;
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    Dest[0] = PNGFilter3(Source, (u8 *)&APixel, BPixel, 0);
                    Dest[1] = PNGFilter3(Source, (u8 *)&APixel, BPixel, 1);
                    Dest[2] = PNGFilter3(Source, (u8 *)&APixel, BPixel, 2);
                    Dest[3] = PNGFilter3(Source, (u8 *)&APixel, BPixel, 3);

                    APixel = *(u32 *)Dest;

                    BPixel += PriorRowAdvance;
                    Dest += 4;
                    Source += 4;
                }
            } break;

            case 4:
            {
                u32 APixel = 0;
                u32 CPixel = 0;
                u8 *BPixel = PriorRow;
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    Dest[0] = PNGFilter4(Source, (u8 *)&APixel, BPixel, (u8 *)&CPixel, 0);
                    Dest[1] = PNGFilter4(Source, (u8 *)&APixel, BPixel, (u8 *)&CPixel, 1);
                    Dest[2] = PNGFilter4(Source, (u8 *)&APixel, BPixel, (u8 *)&CPixel, 2);
                    Dest[3] = PNGFilter4(Source, (u8 *)&APixel, BPixel, (u8 *)&CPixel, 3);

                    CPixel = *(u32 *)BPixel;
                    APixel = *(u32 *)Dest;

                    BPixel += PriorRowAdvance;
                    Dest += 4;
                    Source += 4;
                }
            } break;

            default:
            {
            } break;

        }

        PriorRow = CurrentRow;
        PriorRowAdvance = 4;
    }
}

internal image_u32
ParsePNG(memory_arena *Memory, stream File, stream *Info)
{
    // NOTE(casey): This is NOT MEANT TO BE FAULT TOLERANT.  It only loads specifically
    // what we expect, and is happy to crash otherwise.

    stream *At = &File;

    b32x Supported = false;

    u32 Width = 0;
    u32 Height = 0;
    u8 *FinalPixels = 0;

    png_header *FileHeader = Consume(At, png_header);
    if(FileHeader)
    {
        stream CompData = OnDemandMemoryStream(Memory, File.Errors);

        while(At->Contents.Count > 0)
        {
            png_chunk_header *ChunkHeader = Consume(At, png_chunk_header);
            if(ChunkHeader)
            {
                EndianSwap(&ChunkHeader->Length);
                EndianSwap(&ChunkHeader->TypeU32);

                void *ChunkData = ConsumeSize(At, ChunkHeader->Length);
                png_chunk_footer *ChunkFooter = Consume(At, png_chunk_footer);
                EndianSwap(&ChunkFooter->CRC);

                if(ChunkHeader->TypeU32 == 'IHDR')
                {
                    png_ihdr *IHDR = (png_ihdr *)ChunkData;

                    EndianSwap(&IHDR->Width);
                    EndianSwap(&IHDR->Height);

                    if((IHDR->BitDepth == 8) &&
                       (IHDR->ColorType == 6) &&
                       (IHDR->CompressionMethod == 0) &&
                       (IHDR->FilterMethod == 0) &&
                       (IHDR->InterlaceMethod == 0))
                    {
                        Width = IHDR->Width;
                        Height = IHDR->Height;
                        Supported = true;
                    }
                }
                else if(ChunkHeader->TypeU32 == 'IDAT')
                {
                    AppendChunk(&CompData, ChunkHeader->Length, ChunkData);
                }
            }
        }

        if(Supported)
        {
            png_idat_header *IDATHead = Consume(&CompData, png_idat_header);

            u8 CM = (IDATHead->ZLibMethodFlags & 0xF);
            u8 CINFO = (IDATHead->ZLibMethodFlags >> 4);
            u8 FCHECK = (IDATHead->AdditionalFlags & 0x1F);
            u8 FDICT = (IDATHead->AdditionalFlags >> 5) & 0x1;
            u8 FLEVEL = (IDATHead->AdditionalFlags >> 6);


            Supported = ((CM == 8) && (FDICT == 0));

            if(Supported)
            {
                FinalPixels = (u8 *)AllocatePixels(Memory, Width, Height, 4);
                u8 *DecompressedPixels = (u8 *)AllocatePixels(Memory, Width, Height, 4, 1);
                u8 *DecompressedPixelsEnd = DecompressedPixels + (Height*((Width*4) + 1));
                u8 *Dest = DecompressedPixels;

                u32 BFINAL = 0;
                while(BFINAL == 0)
                {
                    Assert(Dest <= DecompressedPixelsEnd);
                    BFINAL = ConsumeBits(&CompData, 1);
                    u32 BTYPE = ConsumeBits(&CompData, 2);

                    if(BTYPE == 0)
                    {
                        FlushByte(&CompData);

                        u16 LEN = (u16)ConsumeBits(&CompData, 16);
                        u16 NLEN = (u16)ConsumeBits(&CompData, 16);
                        if((u16)LEN != (u16)~NLEN)
                        {
                        }

                        while(LEN)
                        {
                            RefillIfNecessary(&CompData);

                            u16 UseLEN = LEN;
                            if(UseLEN > CompData.Contents.Count)
                            {
                                UseLEN = (u16)CompData.Contents.Count;
                            }

                            u8 *Source = (u8 *)ConsumeSize(&CompData, UseLEN);
                            if(Source)
                            {
                                u16 CopyCount = UseLEN;
                                while(CopyCount--)
                                {
                                    *Dest++ = *Source++;
                                }
                            }

                            LEN -= UseLEN;
                        }
                    }
                    else if(BTYPE == 3)
                    {
                    }
                    else
                    {
                        u32 LitLenDistTable[512] = {};
                        png_huffman LitLenHuffman = AllocateHuffman(Memory, 15);
                        png_huffman DistHuffman = AllocateHuffman(Memory, 15);

                        u32 HLIT = 0;
                        u32 HDIST = 0;
                        if(BTYPE == 2)
                        {
                            HLIT = ConsumeBits(&CompData, 5);
                            HDIST = ConsumeBits(&CompData, 5);
                            u32 HCLEN = ConsumeBits(&CompData, 4);

                            HLIT += 257;
                            HDIST += 1;
                            HCLEN += 4;

                            u32 HCLENSwizzle[] =
                            {
                                16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15,
                            };
                            Assert(HCLEN <= ArrayCount(HCLENSwizzle));
                            u32 HCLENTable[ArrayCount(HCLENSwizzle)] = {};

                            for(u32 Index = 0;
                                Index < HCLEN;
                                ++Index)
                            {
                                HCLENTable[HCLENSwizzle[Index]] = ConsumeBits(&CompData, 3);
                            }

                            png_huffman DictHuffman = AllocateHuffman(Memory, 7);
                            ComputeHuffman(ArrayCount(HCLENSwizzle), HCLENTable, &DictHuffman);

                            u32 LitLenCount = 0;
                            u32 LenCount = HLIT + HDIST;
                            Assert(LenCount <= ArrayCount(LitLenDistTable));
                            while(LitLenCount < LenCount)
                            {
                                u32 RepCount = 1;
                                u32 RepVal = 0;
                                u32 EncodedLen = HuffmanDecode(&DictHuffman, &CompData);
                                if(EncodedLen <= 15)
                                {
                                    RepVal = EncodedLen;
                                }
                                else if(EncodedLen == 16)
                                {
                                    RepCount = 3 + ConsumeBits(&CompData, 2);

                                    Assert(LitLenCount > 0);
                                    RepVal = LitLenDistTable[LitLenCount - 1];
                                }
                                else if(EncodedLen == 17)
                                {
                                    RepCount = 3 + ConsumeBits(&CompData, 3);
                                }
                                else if(EncodedLen == 18)
                                {
                                    RepCount = 11 + ConsumeBits(&CompData, 7);
                                }
                                else
                                {
                                }

                                while(RepCount--)
                                {
                                    LitLenDistTable[LitLenCount++] = RepVal;
                                }
                            }
                            Assert(LitLenCount == LenCount);

                        }
                        else if(BTYPE == 1)
                        {
                            HLIT = 288;
                            HDIST = 32;
                            u32 BitCounts[][2] =
                            {
                                {143, 8},
                                {255, 9},
                                {279, 7},
                                {287, 8},
                                {319, 5},
                            };

                            u32 BitCountIndex = 0;
                            for(u32 RangeIndex = 0;
                                RangeIndex < ArrayCount(BitCounts);
                                ++RangeIndex)
                            {
                                u32 BitCount = BitCounts[RangeIndex][1];
                                u32 LastValue = BitCounts[RangeIndex][0];
                                while(BitCountIndex <= LastValue)
                                {
                                    LitLenDistTable[BitCountIndex++] = BitCount;
                                }
                            }
                        }
                        else
                        {
                        }

                        ComputeHuffman(HLIT, LitLenDistTable, &LitLenHuffman);
                        ComputeHuffman(HDIST, LitLenDistTable + HLIT, &DistHuffman);

                        for(;;)
                        {
                            u32 LitLen = HuffmanDecode(&LitLenHuffman, &CompData);
                            if(LitLen <= 255)
                            {
                                u32 Out = (LitLen & 0xFF);
                                *Dest++ = (u8)Out;
                            }
                            else if(LitLen >= 257)
                            {
                                u32 LenTabIndex = (LitLen - 257);
                                png_huffman_entry LenTab = PNGLengthExtra[LenTabIndex];
                                u32 Len = LenTab.Symbol;
                                if(LenTab.BitsUsed)
                                {
                                    u32 ExtraBits = ConsumeBits(&CompData, LenTab.BitsUsed);
                                    Len += ExtraBits;
                                }

                                u32 DistTabIndex = HuffmanDecode(&DistHuffman, &CompData);
                                png_huffman_entry DistTab = PNGDistExtra[DistTabIndex];
                                u32 Distance = DistTab.Symbol;
                                if(DistTab.BitsUsed)
                                {
                                    u32 ExtraBits = ConsumeBits(&CompData, DistTab.BitsUsed);
                                    Distance += ExtraBits;
                                }

                                u8 *Source = Dest - Distance;
                                Assert((Source + Len) <= DecompressedPixelsEnd);
                                Assert((Dest + Len) <= DecompressedPixelsEnd);
                                Assert(Source >= DecompressedPixels);

                                while(Len--)
                                {
                                    *Dest++ = *Source++;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }

                Assert(Dest == DecompressedPixelsEnd);
                PNGFilterReconstruct(Width, Height, DecompressedPixels, FinalPixels,
                                     CompData.Errors);
            }
        }
    }


    image_u32 Result = {};
    Result.Width = Width;
    Result.Height = Height;
    Result.Pixels = (u32 *)FinalPixels;
    return(Result);
}

internal void
WritePNG(u32 Width, u32 Height, u32 *Pixels, stream *Out)
{
    stream_chunk *StartCRC = 0;
    u32 OutputPixelSize = 4*Width*Height;

    OutCopy(Out, sizeof(PNGSignature), PNGSignature);

    png_ihdr IHDR;
    IHDR.Width = Width;
    IHDR.Height = Height;
    IHDR.BitDepth = 8;
    IHDR.ColorType = 6;
    IHDR.CompressionMethod = 0;
    IHDR.FilterMethod = 0;
    IHDR.InterlaceMethod = 0;
    EndianSwap(&IHDR.Width);
    EndianSwap(&IHDR.Height);

    png_chunk_header ChunkHeader;
    png_chunk_footer ChunkFooter;

    ChunkHeader.Length = sizeof(IHDR);
    ChunkHeader.TypeU32 = 'IHDR';
    EndianSwap(&ChunkHeader.Length);
    EndianSwap(&ChunkHeader.TypeU32);

    OutStructCopy(Out, ChunkHeader);
    StartCRC = Out->Last;
    OutStructCopy(Out, IHDR);
    ChunkFooter.CRC = EndBigCRC(4, StartCRC);
    OutStructCopy(Out, ChunkFooter);

    png_idat_header IDAT;
    IDAT.ZLibMethodFlags = 8;
    IDAT.AdditionalFlags = 29;
    // NOTE(casey): The FCHECK value has to make the IDAT be a multiple of 31,
    // so 29 corrects for the fact that we would be remainder 2 when we set
    // ZLibMethodFlags to 8.

    u32 MaxChunkSize = 65535;

    u32 TotalLength = ((Width*4 + 1)*Height);
    u32 ChunkCount = (TotalLength + MaxChunkSize - 1) / MaxChunkSize;
    if(ChunkCount == 0)
    {
        ChunkCount = 1;
    }

    u32 ChunkOverhead = (sizeof(u8) + sizeof(u16) + sizeof(u16));
    ChunkHeader.Length = (sizeof(IDAT) + (ChunkCount*ChunkOverhead) + TotalLength + sizeof(u32));
    ChunkHeader.TypeU32 = 'IDAT';
    EndianSwap(&ChunkHeader.Length);
    EndianSwap(&ChunkHeader.TypeU32);

    OutStructCopy(Out, ChunkHeader);
    StartCRC = Out->Last;
    OutStructCopy(Out, IDAT);

    adler_32 Adler = BeginAdler32();

    u32 B = 0;
    u32 Y = 0;
    u32 LengthRemaining = TotalLength;
    for(u32 ChunkIndex = 0;
        ChunkIndex < ChunkCount;
        ++ChunkIndex)
    {
        u16 Len = (u16)MaxChunkSize;
        if(Len > LengthRemaining)
        {
            Len = (u16)LengthRemaining;
        }
        u16 NLen = (u16)~Len;
        LengthRemaining -= Len;

        u32 TotalRowLen = (4*Width + 1);
        u8 BFinalType = ((ChunkIndex + 1) == ChunkCount) ? 0x1 : 0x0;
        OutStructCopy(Out, BFinalType);
        OutStructCopy(Out, Len);
        OutStructCopy(Out, NLen);
        while(Len)
        {
            u8 NoFilter = 0;
            u32 RowLen = 1;
            void *Row = &NoFilter;
            if(B > 0)
            {
                RowLen = TotalRowLen - B;
                if(RowLen > Len)
                {
                    RowLen = Len;
                }
                Row = (u8 *)(Pixels + Y*Width) + B - 1;
            }

            OutCopy(Out, RowLen, Row); // TODO(casey): For speed, this could just append a direct pointer!
            Adler32Append(&Adler, RowLen, Row);
            B += RowLen;
            Len -= (u16)RowLen;
            if(B == TotalRowLen)
            {
                B = 0;
                ++Y;
            }
        }
    }

    u32 Adler32 = EndAdler32(&Adler);
    EndianSwap(&Adler32);
    OutStructCopy(Out, Adler32);
    ChunkFooter.CRC = EndBigCRC(4, StartCRC);
    OutStructCopy(Out, ChunkFooter);

    ChunkHeader.Length = 0;
    ChunkHeader.TypeU32 = 'IEND';
    EndianSwap(&ChunkHeader.Length);
    EndianSwap(&ChunkHeader.TypeU32);
    OutStructCopy(Out, ChunkHeader);
    StartCRC = Out->Last;
    ChunkFooter.CRC = EndBigCRC(4, StartCRC);
    OutStructCopy(Out, ChunkFooter);
}