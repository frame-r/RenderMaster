#include "pch.h"
#include "core.h"
#include "resource_manager.h"
#include "material_manager.h"
#include "filesystem.h"
#include "camera.h"
#include "console.h"
#include "render.h"
#include "input.h"
#include "main_window.h"
#include "corerender/dx11/dx11corerender.h"

#define RESOURCE_DIR "\\resources"
#define FPS_UPDATE_INTERVAL 0.3f
#define DEFAULT_ROOT_RELATIVE_BIN "../"
Core *_core;

char logBuffer__[5000];

//
static std::chrono::steady_clock::time_point start;

static std::unordered_map<const WindowHandle*, size_t> windowToViewId;
static size_t viewIDCounter{};

int getMsaaSamples(INIT_FLAGS flags);
int getVSync(INIT_FLAGS flags);

uint Core::getNumLines()
{
	return 3;
}

std::string Core::getString(uint i)
{
	switch (i)
	{
		case 0: return "==== Core ====";
		case 1: return "FPS: " + std::to_string(_fpsLazy);
	}
	return "";
}

Core::Core()
{
	fs = new FileSystem;
	resMan = new ResourceManager;
	console = new Console;
	input = new Input;
	render = new Render;
	matManager = new MaterialManager;

	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
}

bool Core::Init(const char* rootPath, const WindowHandle* externHandle, INIT_FLAGS flags)
{
	Log("--------- Core Initialization ---------");

	fs->Init();

	// root path
	if (fs->IsRelative(rootPath))
	{
		workingPath_ = fs->GetWorkingPath("");
		string rootRelativeToWorking_;
		if (strlen(rootPath) == 0)
			rootRelativeToWorking_ = DEFAULT_ROOT_RELATIVE_BIN;
		else
			rootRelativeToWorking_ = rootPath;
		rootPath_ = fs->GetWorkingPath(rootRelativeToWorking_.c_str());; 
	} else
		rootPath_ = rootPath;

	if (!fs->DirectoryExist(rootPath_.c_str()))
		return false;

	assert(rootPath_.size() > 0);

	// data path
	dataPath_ = rootPath_ + RESOURCE_DIR;

	const bool createWindow = (flags & INIT_FLAGS::WINDOW_FLAG) != INIT_FLAGS::EXTERN_WINDOW && !externHandle;

	console->Init(createWindow);

	Log("Root path:     %s", rootPath_.c_str());
	Log("Data path:     %s", dataPath_.c_str());
	Log("Working path:  %s", workingPath_.c_str());

	AddProfilerCallback(this);

	WindowHandle handle;
	if (createWindow)
	{
		window = new MainWindow(sMainLoop);
		window->AddMessageCallback(sMessageCallback);
		window->Create();
		handle = *window->handle();
	} else
		handle = *externHandle;

	MSAASamples = getMsaaSamples(flags);
	VSync = getVSync(flags);

	if (!initCoreRender(&handle))
		return false;

	resMan->Init();
	input->Init();
	matManager->Init();
	render->Init();

	Log("Core Inited");

	return true;
}

bool Core::initCoreRender(WindowHandle *handle)
{	
	coreRender = new DX11CoreRender;
	return coreRender->Init(handle, MSAASamples, VSync);
}

void Core::setWindowCaption(int is_paused, int fps)
{
	if (!window)
		return;

	std::wstring title = std::wstring(L"Test");

	if (is_paused)
		title += std::wstring(L" [Paused]");
	else
	{
		string fps_str = std::to_string(fps);
		title += std::wstring(L" [") + std::wstring(fps_str.begin(), fps_str.end()) + std::wstring(L"]");
	}

	window->SetCaption(title.c_str());
}

void Core::messageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void* pData)
{
	switch (type)
	{
	case WINDOW_MESSAGE::SIZE:
		if (coreRender)
			coreRender->SetViewport(param1, param2);
		break;

	case WINDOW_MESSAGE::WINDOW_UNMINIMIZED:
		if (window)
			window->SetPassiveMainLoop(0);
		if (console)
			console->Show();
		break;

	case WINDOW_MESSAGE::WINDOW_MINIMIZED:
		if (window)
			window->SetPassiveMainLoop(1);
		if (console)
			console->Hide();
		break;

	case WINDOW_MESSAGE::WINDOW_REDRAW:
		mainLoop();
		break;

	case WINDOW_MESSAGE::APPLICATION_ACTIVATED:
		if (window)
			window->SetPassiveMainLoop(0);
		break;

	case WINDOW_MESSAGE::APPLICATION_DEACTIVATED:
		if (window)
		{
			setWindowCaption(1, 0);
			window->SetPassiveMainLoop(1);
		}
		break;

	case WINDOW_MESSAGE::WINDOW_CLOSE:
		//if (_pConsoleWindow)
		//	_pConsoleWindow->BringToFront();
		break;

	default:
		break;
	}
}

