#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, rotat1, rotat2, speed, slow, hspeed, hslow;

	float carspeed = 0.0f;
	float carspeedh = 0.0f;
	float acceleration = 0.0f;
	float hacceleration = 0.0f;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Transform *car = nullptr;

	glm::quat car_base_rotation;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
