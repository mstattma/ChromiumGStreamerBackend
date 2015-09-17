Chromium GStreamer Backend
==========================

[Chromium](https://www.chromium.org/Home), [GStreamer](http://gstreamer.freedesktop.org/features/), [MediaProcess](#media-process-overview), [Sandbox](#media-process-sandbox), [MSE](#mse), [GstPlayer](https://github.com/sdroege/gst-player/commits/master), [GstGL](#media-process-stack), [GstChromiumHttpSrc](#media-process-stack), [Build](#build), [Tips](#tips), [Maintenance](#maintenance), [Issues](#issues), [Roadmap](#roadmap)

### Current branching point from official chromium/src  ###
85c2c2c4b76b0e6112dc03f640772bf164a548b9 (Wed Sep 16)

### Project description ###
This is an experimental project that aims to have GStreamer as media player in Chromium browser.
We introduced a dedicated [Media Process](#media-process-overview) that maintains GStreamer pipelines.
The Media Process is [sandboxed](#media-process-sandbox) with same level as [GPU Process](https://code.google.com/p/chromium/wiki/LinuxSandboxing).
[GstPlayer](http://gstreamer.freedesktop.org/projects/gstplayer.html) is used to construct and to maintain GStreamer pipelines. 
Each [HTML5 video tag](http://www.w3schools.com/html/html5_video.asp) is backed by a GStreamer pipeline that lives in the Media Process. 

### Licence ###
Same as official chromium/src source code: [here](https://chromium.googlesource.com/chromium/src.git/+/master/LICENSE).

### Warning ###
Please do not take this project as ready to use and full featured. There is a lot of work to do to reach that point.
We wanted to opensource our researches as soon as we could play basic videos with sandbox activated.
We want to discuss the architecture with anyone interested in this project.
We are open to any change related to the design or to the implementation.
There are choices we made that need to be discussed or re-considered, see [issues](#issues) and [roadmap](#roadmap).

### Status ###
20/07/2015: Can play classic html5 videos and Youtube. It supports MSE without seeking for now like WebKitGTK.

``` bash
# Progressive streaming:
http://www.w3schools.com/html/mov_bbb.ogg
http://www.w3schools.com/html/html5_video.asp

<video width="560" height="320" controls>
    <source src="http://video.webmfiles.org/big-buck-bunny_trailer.webm" type="video/webm">
</video>

# Adaptive streaming
<!-- HLS -->
<video width="480" height="360" controls>
    <source src="http://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8" type="application/x-mpegURL">
</video>

```

### List of modified and added files ###
21/08/2015: (just to give an idea of the delta from official chromium/src)  
53 files modified, 642 insertions(+), 13 deletions(-)  
51 files added, 9187 insertions(+)  
git diff --diff-filter=AM --stat sha-1 HEAD  

### Build ###
Start from a working build of official [Chromium Browser](https://code.google.com/p/chromium/wiki/LinuxBuildInstructions).
Then refer to this [section](#build-steps) to build the Chromium GStreamer Backend.

### Media Process overview ###
There is only one Media Process no matter the number of video tags in the same pages or the number of pages (~ tabulations).
GStreamer is almost only used through the a high level API [GstPlayer](https://github.com/sdroege/gst-player/commits/master) (ex: gst_player_new. gst_player_play, gst_player_pause, gst_player_seek)
It reduces GStreamer code lines a lot and it avoids doing mistakes when using more low level GStreamer API.
Exception for the video rendering part because GstGL needs to be setup to use GPU Process. In short we pass an external get_process_addr to GstGL.
Indeed the Media Process does not load OpenGL libraries; it uses chromium API to forward OpenGL calls to GPU Process which is the only sandboxed
process that is allowed to load GL libraries. 
Exception also for the [GstChromiumHttpSrc](#media-process-stack). It is a GStreamer element that wraps chromium API to load an url.
![](https://github.com/Samsung/ChromiumGStreamerBackend/blob/master/images/chromium_media_process_overview.png?raw=true)

__  

### Media Process stack ###
**GstGLContextGPUProcess:**
A new backend that allows to build the GstGL’s vtable from a given get_proc_addr, i.e. chromium::gles2::GetGLFunctionPointer.  See [gpu-accelerated-compositing-in-chrome](https://www.chromium.org/developers/design-documents/gpu-accelerated-compositing-in-chrome) “From the client's perspective, an application has the option to either write commands directly into the command buffer or use the GL ES 2.0 API via a client side library that we provide which handles the serialization behind the scenes. Both the compositor and WebGL currently use the GL ES client side library for convenience. On the server side, commands received via the command buffer are converted to calls into either desktop OpenGL or Direct3D via ANGLE.” The Media Process has its own connection to the GPU Process and it uses this second mechanism to forward gl calls it. In the Media Process all gl calls happen in the GLThread.

**MediaPlayerGStreamer:** Created in the Media Process for each video tag. It receives player commands (load, play, pause, seek) from the WebMediaPlayerGStreamer that lives a Renderer Process. It uses the gst-player library to play a stream. In the handler of glimagesink “client-draw” signal, the current gltexture is wrapped using  glGenMailboxCHROMIUM/glProduceTextureDirectCHROMIUM (see mailbox in [gpu-accelerated-compositing-in-chrome](https://www.chromium.org/developers/design-documents/gpu-accelerated-compositing-in-chrome)). The mailbox’s name for each gltexture is forwarded to WebMediaPlayerGStreamer through IPC (MediaChannel). But this can be avoided by making MediaPlayerGStreamer inherits from cc::VideoFrameProvider in order to avoid sending the gltexture id to Renderer Process (to be sync with the web compositor). Indeed according to [oop-iframes-rendering](https://www.chromium.org/developers/design-documents/oop-iframes) and [oop-iframes-rendering](https://www.chromium.org/developers/design-documents/oop-iframes/oop-iframes-rendering) it is theoretically possible to make our Media Process be a kind of SubFrame renderer. By registering a cc::VideoLayer in the MediaProcess, the ubercompositor will do the synchronisation with the Frame in the RendererProcess. We suspect it is all about having a cc::VideoFrameProvider::Client in MediaProcess.

**WebMediaPlayerGStreamer:** Created in a Renderer Process for each video tag. It inherits from blink::WebMediaPlayer and cc::VideoFrameProvider. The gltexture is retrieved from a mailbox name and wrapped into a media::VideoFrame using media::VideoFrame::WrapNativeTexture. At any time the compositing engine is free to call GetCurrentFrame(). This late part can be avoided, see MediaPlayerGStreamer description.

**GstChromiumHttpSrc:** GStreamer element that wraps the chromium media::BufferedDataSource and instantiate a content::WebURLLoader . It was not possible to re-use WebKitWebSrc because some interfaces are missing or has been removed like PlatformMediaResourceLoader. It uses some parts of WebKitWebSrc though but not sure if it is necessary. Also It does not use appsrc comparing to WebKitWebSrc. It is more similar to filesrc due to media::BufferedDataSource design. That’s clearly an area to improve, maybe we can get rid of media::BufferedDataSource and implement missing interface in order to re-use WebKitWebSrc. Indeed internally it uses ResourceDispatcher, see [multi-process-resource-loading](https://www.chromium.org/developers/design-documents/multi-process-resource-loading). Or maybe we can design a complete new element and interfaces that could be shared between blink and WebKit.

![](https://github.com/Samsung/ChromiumGStreamerBackend/blob/master/images/chromium_media_process_stack.png?raw=true)

__  

### Media Process sandbox ###
It has limited access to resources. For example it cannot create network sockets. It has to use
chromium API in order to ask the Browser Process to create a socket. This is similar to Renderer Process.
Also all system calls are filters using linux kernel feature Seccomp-BPF (link).

We opted for similar design as GPU Process sandbox, using Seccomp-BPF, see [chromium sandbox doc](https://code.google.com/p/chromium/wiki/LinuxSandboxing). All the following sequence is existing sequence for GPU Process. In some sort the Media Process just defines its own policy for the system calls, see [bpf_media_policy_linux.cc](https://github.com/Samsung/ChromiumGStreamerBackend/blob/45.0.2440.3_gst/content/common/sandbox_linux/bpf_media_policy_linux.cc). A main difference is that the Media Process allow loading GStreamer plugins dynamically. But the list can be restricted and open is done in
the brocker process.

In Media Process main the sandbox is setup before any thread creation. Also there is a preSansbox step where it is possible to dlopen some particular resources. A routine is executed in one go for each system call. In this loop it is decided to allow the system call number, to refuse it or to add a handler for emulation. All of this is setup through Seccomp-BPF. A trap handler (SECCOMP_RET_TRAP), is installed for open and access. Then when starting the sandbox the process forks itself (see chromium/src/sandbox/linux/syscall_broker/broker_process ::BrokerProcess::Init). The child becomes the so called Media Broker Process which instantiates a BrokerHost. The Media Process instantiates a BrokerProcessClient. When a open or access happens it sends a cachable SIGSYS. In this trap handler the BrokerClient emulates the open/access call. It actually asks the BrokerProcessHost, through IPC, to do the real call. Depending on the policy that has been setup the path is allowed or rejected. The return value is passed back to the BrokerClient, i.e. the Media Process, for usage if the call has been accepted.

![](https://github.com/Samsung/ChromiumGStreamerBackend/blob/master/images/chromium_media_process_sandbox.png?raw=true)

__  

### Media Process IPC ###

IPC between media process and render process is maintained via browser process. Browser process MediaProcessHost is initialized with a connection to media process during the browser start up. Later, WebMediaPlayerDispatcher is used when the render process decides to create a media player for html media element. A first message is sent to media process. It establishes a direct IPC channel from render process to media process is created.

Establish media channel request comes from WebMediaPlayerGStreamer to RenderThreadImpl (main render thread), which creates MediaChannelHost for render process and sends establish channel message to MediaMessageFilter in browser process, which redirects the message through MediaProcessHost to media process.

Media process, upon receiving the request, establishes the channel (MediaChannel) with the help of MediaChannelFilter and sends back the channel handler which is passed then by browser process to the render process media player and used to further IPC communication between render process and media process. Any messages from media process to render process get filtered by MediaChannelHost and get dispatched by WebMediaPlayerMessageDispatcher to the right WebMediaPlayergStreamer instance.

For any messages from WebMediaPlayerGStreamer  WebMediaPlayerMessageDispatcher dispatches messages via MediaChannelHost to MediaChannel in media process. MediaChannel maps the message to the associated media player and call the callback.

![](https://github.com/Samsung/ChromiumGStreamerBackend/blob/master/images/chromium_media_process_ipc.png?raw=true)

__  

### MSE ###

[Media Source Extensions specification](https://w3c.github.io/media-source/).

When load_type param of blink::WebMediaPlayer::load is equal to LoadTypeMediaSource the url is formated to start with mediasourceblob://.

It allows GstPlayer's pipeline to select GstChromiumMediaSrc. This GstBin adds and removes appsrc elements when receiving AddSourceBuffer and RemoveSourceBuffer.

Encoded data is currently sent through IPC message. We can later consider using shared memory but it seems efficient enough like.
And it is also what Android does.

Currently seeking is not implemented but WebKitGTK does not support it too.
Also it exists on-going work somewhere. And it should be easier to handle it with future multiappsrc element.

![](https://github.com/Samsung/ChromiumGStreamerBackend/blob/master/images/chromium_media_process_mse.png?raw=true)

__  

### Build steps ###
``` bash
# GStreamer
gstreamer >= 1.5.2 is required because of GstGL dependency. Required GstGL feature is not present in 1.5.1
GstPlayer: git clone https://github.com/sdroege/gst-player.git

# clone official chromium repositories
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git (then add it in front of your PATH)
git clone https://chromium.googlesource.com/chromium/chromium

# fetch and sync all sub repos. (run all of this from chromium directory, not chromium/src)
fetch --nohooks chromium # only the very first time
git rebase-update
gclient sync # see proxy section
gclient runhooks # see proxy section
build/install-build-deps.sh

# go to chromium/src
cd src
build/install-build-deps.sh

# checkout GStreamer Chromium Backend
git remote add github_gstbackend https://github.com/Samsung/ChromiumGStreamerBackend
git fetch github_gstbackend
git checkout -b gstbackend --track github_gstbackend/master
Then re-run the 2 previous blocks (rebase, sync, deps)

# or replay it on top of your working branch (it creates a detached branch)
echo "$(git rev-parse 2553ff05b802a94ef281e647874d37941eefd154~1) 64334b71ec2d2e4a8bf0cf5ca7e2dd18d90db0cb" >> .git/info/grafts
git merge-base origin/master github_samsung/master
git rebase --onto my_working_branch origin/master github_samsung/master

# configure GYP (equivalent to ./autogen.sh or ./configure)
GYP_DEFINES="linux_use_debug_fission=0 linux_use_bundled_binutils=0 clang=0 proprietary_codecs=1" build/gyp_chromium -D component=shared_library

# build (we recommend to use icecc. We can do a fresh build of chromium browser in less than 10 min in release mode)
ninja -C out/Release chrome chrome_sandbox -jN # if icecc -j60

# Run without any sandbox
./out/Release/chrome --disable-media-sandbox --disable-setuid-sandbox http://www.w3schools.com/html/mov_bbb.ogg

# Run with sandbox
CHROME_DEVEL_SANDBOX=out/Release/chrome_sandbox ./out/Release/chrome http://www.w3schools.com/html/mov_bbb.ogg
```

### Proxy ###
``` bash
# touch ~/.boto and copy the 3 following lines inside ~/.boto
[Boto]
proxy = _proxy_ip
proxy_port = _proxy_port

# run
NO_AUTH_BOTO_CONFIG=~/.boto gclient sync
NO_AUTH_BOTO_CONFIG=~/.boto gclient runhooks
```

### Maintenance ###
``` bash
# We have not managed to push original history of chromium because:
# "remote: error: File chrome_frame/tools/test/reference_build/chrome_frame/chrome_dll.pdb is
#   124.32 MB; this exceeds GitHub's file size limit of 100.00 MB"
# This file is not there anymore but it was there in the past.
# Even using tools like "bfg --strip-blobs-bigger-than 50M" it will change all the next SHA
# from the point it strips a file.
# Also there no official chromium/src repo on github that we could fork.

# solution: truncate history to a point where there is not file bigger than 100.00 MB in order
# to push to github.
# We use "git replace" to allow rebasing between our truncated branch and original branch.

# fake our branch point because we truncated the history
git replace 2b865e2bf4c6eefeaca882fe4ba9f561ca2bbed0 85c2c2c4b76b0e6112dc03f640772bf164a548b9

# replay chromium original upstream commits on top of our branch with truncated history
git checkout NEW_ORIGIN_SHA
git checkout -b master_new
git rebase 2b865e2bf4c6eefeaca882fe4ba9f561ca2bbed0
git replace $(git rev-parse HEAD) NEW_ORIGIN_SHA

# replay gst backend
git checkout -b gstbackend --track github_gstbackend/master
git rebase master_new
git push github_gstbackend gstbackend:master --force

# replace are located in .git/refs/replace/

```

### Tips ###
``` bash

# disable nacl to reduce build time
Insert disable_nacl=1 to GYP_DEFINES, see build steps.

# command lines options for ./out/Release/chrome
A list of all command line switches is available here: http://peter.sh/experiments/chromium-command-line-switches/

# disable gpu process and particular sandbox at runtime
CHROME_SANDBOX_DEBUGGING=1 CHROME_DEVEL_SANDBOX=out/Release/chrome_sandbox ./out/Release/chrome --allow-sandbox-debugging --disable-render-sandbox --disable-hang-monitor  http://localhost/test/mov_bbb.ogg
CHROME_DEVEL_SANDBOX=out/Release/chrome_sandbox ./out/Release/chrome --disable-render-sandbox --disable-gpu --disable-hang-monitor about:blank

# run chromium in debug mode
CHROME_DEVEL_SANDBOX=out/Debug/chrome_sandbox ./out/Debug/chrome http://www.w3schools.com/html/mov_bbb.ogg
CHROME_DEVEL_SANDBOX=out/Debug/chrome_sandbox ./out/Debug/chrome --media-startup-dialog --allow-sandbox-debugging about:blank

# gdb
./out/Release/chrome --disable-media-sandbox --disable-setuid-sandbox --allow-sandbox-debugging --media-launcher='xterm -title renderer -e gdb --args'
CHROME_DEVEL_SANDBOX=out/Release/chrome_sandbox ./out/Release/chrome --disable-render-sandbox --disable-gpu --disable-hang-monitor --media-startup-dialog --allow-sandbox-debugging  about:blank

# disable all processes, i.e. make all processes running as in-process mode in browser process.
--single-process

# logs in debug mode
DVLOG(level) in debug mode, DVLOG(1)
VLOG(level) in release mode, VLOG(1)
--enable-logging=stderr --v=N (level)
--enable-logging=stderr --v=1

# attach debugger
pass --media-startup-dialog to pause the media process at startup and print the pid
Then attach a debugger: gdb -p the_pid
In gdb type: signal SIGUSR1 to resume pause and type c to continue

# indent code
Just run: git cl format, to indent latest commit.
```

### Issues ###
* Pulse audio client crashes when filtering system calls.
* GstChromiumHttpSrc design has to be discussed regarding media::BufferedDataSource usage.
* Registering ResourceMessageFilter in MediaProcessHost has to be discussed regarding setting-up its content::StoragePartition.
* Random crashes regarding seeking. We suspect it highly depends on GstChromiumHttpSrc.
* chrome://flags, kEnableGStreamerMediaBackend dynamic flag sometimes make the browser to crash when enabling, disabling.
* Playing video from disk (file://) shows black frame.
* Media Process can work as "In Process" mode when passing --in-process-media command line argument but it currently crashes.

### Roadmap ###
* Add EME support.
* (On going) Zero-copy, see [mesa/vaapi/nouveau](https://bugs.freedesktop.org/show_bug.cgi?id=89969), [mesa/vaapi/dmabuf](https://bugs.freedesktop.org/show_bug.cgi?id=92022), [gstreamer-vaapi_dmabuf](https://bugzilla.gnome.org/show_bug.cgi?id=755072),
[glimagesink/dmabuf](https://bugzilla.gnome.org/show_bug.cgi?id=743345), [chromium/dmabuf](https://chromium.googlesource.com/chromium/src/+/194331b394178f526b37c51ba0f663e33f46431a)
* Add seeking support for MSE. Need to review and try [multiappsrc](https://bugzilla.gnome.org/show_bug.cgi?id=725187).
* Add support for gn build. (we only support gyp for now)
* For development, pull and build GStreamer locally like it is done for WebKit using jhbuild. (For now we use system wide GStreamer)
* Use ubercompositor in Media Process

