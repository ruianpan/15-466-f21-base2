#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint city_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > city_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("city.pnct"));
	city_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > city_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("city.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh =city_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = city_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*city_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Car Body") car = &transform;
	}
	if (car == nullptr) throw std::runtime_error("car not found.");

	car_base_rotation = car->rotation;

	//get pointer to camera for convenience:
	//if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_q) {
			rotat1.downs += 1;
			rotat1.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_e) {
			rotat2.downs += 1;
			rotat2.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_1) {
			speed.downs += 1;
			speed.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_2) {
			slow.downs += 1;
			slow.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_3) {
			hspeed.downs += 1;
			hspeed.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_4) {
			hslow.downs += 1;
			hslow.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_q) {
			rotat1.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_e) {
			rotat2.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_1) {
			speed.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_2) {
			slow.pressed = false;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_3) {
			hspeed.downs += 1;
			hspeed.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_4) {
			hslow.downs += 1;
			hslow.pressed = true;
			return true;
		}
	} 

	return false;
}

void PlayMode::update(float elapsed) {
	
	car->rotation = car->rotation * glm::angleAxis(
		10.0f * elapsed*(rotat1.downs-rotat2.downs),
		glm::vec3(0.0f, 0.0f, 1.0f));
	
	
	carspeed  = carspeed + 0.05f*speed.downs -0.05f*slow.downs; 
	carspeedh  = carspeedh + 0.05f*hspeed.downs -0.05f*hslow.downs; 
	car->position.x += carspeed;
	car->position.y += carspeedh;

	if(car->position.x >=6){
		car->position.x =6;
	}

	if(car->position.x <=-3){
		car->position.x =-3;
	}

	if(car->position.y >=5){
		car->position.y =5;
	}
	if(car->position.y <=-5){
		car->position.y =-5;
	}

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * forward;
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	rotat1.downs = 0;
	rotat2.downs = 0;

	
	speed.downs = 0;
	slow.downs = 0;
	hspeed.downs = 0;
	hslow.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	
}
