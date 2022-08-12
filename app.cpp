/* 
 *  Copyright harfang_emscripten_c - 2020-2022 Movida Production - Camille Dudognon - Thomas Simonnet
 */

#ifdef EMSCRIPTEN
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>
#include <emscripten/wire.h>
#endif

#include <engine/scene.h>
#include <engine/assets.h>
#include <engine/render_pipeline.h>
#include <engine/forward_pipeline.h>
#include <engine/scene_forward_pipeline.h>
#include <engine/fps_controller.h>
#include <engine/create_geometry.h>

#include <foundation/format.h>
#include <foundation/log.h>
#include <foundation/matrix4.h>
#include <foundation/clock.h>
#include <foundation/projection.h>
#include <platform/input_system.h>
#include <platform/window_system.h>

#include <filesystem>

using namespace hg;

int width, height;
Window* win;

bgfx::ProgramHandle prg;
Model cube_mdl;


void InitScene() {
	// create cube mdl
	bgfx::VertexLayout vs_decl;

	vs_decl.begin();
	vs_decl.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
	vs_decl.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
	vs_decl.end();

	cube_mdl = CreateCubeModel(vs_decl, 1, 1, 1);

	// 
	// load program
	// load shader
	extern unsigned char mdl_vsb[];
	extern unsigned int mdl_vsb_len;

	const auto mem_vs = bgfx::copy(mdl_vsb, mdl_vsb_len);
	const char* vs_name = "default_mdl.vsb";
	auto vs = bgfx::createShader(mem_vs);

	if (!bgfx::isValid(vs))
		warn(format("Failed to load vertex shader '%1'").arg(vs_name));
	else
		bgfx::setName(vs, vs_name);

	extern unsigned char mdl_fsb[];
	extern unsigned int mdl_fsb_len;

	const auto mem_fs = bgfx::copy(mdl_fsb, mdl_fsb_len);
	const char* fs_name = "default_mdl.fsb";
	auto fs = bgfx::createShader(mem_fs);

	if (!bgfx::isValid(fs))
			warn(format("Failed to load fragment shader '%1'").arg(fs_name));
	else
		bgfx::setName(vs, vs_name);

	prg = bgfx::createProgram(vs, fs, true);

	if (!bgfx::isValid(prg))
		warn(format("Failed to create program from shader '%1' and '%2'").arg(vs_name).arg(fs_name));
}

void loop() {
	auto dt = tick_clock();

	int w, h;
	GetWindowClientSize(win, w, h);

	// resize
	if (w != 0 && h != 0 && (w != width || h != height)) {
		width = w;
		height = h;

		bgfx::reset(width, height, BGFX_RESET_MSAA_X8 | BGFX_RESET_VSYNC | BGFX_RESET_FLIP_AFTER_RENDER | BGFX_RESET_FLUSH_AFTER_RENDER | BGFX_RESET_MAXANISOTROPY);

		bgfx::frame();
		bgfx::frame();
	}

	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	bgfx::setViewRect(0, 0, 0, width, height);
	//
	const auto cam = TransformationMat4({ 0.f, 0.f, -2.f }, { 0.f, 0.f, 0.f });

	const auto view = InverseFast(cam);
	const auto proj = ComputePerspectiveProjectionMatrix(0.01f, 100.f, 1.0f, { (float)width / (float)height, 1.f });

	bgfx::setViewTransform(0, to_bgfx(view).data(), to_bgfx(proj).data());

	Mat4 m = TransformationMat4({ 0, 0, 0.f }, { 0.f, time_to_sec_f(get_clock()), 0.f });
	DrawModel(0, cube_mdl, prg, {}, {}, &m);

	bgfx::frame();
	UpdateWindow(win);
}


int main(int argc, char* argv[])
{
	set_log_level(LL_Normal);
	set_log_detailed(false);

	InputInit();
	WindowSystemInit();

#ifndef EMSCRIPTEN
	width = 1600;
	height = 900;
#else
	double w, h;
	emscripten_get_element_css_size("canvas", &w, &h);
	width = w;
	height = h;
	log(format("windows size: %1x%2").arg(width).arg(height).c_str());
#endif

	win = NewWindow(width, height);

#ifndef EMSCRIPTEN
	RenderInit(win, bgfx::RendererType::Direct3D11, nullptr);
#else
	RenderInit(win, bgfx::RendererType::OpenGLES);
#endif
	bgfx::reset(width, height, BGFX_RESET_MSAA_X8 | BGFX_RESET_VSYNC | BGFX_RESET_FLIP_AFTER_RENDER | BGFX_RESET_FLUSH_AFTER_RENDER | BGFX_RESET_MAXANISOTROPY);

	SetWindowTitle(win, std::string("Harfang/Emscripten"));

	InitScene();

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(loop, 0, 1);
#else
	while (!ReadKeyboard("default").key[K_Escape] && IsWindowOpen(win))
		loop();
#endif
}
