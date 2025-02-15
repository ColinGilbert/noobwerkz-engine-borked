#include "SkeletalAnim.hpp"

#include "Logger.hpp"



void noob::skeletal_anim::init(const std::string& filename)
{
	ozz::io::File file(filename.c_str(), "rb");
	if (!file.opened())
	{
		logger::log(noob::importance::ERROR, noob::concat("[AnimatedModel] Load_skeleton(", filename, ") fail. Cannot open file."));
		valid = false;
		return;
	}

	ozz::io::IArchive archive(&file);
	if (!archive.TestTag<ozz::animation::Skeleton>())
	{
		logger::log(noob::importance::ERROR, noob::concat("[AnimatedModel] Load_skeleton(", filename , ") fail. Archive corrupt."));
		valid = false;
		return;
	}

	archive >> skeleton;
	model_matrices.resize(skeleton.num_joints()); // = allocator->AllocateRange<ozz::math::Float4x4>(skeleton.num_joints());

	logger::log(noob::importance::INFO, noob::concat("[AnimatedModel] Load_skeleton(", filename, ") success!"));

	valid = true;
}


bool noob::skeletal_anim::load_animation_file(const std::string& filename, const std::string& anim_name)
{
	ozz::animation::offline::RawAnimation _animation;
	ozz::io::File file(filename.c_str(), "rb");

	if (!file.opened())
	{
		return false;
	}

	ozz::io::IArchive archive(&file);
	if (!archive.TestTag<ozz::animation::offline::RawAnimation>())
	{
		return false;
	}

	archive >> _animation;

	raw_anims.insert(std::make_pair(anim_name, _animation));

	return true;
}


void noob::skeletal_anim::optimize(const std::string& name)
{
	ozz::animation::offline::AnimationBuilder builder;
	ozz::animation::offline::AnimationOptimizer optimizer;

	auto runtime_anim_search = runtime_anims.find(name);
	if (runtime_anim_search != runtime_anims.end())
	{
		// allocator->Delete(runtime_anim_search->second);
		runtime_anims.erase(name);
	}

	auto raw_anim_search = raw_anims.find(name);
	if (raw_anim_search != raw_anims.end())
	{
		ozz::animation::offline::RawAnimation raw_anim = raw_anim_search->second;

		ozz::animation::offline::RawAnimation optimized_anim;
		optimizer(raw_anim, skeleton, &optimized_anim);
		auto runtime_anim = builder(optimized_anim);
		runtime_anims.insert(std::make_pair(name, std::move(runtime_anim)));
	}
}


bool noob::skeletal_anim::switch_to_anim(const std::string& name)
{
	auto search = runtime_anims.find(name);
	if (search != runtime_anims.end())
	{
		current_anim_name = name;
		current_anim = search->second.get();
		return true;
	}
	return false;
}


bool noob::skeletal_anim::anim_exists(const std::string& name) const
{
	auto search = runtime_anims.find(name);
	if (search != runtime_anims.end())
	{
		return true;
	}
	return false;
}


void noob::skeletal_anim::update(float dt)
{
	noob::skeletal_anim::sampler current_sampler = create_sampler(*current_anim);
	current_sampler.controller.time = current_time;
	current_sampler.update(dt);
	current_time += dt;
	current_sampler.get_model_mats(model_matrices);
}


void noob::skeletal_anim::reset_time(float t)
{
	current_time = t;
}

void noob::skeletal_anim::set_total_runtime(float f)
{
	total_runtime = f;
}


std::string noob::skeletal_anim::get_current_anim() const
{
	return current_anim_name;
}


std::vector<noob::mat4f> noob::skeletal_anim::get_matrices() const
{
	std::vector<noob::mat4f> mats;
	mats.reserve(skeleton.num_joints());

	size_t num_mats = model_matrices.size();
	for (size_t i = 0; i < num_mats; ++i)
	{
		noob::mat4f m;
		ozz::math::Float4x4 ozz_mat = model_matrices[i];
		for (size_t c = 0; c < 4; ++c)
		{
			ozz::math::StorePtr(ozz_mat.cols[c], &m[c*4]);
		}
		mats.emplace_back(m);
	}
	return mats;
}


