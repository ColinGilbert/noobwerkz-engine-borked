#ifndef APPLICATION_H_
#define APPLICATION_H_



#include <assert.h>
#include <math.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>


// #include <bgfx.h>

#define PI (float)3.14159265

#include <time.h>
#include <assert.h>
#include <memory>
#include <cstdlib>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <sstream>
#include <map>
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <fstream>

/*
#include <cereal/access.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/map.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/binary.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
*/

#include "Logger.hpp"
#include "Untracked.hpp"
#include "MathFuncs.hpp"
#include "Camera.hpp"

class application
{
	public:

		application();
		virtual ~application();
		static application& get();
		void init();
		void step();
		void pause();
		void resume();
		uint32_t get_height() const { return static_cast<uint32_t>(height); }
		uint32_t get_width() const { return static_cast<uint32_t>(width); }
		void update(double delta);
		void draw(double delta);
		void window_resize(int w,int h);
		void touch(int pointerID, float x, float y, int action);
		void set_archive_dir(const std::string & filepath);
		const std::unique_ptr<noob::camera> cam;

		bool is_valid(int i)
		{
				return (i > 0);
		}

	protected:
		void init_graphics();

		static application* app_pointer;
		std::unique_ptr<std::string> prefix;
		bool paused, input_has_started;
		uint64_t time;
		std::vector<noob::vec2> finger_positions;
		float width, height;
		noob::mat4 proj_matrix;
};
#endif