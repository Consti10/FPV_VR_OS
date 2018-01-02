# FPV_VR_2018
Latest version of FPV_VR

I created a new repository because this is more a re-write than an update.

Changelog from version 3.3 (2017) to 4.0 (2018).

On the one hand, from the VR OpenGL engine to parsing and decoding, all core functionalities have been re-written in plain cpp for less, more consistent latency and higher performance especially on older devices. This took me almost half a year, so I’m really sorry that I couldn't publish an update for such a long time. 
On the other hand, I also added a lot of new features and improvements, that list as follow:
New OSD GUI:
•	Higher quality and better readable fonts 
•	Improved  2D height ladder
•	New 2D compass ladder with home arrow
•	Added Flight time text element
Lower latency:
•	Less parsing/ decoding latency
•	Less latency in monoscope rendering on all android versions (let the HW composer do the work when not doing side-by-side rendering)
•	New settings to reduce latency and improve visual quality in side-by-side mode on different android versions.
Android 6 and below: 
	reduced latency slightly, render with more than 60fps. (Disable 60fps caps)
	Improved option to enable MSAA.
Android 7 and higher: 
	“Disable VSYNC” Depending on the HW, this new feature reduces video latency a lot (10-20 %). It should be available on most android 7 devices (especially on flagship phones like the galaxy s8 ) and has less HW requirements than SuperSync. It’s only disadvantage is that this results in some small Imperfections In the image (tearing).
	“SuperSync” Previous “Synchronous front buffer rendering”: As low/lower latency than disabling VSYNC, no tearing. Supported on daydream-ready devices and the Nexus 5X.
•	With a low-persistence screen (AMOLED, daydream-ready) and SuperSync btw. no VSYNC, the video latency using a VR headset and a daydream-ready phone might even be lower than with some fpv goggles, since some “traditional” fpv goggles introduce a significant amount of latency in HDMI mode.
Easier to use:
•	Automatic check of hardware capabilities 
•	No more decoder “fixes” 
•	Improved  “connect to ez-wifibroadcast” gui : automatically detect wifi/usb connection,
display received video and telemetry data
•	Restructured settings
•	Less crashes, less cpu usage when running in the background