void Core::sMessageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void* pData)
{
	_core->messageCallback(type, param1, param2, pData);
}

void Core::engineUpdate()
{
	static float accum = 0.0f;

	std::chrono::duration<float> _durationSec = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start);

	_dt = _durationSec.count();
	_fps = static_cast<int>(1.0f / _dt);

	accum += _dt;

	if (accum > FPS_UPDATE_INTERVAL)
	{
		accum = 0.0f;
		setWindowCaption(0, _fps);
		_fpsLazy = _fps;
	}

	start = std::chrono::steady_clock::now();

	coreRender->Update();
	render->Update();
	input->Update();
	resMan->Update(_dt);
	onUpdate.Invoke(_dt);

	_frame++;
}

void Core::mainLoop()
{
	engineUpdate();

	if (camera)
	{
		uint w, h;
		CORE_RENDER->GetViewport(&w, &h);
		float aspect = (float)w / h;

		mat4 ProjMat = camera->GetProjectionMatrix(aspect);
		mat4 ViewMat = camera->GetViewMatrix();

		render->RenderFrame(0, ViewMat, ProjMat, nullptr, 0);
		CORE_RENDER->SwapBuffers();
	}else
	{
		CORE_RENDER->Clear();
		CORE_RENDER->SwapBuffers();
	}
}

auto DLLEXPORT Core::ManualUpdate() -> void
{
	engineUpdate();
}

auto DLLEXPORT Core::ManualRenderFrame(const WindowHandle *externHandle, const mat4& ViewMat, const mat4& ProjMat, Model** wireframeModels, int modelsNum) -> void
{
	CORE_RENDER->MakeCurrent(externHandle);

#ifdef WIN32
	RECT r;
	GetClientRect(*externHandle, &r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;

	if (w < 1 || h < 1)
		return;

	CORE_RENDER->SetViewport(w, h);
#else
	assert(false); // not impl
#endif // WIN32

	auto it = windowToViewId.find(externHandle);

	if (it == windowToViewId.end())
		windowToViewId[externHandle] = viewIDCounter++;

	size_t viewID = windowToViewId[externHandle];

	render->RenderFrame(viewID, ViewMat, ProjMat, wireframeModels, modelsNum);
}

auto DLLEXPORT Core::AddProfilerCallback(IProfilerCallback * c) -> void
{
	profilerCallbacks.push_back(c);
}

auto DLLEXPORT Core::RemoveProfilerCallback(IProfilerCallback * c) -> void
{
	profilerCallbacks.erase(std::remove(profilerCallbacks.begin(), profilerCallbacks.end(), c), profilerCallbacks.end());
}

void Core::sMainLoop()
{
	_core->mainLoop();
}

void Core::Free()
{
	RemoveProfilerCallback(this);

	render->Free();
	matManager->Free();
	input->Free();
	resMan->Free();
	freeCoreRender();

	if (window)
		window->Destroy();

	Log("--------- Core Free ---------");

	console->Free();

	fs->Free();
}

auto DLLEXPORT Core::Start(Camera *cam) -> void
{
	camera = cam;
	onInit.Invoke();
	start = std::chrono::steady_clock::now();

	if (window)
	{
		CORE_RENDER->MakeCurrent(window->handle());

		int w, h;
		window->GetClientSize(w, h);

		CORE_RENDER->SetViewport(w, h);
		window->StartMainLoop();
	}
}

//void Core::ReloadCoreRender()
//{
//	resMan->Reload();
//	freeCoreRender();
//	assert(initCoreRender());
//}

Core::~Core()
{
	delete matManager;
	matManager = nullptr;

	delete render;
	render = nullptr;

	delete input;
	input = nullptr;

	delete resMan;
	resMan = nullptr;

	delete coreRender;
	coreRender = nullptr;

	delete window;
	window = nullptr;

	delete console;
	console = nullptr;

	delete fs;
	fs = nullptr;
}

void Core::freeCoreRender()
{
	if (!coreRender)
		return;

	coreRender->Free();

	delete coreRender;
	coreRender = nullptr;
}

DLLEXPORT Core* GetCore()
{
	Core* c = new Core();
	_core = c;
	return c;
}
DLLEXPORT void ReleaseCore(Core* c)
{
	if (c && c == _core)
	{
		delete _core;
		_core = nullptr;
	}
}

int getMsaaSamples(INIT_FLAGS flags)
{
	if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_2X) return 2;
	else if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_4X) return 4;
	else if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_8X) return 8;
	else if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_16X) return 16;
	else if ((flags & INIT_FLAGS::MSAA_FLAG) == INIT_FLAGS::MSAA_32X) return 32;
	return 1;
}

int getVSync(INIT_FLAGS flags)
{
	if (int(flags & INIT_FLAGS::VSYNC_FLAG))
		return (flags & INIT_FLAGS::VSYNC_FLAG) == INIT_FLAGS::VSYNC_ON;
	return 0;
}


