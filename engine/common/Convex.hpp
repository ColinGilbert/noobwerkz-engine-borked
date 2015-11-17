#pragma once


#include "Shape.hpp"
#include "MathFuncs.hpp"

namespace noob
{
	class convex : protected shape
	{
		public:
			template <class Archive>
				void serialize(Archive& ar)
				{
					ar(points);
				}


			template <class Archive>
				static void load_and_construct( Archive & ar, cereal::construct<noob::convex> & construct )
				{
					std::vector<noob::vec3> points;
					ar(points);
					construct(points);
				}
			convex(float _mass, const std::vector<noob::vec3> _points) : points(_points)
		{	
			inner_shape = new btConvexHullShape(&points[0].v[0], points.size());

		}

		protected:
			std::vector<noob::vec3> points;
	};
}