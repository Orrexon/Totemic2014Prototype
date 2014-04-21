#include <minwindef.h>
// super hacks because VS6 headers don't have this crap
// this is all copied from VS.NET's winuser.h

#if(_WIN32_WINNT < 0x0501)

#if !defined(_USER32_)
#define WINUSERAPI DECLSPEC_IMPORT
#else
#define WINUSERAPI
#endif

/*
* Raw Input request APIs
*/
typedef struct tagRAWINPUTDEVICE {
	USHORT usUsagePage; // Toplevel collecti2277on UsagePage
	USHORT usUsage;     // Toplevel collection Usage
	DWORD dwFlags;
	HWND hwndTarget;    // Target hwnd. NULL = follows keyboard focus
} RAWINPUTDEVICE, *PRAWINPUTDEVICE, *LPRAWINPUTDEVICE;

typedef CONST RAWINPUTDEVICE* PCRAWINPUTDEVICE;

#define RIDEV_NOLEGACY          0x00000030

DECLARE_HANDLE(HRAWINPUT);

typedef struct tagRAWINPUTDEVICELIST {
	HANDLE hDevice;
	DWORD dwType;
} RAWINPUTDEVICELIST, *PRAWINPUTDEVICELIST;

// typedef hacks

typedef
//  WINUSERAPI
BOOL
//  WINAPI
(WINAPI *RegisterRawInputDevices_t)(
IN PCRAWINPUTDEVICE pRawInputDevices,
IN UINT uiNumDevices,
IN UINT cbSize);

typedef
//  WINUSERAPI
UINT
//  WINAPI
(WINAPI *GetRawInputData_t)(
IN HRAWINPUT    hRawInput,
IN UINT         uiCommand,
OUT LPVOID      pData,
IN OUT PUINT    pcbSize,
IN UINT         cbSizeHeader);

typedef
//  WINUSERAPI
UINT
//  WINAPI
(WINAPI *GetRawInputDeviceList_t)(
OUT PRAWINPUTDEVICELIST pRawInputDeviceList,
IN OUT PUINT puiNumDevices,
IN UINT cbSize);

#define WM_INPUT    0x00FF

#define RID_INPUT   0x10000003

typedef struct tagRAWINPUTHEADER {
	DWORD dwType;
	DWORD dwSize;
	HANDLE hDevice;
	WPARAM wParam;
} RAWINPUTHEADER, *PRAWINPUTHEADER, *LPRAWINPUTHEADER;

typedef struct tagRAWMOUSE {
	/*
	* Indicator flags.
	*/
	USHORT usFlags;

	/*
	* The transition state of the mouse buttons.
	*/
	union {
		ULONG ulButtons;
		struct  {
			USHORT  usButtonFlags;
			USHORT  usButtonData;
		};
	};

	/*
	* The raw state of the mouse buttons.
	*/
	ULONG ulRawButtons;

	/*
	* The signed relative or absolute motion in the X direction.
	*/
	LONG lLastX;

	/*
	* The signed relative or absolute motion in the Y direction.
	*/
	LONG lLastY;

	/*
	* Device-specific additional information for the event.
	*/
	ULONG ulExtraInformation;

} RAWMOUSE, *PRAWMOUSE, *LPRAWMOUSE;


typedef struct tagRAWKEYBOARD {
	/*
	* The "make" scan code (key depression).
	*/
	USHORT MakeCode;

	/*
	* The flags field indicates a "break" (key release) and other
	* miscellaneous scan code information defined in ntddkbd.h.
	*/
	USHORT Flags;

	USHORT Reserved;

	/*
	* Windows message compatible information
	*/
	USHORT VKey;
	UINT   Message;

	/*
	* Device-specific additional information for the event.
	*/
	ULONG ExtraInformation;


} RAWKEYBOARD, *PRAWKEYBOARD, *LPRAWKEYBOARD;

typedef struct tagRAWHID {
	DWORD dwSizeHid;    // byte size of each report
	DWORD dwCount;      // number of input packed
	BYTE bRawData[1];
} RAWHID, *PRAWHID, *LPRAWHID;

typedef struct tagRAWINPUT {
	RAWINPUTHEADER header;
	union {
		RAWMOUSE    mouse;
		RAWKEYBOARD keyboard;
		RAWHID      hid;
	} data;
} RAWINPUT, *PRAWINPUT, *LPRAWINPUT;


#define RIM_TYPEMOUSE       0
#define RIM_TYPEKEYBOARD    1
#define RIM_TYPEHID         2

#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_VIRTUAL_DESKTOP    0x02  // the coordinates are mapped to the virtual desktop
#define MOUSE_ATTRIBUTES_CHANGED 0x04  // requery for mouse attributes

#define RI_MOUSE_LEFT_BUTTON_DOWN   0x0001  // Left Button changed to down.
#define RI_MOUSE_LEFT_BUTTON_UP     0x0002  // Left Button changed to up.
#define RI_MOUSE_RIGHT_BUTTON_DOWN  0x0004  // Right Button changed to down.
#define RI_MOUSE_RIGHT_BUTTON_UP    0x0008  // Right Button changed to up.
#define RI_MOUSE_MIDDLE_BUTTON_DOWN 0x0010  // Middle Button changed to down.
#define RI_MOUSE_MIDDLE_BUTTON_UP   0x0020  // Middle Button changed to up.

#define RI_MOUSE_BUTTON_1_DOWN      RI_MOUSE_LEFT_BUTTON_DOWN
#define RI_MOUSE_BUTTON_1_UP        RI_MOUSE_LEFT_BUTTON_UP
#define RI_MOUSE_BUTTON_2_DOWN      RI_MOUSE_RIGHT_BUTTON_DOWN
#define RI_MOUSE_BUTTON_2_UP        RI_MOUSE_RIGHT_BUTTON_UP
#define RI_MOUSE_BUTTON_3_DOWN      RI_MOUSE_MIDDLE_BUTTON_DOWN
#define RI_MOUSE_BUTTON_3_UP        RI_MOUSE_MIDDLE_BUTTON_UP

#define RI_MOUSE_BUTTON_4_DOWN      0x0040
#define RI_MOUSE_BUTTON_4_UP        0x0080
#define RI_MOUSE_BUTTON_5_DOWN      0x0100
#define RI_MOUSE_BUTTON_5_UP        0x0200

#endif
