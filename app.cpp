/* 
 *  Copyright harfang_emscripten_c - 2020-2022 Movida Production - Camille Dudognon - Thomas Simonnet
 */

//#ifdef EMSCRIPTEN
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>
#include <emscripten/wire.h>
//#endif

#include <engine/scene.h>
#include <engine/assets.h>
#include <engine/render_pipeline.h>
#include <engine/forward_pipeline.h>
#include <engine/scene_forward_pipeline.h>
#include <engine/fps_controller.h>

#include <foundation/format.h>
#include <foundation/log.h>
#include <foundation/clock.h>
#include <platform/input_system.h>
#include <platform/window_system.h>

#include <filesystem>

int width, height;
hg::Node node, camera;
hg::Window* win;
int current_frame;
std::filesystem::path exe_path;
hg::Scene scene;
hg::PipelineResources res;
hg::ForwardPipeline pipeline;
hg::Keyboard keyboard;
hg::Mouse mouse;


// callback to load file
bool is_downloading;
void progress_load_zip(unsigned, void *, int p) {
	float loading_value = p / 100.f;
	hg::log(hg::format("Loading progress: %1").arg(p));
}
void error_load_zip(unsigned, void *, int err) {
	hg::log(hg::format("Loading error: %1").arg(err));
}

void load_zip(unsigned, void *, const char *file_name) {
	hg::log(hg::format("Loading file: %1").arg(file_name));
	hg::AddAssetsPackage(file_name);
	is_downloading = true;
	
	// init scene
	hg::LoadSceneContext ctx;
	hg::LoadSceneFromAssets("scene/scene.scn", scene, res, hg::GetForwardPipelineInfo(), ctx);

	node = scene.GetNode("Sketchfab_model");
	camera = scene.GetNode("Camera");
}

void GetAssetsPackage(std::string s){
	is_downloading = false;
	emscripten_async_wget2(s.c_str(), s.c_str(), "GET", "", nullptr, &load_zip, &error_load_zip, &progress_load_zip);
}

void loop() {
	auto dt = hg::tick_clock();

	int w, h;
	hg::GetWindowClientSize(win, w, h);

	// resize
	if (w != 0 && h != 0 && (w != width || h != height)) {
		width = w;
		height = h;

		bgfx::reset(width, height, BGFX_RESET_MSAA_X8 | BGFX_RESET_VSYNC | BGFX_RESET_FLIP_AFTER_RENDER | BGFX_RESET_FLUSH_AFTER_RENDER | BGFX_RESET_MAXANISOTROPY);

		current_frame = bgfx::frame();
		current_frame = bgfx::frame();
	}

	keyboard.Update();
	mouse.Update();
                    
	auto cam_pos = camera.GetTransform().GetPos();
	auto cam_rot = camera.GetTransform().GetRot();
	auto cam_speed = 0.3f;
	hg::FpsController(keyboard, mouse, cam_pos, cam_rot, cam_speed, dt);
	camera.GetTransform().SetPos(cam_pos);
	camera.GetTransform().SetRot(cam_rot);

	scene.Update(dt);

	bgfx::ViewId view_id = 0;
	hg::SceneForwardPipelinePassViewId views;
	hg::SubmitSceneToPipeline(view_id, scene, hg::Rect<int>(0, 0, width, height), true, pipeline, res, views);

	auto v = node.GetTransform().GetRot();
	v.y = v.y + 1 * hg::time_to_sec_f(dt);
	node.GetTransform().SetRot(v);
	
	current_frame = bgfx::frame();

	hg::UpdateWindow(win);
}


int main(int argc, char* argv[])
{
	exe_path = std::filesystem::path();

	hg::set_log_level(hg::LL_Normal);
	hg::set_log_detailed(false);

	hg::InputInit();
	hg::WindowSystemInit();

	double w, h;
	emscripten_get_element_css_size("canvas", &w, &h);
	width = w;
	height = h;
	hg::log(hg::format("windows size: %1x%2").arg(width).arg(height).c_str());

	win = hg::NewWindow(width, height);

	hg::RenderInit(win, bgfx::RendererType::OpenGLES);
	bgfx::reset(width, height, BGFX_RESET_MSAA_X8 | BGFX_RESET_VSYNC | BGFX_RESET_FLIP_AFTER_RENDER | BGFX_RESET_FLUSH_AFTER_RENDER | BGFX_RESET_MAXANISOTROPY);

	hg::SetWindowTitle(win, std::string("Harfang/Emscripten"));

	// rendering pipeline
	pipeline = hg::CreateForwardPipeline(2048, false);

	GetAssetsPackage("project_compiled.zip");

	emscripten_set_main_loop(loop, 0, 1);
}