/*
   void noob::skeletal_anim::get_matrices(std::vector<noob::mat4f> mats) const
   {
   mats.clear();
   mats.reserve(skeleton.num_joints());

   size_t num_mats = model_matrices.Size();
   for (size_t i = 0; i < num_mats; ++i)
   {
   noob::mat4f m;
   ozz::math::Float4x4 ozz_mat = model_matrices[i];
   for (size_t c = 0; c < 4; ++c)
   {
   ozz::math::StorePtr(ozz_mat.cols[c], &m[c*4]);
   }
   mats.emplace_back(m);
   }
   }
   */


/*
   std::array<noob::vec3f, 4> noob::skeletal_anim::get_skeleton_bounds() const
   {

   }
   */


void noob::skeletal_anim::playback_controller::update(const ozz::animation::Animation& animation, float dt)
{
	if (!paused)
	{
		float new_time = time + dt * playback_speed;
		float loops = new_time / animation.duration();
		time = new_time - floorf(loops) * animation.duration();
	}
}


void noob::skeletal_anim::playback_controller::reset()
{
	time = 0.0;
	playback_speed = 1.0;
	paused = false;
}



void noob::skeletal_anim::sampler::update(float dt)
{
	ozz::animation::SamplingJob sampling_job;
	sampling_job.animation = animation;
	sampling_job.cache = cache;
	controller.update(*animation, dt);
	sampling_job.ratio = controller.time / runtime;
	sampling_job.output = ozz::make_span(locals);
	if (!sampling_job.Run())
	{
		logger::log(noob::importance::ERROR, noob::concat("[noob::skeletal_anim::sampler.update(", noob::to_string(dt), ") - sampling job failed."));
		return;
	}

}


ozz::vector<ozz::math::SoaTransform> noob::skeletal_anim::sampler::get_local_mats() const
{
	return locals;
}



void noob::skeletal_anim::sampler::get_model_mats(ozz::vector<ozz::math::Float4x4>& models)
{
	ozz::animation::LocalToModelJob ltm_job;
	ltm_job.skeleton = skeleton;
	ltm_job.input = ozz::make_span(locals);
	ltm_job.output = ozz::make_span(models);
	if (!ltm_job.Run())
	{
		logger::log(noob::importance::ERROR, "noob::skeletal_anim::sampler.get_model_mats() - local-to-model job failed.");
		return;
	}
}


noob::skeletal_anim::sampler noob::skeletal_anim::create_sampler(const ozz::animation::Animation& anim)
{
	noob::skeletal_anim::sampler sampler;
	sampler.animation = const_cast<ozz::animation::Animation*>(&anim);
	sampler.skeleton = &skeleton;
	sampler.cache->Resize(skeleton.num_joints()); // = allocator->New<ozz::animation::SamplingCache>(skeleton.num_joints());
	sampler.locals.resize(skeleton.num_soa_joints()); // = allocator->AllocateRange<ozz::math::SoaTransform>(skeleton.num_soa_joints());

	return sampler;
}


void noob::skeletal_anim::destroy_sampler(noob::skeletal_anim::sampler& sampler)
{
	// allocator->Deallocate(sampler.locals);
	// allocator->Delete(sampler.cache);
}


// TODO: Reintegrate the blended animations into the engine.
/*
   void noob::skeletal_anim::update(float dt)
   {
   for (size_t i = 0; i < num_layers; ++i)
   {
   noob::skeletal_anim::sampler& sampler = samplers[i];

   sampler.controller.update(sampler.animation, dt);

   ozz::animation::SamplingJob sampling_job;
   sampling_job.animation = &sampler.animation;
   sampling_job.cache = sampler.cache;
   sampling_job.time = sampler.controller.get_time();
   sampling_job.output = sampler.locals;

   if (!sampling_job.Run())
   {
   return;
   }
   }

   ozz::animation::BlendingJob::Layer layers[num_layers];
   for (size_t i = 0; i < num_layers; ++i)
   {
   layers[i].transform = samplers[i].locals;
   layers[i].weight = samplers[i].weight;
   }

   ozz::animation::BlendingJob blend_job;
   blend_job.threshold = threshold;
   blend_job.layers.begin = layers;
   blend_job.layers.end = layers + num_layers;
   blend_job.bind_pose = skeleton.bind_pose();
   blend_job.output = blended_locals;

   if (!blend_job.Run())
   {
   return;
   }

   ozz::animation::LocalToModelJob ltm_job;
   ltm_job.skeleton = &skeleton;
   ltm_job.input = blended_locals;
   ltm_job.output = model_matrices;

   if (!ltm_job.Run())
   {
   return;
   }
   }
   */
